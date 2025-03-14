local _M = {
	shouldStop = false,
}

function _M.onSetup()
	setPanelManaged(false)
	print("vamo")
	_M.wah = {}
	_M.timer = 1000
	_M.frame = 1
	_M.wah[1] = decodePng("/scripts/wah3.png")
	_M.wah[2] = decodePng("/scripts/wah2.png")
	_M.wah[3] = decodePng("/scripts/wah1.png")
	_M.wah[4] = _M.wah[2]

end


function _M.onLoop(dt)
	clearPanelBuffer()

	local pix = 1
	local wahframe = _M.wah[_M.frame]
	for y=1, 13 do 
		for x=1, 23 do  
			drawPanelPixel(x, y, wahframe[pix])
			pix = pix+1
		end
	end

	_M.timer = _M.timer  - dt 

	if (_M.timer < 0) then 
		_M.timer = 1000
		_M.frame = _M.frame +1
		if (_M.frame > 4) then 
			_M.frame = 1
		end
	end
	

	flipPanelBuffer()
	if readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED then 
		_M.shouldStop = true
		return true
	end
end 

function _M.onClose()
	_M.wah = nil
	collectgarbage()
end

return _M