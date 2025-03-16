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

	_M.y = 0
	_M.x = 10
	_M.vy = 70
	_M.grounded = false

end


function _M.onLoop(dt)
	clearPanelBuffer()

	local pix = 1
	local wahframe = _M.wah[_M.frame]

	
	oledDrawRect(0,0,65, 33, 1)
	for y=1, 13 do 
		for x=1, 23 do  
			drawPanelPixel(_M.x+x, y+_M.y, wahframe[pix])
			drawPanelPixel((-_M.x)+128-x, y+_M.y, wahframe[pix])
			if wahframe[pix] ~= 0 then 
				oledDrawPixel(_M.x+x+1,y+0+_M.y, 1)
			end
			pix = pix+1
		end
	end

	_M.timer = _M.timer  - dt 

	if (_M.timer < 0) then 
		_M.timer = 200
		_M.frame = _M.frame +1
		if (_M.frame > 4) then 
			_M.frame = 1
		end
	end
	dt = dt/1000;

	if readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED and _M.grounded then 
		_M.vy = -40
		_M.grounded = false
	end

	_M.vy = _M.vy + 70*dt

	_M.y = _M.y + _M.vy*dt;
	if _M.y >= 32-14 then  
		_M.y = 32-14
		_M.grounded = true
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