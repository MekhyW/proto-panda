local _M = {}

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
	cfg.noise_threshold = cfg.noise_threshold or 2000
	cfg.band_count = cfg.band_count or 16

	cfg.fft.enabled = beginFft(cfg.gpio, cfg.samples, cfg.sampling_frequency, cfg.noise_threshold, cfg.band_count)

	if not cfg.fft.enabled then 
		generic.displaySplashMessage("FFT FAILED")
		delay(2000)
	end
end
return _M