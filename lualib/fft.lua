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


local CALIB_STATE_NOISE  = 0
local CALIB_STATE_TALK   = 1
local CALIB_STATE_REVIEW = 2
local CALIB_STATE_TEST   = 3


local ENVELOPE_ATTACK_ALPHA  = 0.5
local ENVELOPE_RELEASE_ALPHA = 0.02

-- How far above a band's own noise floor it must sit (envelope-wise)
-- before that band is considered "active speech" this frame.
local NOISE_ACTIVE_MULT   = 1.8
local NOISE_ACTIVE_MARGIN = 500

-- How fast the detected [bandStart, bandEnd] range is allowed to drift.
-- Lower = more resistant to a stray/transient noisy bin, but slower to
-- track a genuine shift in the speaker's voice.
local BAND_ADAPT_ALPHA = 0.15

-- Overall energy (summed across the current band range) must exceed
-- this multiple of the noise energy in that same range before we
-- consider the user to be "speaking" (used to gate min/max tracking).
local ACTIVE_ENERGY_MULT = 2.0


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
	_M.noise_threshold = cfg.default_noise_threshold or _M.noise_threshold

	_M.loadCalibration()
	cfg.band_count = cfg.band_count or 16

	fft.enabled = beginFft(cfg.gpio, cfg.samples, cfg.sampling_frequency, 100, cfg.band_count)

	

	if not fft.enabled then
		generic.displaySplashMessage("FFT FAILED")
		delay(2000)
	end

	startFft()
end

function _M.loadCalibration()
	_M.noise_threshold 			= tonumber(dictGet("fft_noise_threshold")) or _M.noise_threshold 
	_M.band_start 				= tonumber(dictGet("fft_speech_band_start")) or _M.band_start
	_M.band_end 				= tonumber(dictGet("fft_speech_band_end")) or _M.band_end
	_M.min_energy 				= tonumber(dictGet("fft_speech_min_energy")) or _M.min_energy
	_M.max_energy 				= tonumber(dictGet("fft_speech_max_energy")) or _M.max_energy
	_M.frist_frame_threshold 	= tonumber(dictGet("fft_speech_frist_frame_threshold")) or _M.frist_frame_threshold
	setNoiseThreshold(_M.noise_threshold)
end

function _M.getSpeechLevel(dt, attack, release, maxLevels)
	local energy = 0
    for b = _M.band_start, _M.band_end do
        energy = energy + getBandValueFft(b)
    end


    local tau = (energy > _M.smoothed) and (attack or 0.05) or (release or 0.2)
    local alpha = 1 - math.exp(-dt / tau)
    _M.smoothed = _M.smoothed * (1 - alpha) + energy * alpha

    if _M.smoothed > _M.frist_frame_threshold then
        local norm = (_M.smoothed - _M.min_energy) / (_M.max_energy - _M.min_energy)
        if norm < 0 then norm = 0 end
        if norm > 1 then norm = 1 end
        local level = math.floor(norm * maxLevels)
        if level >= maxLevels then
        	level = maxLevels-1
       	end
        if level < 1 then 
        	level = 1 
       	end
       	_M.level = level
    else
    	_M.level = 0
    end
    return _M.level
end



local function anyDirectionPressed()
	return input.readButtonStatus(BUTTON_UP) == BUTTON_PRESSED
		or input.readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED
		or input.readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED
		or input.readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED
end

-- Peak follower: snaps up fast, decays slowly. Good for tracking a
-- recent "ceiling" (max energy / per-band envelope).
local function updateEnvelope(prev, v)
	local alpha = (v > prev) and ENVELOPE_ATTACK_ALPHA or ENVELOPE_RELEASE_ALPHA
	return prev * (1 - alpha) + v * alpha
end

-- Floor follower: snaps down fast, rises slowly. Good for tracking a
-- recent "floor" (min sustained energy) without being permanently
-- dragged down by a single quiet frame.
local function updateFloor(prev, v)
	local alpha = (v < prev) and ENVELOPE_ATTACK_ALPHA or ENVELOPE_RELEASE_ALPHA
	return prev * (1 - alpha) + v * alpha
end

