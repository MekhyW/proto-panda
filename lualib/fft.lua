local _M = {

	noise_threshold = 2500,
	band_start = 2,
    band_end = 8,
    min_energy = 50000,
    max_energy = 200000,
    frist_frame_threshold = 60000,


    --Speech level:
    smoothed = 0,
    level = 0,

}

local generic = require("generic")
local Map = generic.map


local CALIB_STATE_NOISE = 0
local CALIB_STATE_NOISE_CONFIRM = 1
local CALIB_STATE_TALK = 2
local CALIB_STATE_TALK_LOUD = 3
local CALIB_STATE_DONE = 4


local ACTIVE_BAND_THRESHOLD = 8
local ENVELOPE_ATTACK_ALPHA = 0.5
local ENVELOPE_RELEASE_ALPHA = 0.02


function _M.load()
	local cfg = configloader.Get()
	if not cfg.fft or not cfg.fft.enabled then
		generic.displaySplashMessage("Starting:\nNo fft configued")
		delay(200)
		return
	end
	local cfg = cfg.fft

	if not tonumber(cfg.gpio) then
		error("GPIO for fft must be a number.")
	end

	cfg.gpio = tonumber(cfg.gpio)

	if cfg.gpio <= 0 or cfg.gpio >= 11 then
		error("GPIO for fft must be channel1, that means gpipo between 1 and 10")
	end

	cfg.samples = cfg.samples or 512
	cfg.sampling_frequency = cfg.sampling_frequency or 44100

	
	cfg.band_count = cfg.band_count or 16

	fft.enabled = beginFft(cfg.gpio, cfg.samples, cfg.sampling_frequency, 100, cfg.band_count)

	_M.loadCalibration()

	if not fft.enabled then
		generic.displaySplashMessage("FFT FAILED")
		delay(2000)
	end

	startFft()
end

function _M.loadCalibration()
	_M.noise_threshold 			= tonumber(dictGet("fft_noise_threshold")) or cfg.default_noise_threshold or _M.noise_threshold
	_M.band_start 				= tonumber(dictGet("fft_speech_band_start")) or _M.band_start
	_M.band_end 				= tonumber(dictGet("fft_speech_band_end")) or _M.band_end
	_M.min_energy 				= tonumber(dictGet("fft_speech_min_energy")) or _M.min_energy
	_M.max_energy 				= tonumber(dictGet("fft_speech_max_energy")) or _M.max_energy
	_M.frist_frame_threshold 	= tonumber(dictGet("fft_speech_frist_frame_threshold")) or _M.frist_frame_threshold
	setNoiseThreshold(_M.noise_threshold)
end

function _M.getSpeechLevel(attack, release, maxLevels)
	local energy = 0
    for b = _M.band_start, _M.band_end do
        energy = energy + getBandValueFft(b)
    end


    local tau = (energy > _M.smoothed) and (attack or 0.05) or (release or 0.2)
    local alpha = 1 - math.exp(-dt / tau)
    _M.smoothed = _M.smoothed * (1 - alpha) + energy * alpha

    if _M.smoothed > _M.frist_frame_threshold then
        local norm = (obj.mouthSmoothed - _M.min_energy) / (_M.max_energy - _M.min_energy)
        if norm < 0 then norm = 0 end
        if norm > 1 then norm = 1 end
        local level = math.floor(norm * maxLevels)
        if level > maxLevels - 1 then
        	level = maxLevels - 1 
       	end
        if level < 1 then 
        	level = 1 
       	end
       	_M.level = level
    end
    return _M.level
end



local function anyDirectionPressed()
	return input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED
		or input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED
		or input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED
		or input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED
end

local function updateEnvelope(prev, v)
	local alpha = (v > prev) and ENVELOPE_ATTACK_ALPHA or ENVELOPE_RELEASE_ALPHA
	return prev * (1 - alpha) + v * alpha
end

local INSTRUCTIONS = {
	[CALIB_STATE_NOISE] = {
		title = "STEP 1/4 - NOISE",
		lines = { "Stay silent.", "Let the mic read", "background noise." },
	},
	[CALIB_STATE_NOISE_CONFIRM] = {
		title = "STEP 2/4 - MAX LVL",
		lines = { "Make a loud noise", "(clap or shout) to", "set the max level." },
	},
	[CALIB_STATE_TALK] = {
		title = "STEP 3/4 - TALK",
		lines = { "Say: ", "ba ba ba ba ba", "a few times" },
	},
	[CALIB_STATE_TALK_LOUD] = {
		title = "STEP 4/4 - TALK",
		lines = { "Talk louder,", "a bit.", "" },
	},
}

local function drawInstructions(info)
	oledSetCursor(0, 0)
	oledDrawText(info.title)

	local y = 16
	for _, line in ipairs(info.lines) do
		oledSetCursor(0, y)
		oledDrawText(line)
		y = y + 10
	end

	oledSetCursor(0, 54)
	oledDrawText("Press CONFIRM")
end

function _M.onEnter()
	setNoiseThreshold(1)

	_M.maxBandInput = 100
	_M.noise = 1
	_M.state = CALIB_STATE_NOISE

	_M.avgNoise = 0
	_M.noiseBandValues = {} 

	_M.bandStartMin = nil
	_M.bandEndMax = nil
	_M.talkEnvelope = {}

	_M.loudEnergyEnvelope = 0
	_M.maxEnergy = 0

	_M.result = nil
	_M.completed = false

	_M.showingInstructions = true

	return true
end

function _M.CalibrateDraw()
	oledClearScreen()

	if _M.showingInstructions and INSTRUCTIONS[_M.state] then
		drawInstructions(INSTRUCTIONS[_M.state])
		oledDisplay()
		return
	end

	if _M.state == CALIB_STATE_NOISE then
		local count = getBandCountFft()
		local avg = 0
		for i = 1, count do
			local v = getBandValueFft(i)
			_M.maxBandInput = math.max(v, _M.maxBandInput)
			avg = avg + v

			_M.noiseBandValues[i] = updateEnvelope(_M.noiseBandValues[i] or 0, v)

			local mapped = Map(v, 0, _M.maxBandInput * 1.1, 0, 64)
			oledDrawFilledRect(i * 5, 0, 4, math.ceil(mapped), 1)
		end
		avg = avg / count

		_M.avgNoise = _M.avgNoise * 0.8 + avg * 0.2
		oledDrawFastHLine(0, Map(_M.avgNoise * 1.1, 0, _M.maxBandInput * 1.1, 0, 64), 128, 1)
		oledSetCursor(0, 40)
		oledDrawText("Stay quiet...")
		oledSetCursor(0, 49)
		oledDrawText("Noise: " .. (_M.avgNoise * 1.1))

	elseif _M.state == CALIB_STATE_NOISE_CONFIRM then
		local count = getBandCountFft()
		for i = 1, count do
			local v = getBandValueFft(i)
			_M.maxBandInput = math.max(v, _M.maxBandInput)
			local mapped = Map(v, 0, _M.maxBandInput * 1.1, 0, 64)
			oledDrawFilledRect(i * 5, 0, 4, math.ceil(mapped), 1)
		end
		oledSetCursor(0, 40)
		oledDrawText("Make a LOUD noise")
		oledSetCursor(0, 49)
		oledDrawText("Max: " .. _M.maxBandInput)

	elseif _M.state == CALIB_STATE_TALK then
		local count = getBandCountFft()
		local startingOverLimit = -1
		local finishedOverLimit = -1
		for i = 1, count do
			local v = getBandValueFft(i)
			_M.talkEnvelope[i] = updateEnvelope(_M.talkEnvelope[i] or 0, v)

			local mapped = Map(v, 0, _M.maxBandInput, 0, 64)
			oledDrawFilledRect(i * 5, 0, 4, math.ceil(mapped), 1)

			if mapped > ACTIVE_BAND_THRESHOLD then
				if startingOverLimit == -1 then
					startingOverLimit = i
				end
				finishedOverLimit = i
			end
		end

		if startingOverLimit ~= -1 then
			_M.bandStartMin = _M.bandStartMin and math.min(_M.bandStartMin, startingOverLimit) or startingOverLimit
			_M.bandEndMax = _M.bandEndMax and math.max(_M.bandEndMax, finishedOverLimit) or finishedOverLimit
		end

		if _M.bandStartMin then
			oledDrawFastVLine(_M.bandStartMin * 5, 0, 64, 1)
			oledDrawFastVLine(_M.bandEndMax * 5, 0, 64, 1)
		end

		oledSetCursor(0, 40)
		oledDrawText("Keep talking...")
		oledSetCursor(0, 49)
		oledDrawText("Bands: " .. (_M.bandStartMin or "-") .. " | " .. (_M.bandEndMax or "-"))

	elseif _M.state == CALIB_STATE_TALK_LOUD then
		local count = getBandCountFft()
		local sumEnergy = 0
		for i = 1, count do
			local v = getBandValueFft(i)
			local mapped = Map(v, 0, _M.maxBandInput, 0, 64)
			oledDrawFilledRect(i * 5, 0, 4, math.ceil(mapped), 1)

			if i >= _M.bandStartMin and i <= _M.bandEndMax then
				sumEnergy = sumEnergy + v
			end
		end

		_M.loudEnergyEnvelope = updateEnvelope(_M.loudEnergyEnvelope, sumEnergy)
		_M.maxEnergy = math.max(_M.maxEnergy, _M.loudEnergyEnvelope)

		oledDrawFastVLine(_M.bandStartMin * 5, 0, 64, 1)
		oledDrawFastVLine(_M.bandEndMax * 5, 0, 64, 1)

		oledSetCursor(0, 40)
		oledDrawText("Keep talking...")
		oledSetCursor(0, 49)
		oledDrawText("Max energy: " .. math.floor(_M.maxEnergy))

	elseif _M.state == CALIB_STATE_DONE then
		local r = _M.result
		oledSetCursor(0, 0)
		oledDrawText("band_start: " .. r.band_start)
		oledSetCursor(0, 12)
		oledDrawText("band_end: " .. r.band_end)
		oledSetCursor(0, 22)
		oledDrawText("threshold: " .. math.floor(r.frist_frame_threshold))
		oledSetCursor(0, 32)
		oledDrawText("min: " .. math.floor(r.min_energy) )
		oledSetCursor(0, 42)
		oledDrawText("max: " .. math.floor(r.max_energy))
		oledSetCursor(0, 54)
		oledDrawText("Press confirm")
	end

	oledDisplay()