local INSTRUCTIONS = {
	[CALIB_STATE_NOISE] = {
		title = "STEP 1/2 - NOISE",
		lines = { "Stay silent.", "Let the mic read", "background noise." },
	},
	[CALIB_STATE_TALK] = {
		title = "STEP 2/2 - TALK",
		lines = { "Say: ", "ba ba ba ba ba", "at normal volume" },
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
	_M.state = CALIB_STATE_NOISE

	_M.avgNoise = 0
	_M.noiseBandValues = {}

	-- Adaptive band-range tracking (float "smoothed" positions + the
	-- rounded integer range actually used for energy sums).
	_M.bandStartSmooth = nil
	_M.bandEndSmooth = nil
	_M.bandStart = nil
	_M.bandEnd = nil
	_M.talkEnvelope = {}

	-- Live energy tracking while the user talks.
	_M.sumEnergyEnvelope = 0
	_M.floorEnergy = nil
	_M.ceilEnergy = nil
	_M.isSpeaking = false

	_M.result = nil
	_M.calibrating = true
	_M.showingInstructions = true

	return true
end

function _M.CalibrateDraw(dt)
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

	elseif _M.state == CALIB_STATE_TALK then
		local count = getBandCountFft()
		local bandValues = {}
		local bandActive = {}

		for i = 1, count do
			local v = getBandValueFft(i)
			bandValues[i] = v
			_M.maxBandInput = math.max(v, _M.maxBandInput)
			_M.talkEnvelope[i] = updateEnvelope(_M.talkEnvelope[i] or 0, v)

			local mapped = Map(v, 0, _M.maxBandInput * 1.1, 0, 64)
			oledDrawFilledRect(i * 5, 0, 4, math.ceil(mapped), 1)

			local bandThreshold = (_M.noiseBandValues[i] or 0) * NOISE_ACTIVE_MULT + NOISE_ACTIVE_MARGIN
			bandActive[i] = _M.talkEnvelope[i] > bandThreshold
		end

		-- Find the largest contiguous run of currently-active bands.
		-- Using "largest contiguous run" (rather than "any active band")
		-- means a lone stray bin (e.g. a burst of high-frequency noise)
		-- doesn't get treated as part of the speech band.
		local bestStart, bestEnd, bestLen = nil, nil, 0
		local curStart = nil
		for i = 1, count do
			if bandActive[i] then
				if not curStart then curStart = i end
				local len = i - curStart + 1
				if len > bestLen then
					bestLen = len
					bestStart = curStart
					bestEnd = i
				end
			else
				curStart = nil
			end
		end

		if bestStart then
			_M.bandStartSmooth = _M.bandStartSmooth
				and (_M.bandStartSmooth + BAND_ADAPT_ALPHA * (bestStart - _M.bandStartSmooth))
				or bestStart
			_M.bandEndSmooth = _M.bandEndSmooth
				and (_M.bandEndSmooth + BAND_ADAPT_ALPHA * (bestEnd - _M.bandEndSmooth))
				or bestEnd

			_M.bandStart = math.floor(_M.bandStartSmooth + 0.5)
			_M.bandEnd = math.floor(_M.bandEndSmooth + 0.5)
		end

		if _M.bandStart then
			oledDrawFastVLine(_M.bandStart * 5, 0, 64, 1)
			oledDrawFastVLine(_M.bandEnd * 5, 0, 64, 1)

			local sumEnergy = 0
			local noiseEnergy = 0
			for i = _M.bandStart, _M.bandEnd do
				sumEnergy = sumEnergy + (bandValues[i] or 0)
				noiseEnergy = noiseEnergy + (_M.noiseBandValues[i] or 0)
			end

			-- Sticky peak-follower used ONLY as the "are they still
			-- talking" gate. It stays high through the brief natural
			-- pauses between syllables so the gate doesn't flap open
			-- and shut on every "ba" -- but precisely BECAUSE it's
			-- sticky/peaky, it must never be fed into the floor
			-- tracker below (a floor-of-a-peak-signal just hugs the
			-- ceiling, which is why min/max were nearly equal).
			_M.sumEnergyEnvelope = updateEnvelope(_M.sumEnergyEnvelope, sumEnergy)

			local speakThreshold = noiseEnergy * ACTIVE_ENERGY_MULT
			_M.isSpeaking = _M.sumEnergyEnvelope > speakThreshold

			if _M.isSpeaking then
				-- Feed the RAW per-frame energy (not the sticky
				-- envelope) into the floor/ceiling trackers, so they
				-- actually see the true quiet dips and loud peaks
				-- that occur while the user is talking.
				_M.ceilEnergy = _M.ceilEnergy
					and updateEnvelope(_M.ceilEnergy, sumEnergy)
					or sumEnergy
				_M.floorEnergy = _M.floorEnergy
					and updateFloor(_M.floorEnergy, sumEnergy)
					or sumEnergy
			end
		end

		oledSetCursor(0, 40)
		oledDrawText("Bands: " .. (_M.bandStart or "-") .. " | " .. (_M.bandEnd or "-"))
		oledSetCursor(0, 49)
		oledDrawText("min:" .. math.floor(_M.floorEnergy or 0) .. " max:" .. math.floor(_M.ceilEnergy or 0))

	elseif _M.state == CALIB_STATE_REVIEW then
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
		oledDrawText("CONFIRM to test")

	elseif _M.state == CALIB_STATE_TEST then
		_M.testLevel = _M.getSpeechLevel(dt, nil, nil, 5)

		local boxSize = 22
		local gap = 5
		local totalWidth = boxSize * 5 + gap * 4
		local startX = math.floor((128 - totalWidth) / 2)
		local y = 6

		for i = 0, 4 do
			local x = startX + i * (boxSize + gap)
			local isOn = _M.testLevel >= i

			oledDrawRect(x, y, boxSize, boxSize, 1)
			if isOn then
				oledDrawFilledRect(x + 3, y + 3, boxSize - 6, boxSize - 6, 1)
			end

			local label = tostring(i)
			local textColor = isOn and 0 or 1
			oledSetCursor(x + math.floor(boxSize / 2) - 3, y + math.floor(boxSize / 2) - 4)
			oledSetTextColor(textColor)
			oledDrawText(label)
		end
		oledSetTextColor(1)

		oledSetCursor(0, 44)
		oledDrawText("Talk to test level")
		oledSetCursor(0, 54)
		oledDrawText("CONFIRM=save DIR=redo")
	end

	oledDisplay()
end

local function finalizeCalibration()
	local bandStart, bandEnd = _M.bandStart, _M.bandEnd

	local noiseEnergy = 0
	for i = bandStart, bandEnd do
		noiseEnergy = noiseEnergy + (_M.noiseBandValues[i] or 0)
	end

	local minEnergy = _M.floorEnergy or 0
	local maxEnergy = _M.ceilEnergy or (minEnergy * 1.5)

	local threshold = noiseEnergy * 1.5
	if threshold >= minEnergy then
		threshold = minEnergy * 0.5
	end

	if maxEnergy <= minEnergy then
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

local function saveCalibrationBackup()
	_M.savedCalibration = {
		noise_threshold        = _M.noise_threshold,
		band_start             = _M.band_start,
		band_end               = _M.band_end,
		min_energy             = _M.min_energy,
		max_energy             = _M.max_energy,
		frist_frame_threshold  = _M.frist_frame_threshold,
	}
end

local function applyCalibrationResult()
	local r = _M.result

	_M.noise_threshold       = _M.noiseThreshold
	_M.band_start            = r.band_start
	_M.band_end              = r.band_end
	_M.min_energy            = r.min_energy
	_M.max_energy            = r.max_energy
	_M.frist_frame_threshold = r.frist_frame_threshold

	setNoiseThreshold(_M.noise_threshold)

	_M.smoothed = 0
	_M.level = 0
end

local function restoreCalibrationBackup()
	local b = _M.savedCalibration
	if not b then return end

	_M.noise_threshold       = b.noise_threshold
	_M.band_start            = b.band_start
	_M.band_end              = b.band_end
	_M.min_energy            = b.min_energy
	_M.max_energy            = b.max_energy
	_M.frist_frame_threshold = b.frist_frame_threshold

	setNoiseThreshold(_M.noise_threshold)
end

function _M.Calibrate(dt)
	if _M.showingInstructions then
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
			_M.state = CALIB_STATE_TALK
			_M.showingInstructions = true
			_M.noiseThreshold = _M.avgNoise * 1.1
			setNoiseThreshold(_M.noiseThreshold)
			_M.maxBandInput = _M.noiseThreshold * 8
		end
		if anyDirectionPressed() then
			_M.maxBandInput = 100
		end

	elseif _M.state == CALIB_STATE_TALK then
		if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
			if _M.bandStart and _M.floorEnergy then
				finalizeCalibration()
				_M.state = CALIB_STATE_REVIEW
			end
		end
		if anyDirectionPressed() then
			_M.onEnter()
		end

	elseif _M.state == CALIB_STATE_REVIEW then
		if input.readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then
			saveCalibrationBackup()
			applyCalibrationResult()
			_M.state = CALIB_STATE_TEST
		end
		if anyDirectionPressed() then
			_M.onEnter()
		end

	elseif _M.state == CALIB_STATE_TEST then
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
			_M.calibrating = false
		end
		if anyDirectionPressed() then
			restoreCalibrationBackup()
			_M.onEnter()
		end
	end
end

return _M