end

local function finalizeCalibration()
	local bandStart, bandEnd = _M.bandStartMin, _M.bandEndMax

	local minEnergy = 0
	local noiseEnergy = 0
	for i = bandStart, bandEnd do
		minEnergy = minEnergy + (_M.talkEnvelope[i] or 0)
		noiseEnergy = noiseEnergy + (_M.noiseBandValues[i] or 0)
	end

	local threshold = noiseEnergy * 1.5
	if threshold >= minEnergy then
		-- Safety fallback in case the mic is very quiet / noise floor is
		-- close to speech level: keep the threshold meaningfully below min_energy.
		threshold = minEnergy * 0.5
	end

	local maxEnergy = _M.maxEnergy
	if maxEnergy <= minEnergy then
		-- Safety fallback so max_energy - min_energy never ends up <= 0.
		maxEnergy = minEnergy * 1.5
	end

	_M.result = {
		band_start = bandStart,
		band_end = bandEnd,
		frist_frame_threshold = threshold,
		min_energy = minEnergy,
		max_energy = maxEnergy,
	}

	return _M.result
end

function _M.Calibrate(dt)
	if _M.showingInstructions then
		-- Waiting for the user to read the instructions for this stage and
		-- press confirm before we start actually capturing anything.
		if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
			_M.showingInstructions = false
		end
		if _M.state ~= CALIB_STATE_NOISE and anyDirectionPressed() then
			_M.onEnter()
		end
		return
	end

	if _M.state == CALIB_STATE_NOISE then
		if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
			_M.state = CALIB_STATE_NOISE_CONFIRM
			_M.showingInstructions = true
			_M.noiseThreshold = _M.avgNoise * 1.1
			setNoiseThreshold(_M.noiseThreshold)
			_M.maxBandInput = _M.noiseThreshold * 8
		end
		if anyDirectionPressed() then
			_M.maxBandInput = 100
		end

	elseif _M.state == CALIB_STATE_NOISE_CONFIRM then
		if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
			_M.state = CALIB_STATE_TALK
			_M.showingInstructions = true
			_M.maxBandInput = _M.maxBandInput * 1.1
		end
		if anyDirectionPressed() then
			_M.onEnter()
		end

	elseif _M.state == CALIB_STATE_TALK then
		if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
			if _M.bandStartMin then
				_M.state = CALIB_STATE_TALK_LOUD
				_M.showingInstructions = true
				_M.loudEnergyEnvelope = 0
				_M.maxEnergy = 0
			end
			-- If no speech was ever detected above the noise floor, ignore
			-- confirm - keep waiting for the user to actually talk.
		end
		if anyDirectionPressed() then
			_M.onEnter()
		end

	elseif _M.state == CALIB_STATE_TALK_LOUD then
		if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
			if _M.maxEnergy > 0 then
				finalizeCalibration()
				_M.state = CALIB_STATE_DONE
			end
		end
		if anyDirectionPressed() then
			_M.onEnter()
		end

	elseif _M.state == CALIB_STATE_DONE then
		if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
			dictSet("fft_noise_threshold", tostring(_M.noiseThreshold))
			dictSet("fft_speech_min_energy", tostring(_M.result.min_energy))
			dictSet("fft_speech_max_energy", tostring(_M.result.max_energy))
			dictSet("fft_speech_band_start", tostring(_M.result.band_start))
			dictSet("fft_speech_band_end", tostring(_M.result.band_end))
			dictSet("fft_speech_frist_frame_threshold", tostring(_M.result.frist_frame_threshold))
			_M.result = nil
			dictSave()
			_M.loadCalibration()
			_M.quit = true
		end
		if anyDirectionPressed() then
			_M.onEnter()
		end
	end
end

return _M