local _M = {
	VERSION_REQUIRED="1.0.0",
	shouldStop = false,
	red = 255,
	green = 13,
	blue = 150,
	selected=1,
	colors = {"red", "green", "blue"},
	uiTimer = 0,
	rainbowShift = 0,
	rainbowShiftTimer = 0,

}

function _M.onSetup()
	setPanelManaged(false)
	delay(500)
end

local function hslToRgb(h, s, l)
    h = h / 360
    local r, g, b

    if s == 0 then
        r, g, b = l, l, l -- achromatic
    else
        local function hueToRgb(p, q, t)
            if t < 0 then t = t + 1 end
            if t > 1 then t = t - 1 end
            if t < 1/6 then return p + (q - p) * 6 * t end
            if t < 1/2 then return q end
            if t < 2/3 then return p + (q - p) * (2/3 - t) * 6 end
            return p
        end

        local q = l < 0.5 and l * (1 + s) or l + s - l * s
        local p = 2 * l - q
        r = hueToRgb(p, q, h + 1/3)
        g = hueToRgb(p, q, h)
        b = hueToRgb(p, q, h - 1/3)
    end

    return math.floor(r * 255), math.floor(g * 255), math.floor(b * 255)
end

local function positionToRainbowColor(position, total_length)
    position = math.max(0, math.min(position, total_length))


    local hue = (position / total_length) * 360

    local saturation = 1.0
    local lightness = 0.5

    local r, g, b = hslToRgb(hue, saturation, lightness)

    return color565(r, g, b)
end

function _M.drawTextRainbow(x,y, str, shift)
	for i=1,#str do  
		local colorIndex = (i+shift) % (#str+1)
		drawPanelChar(x + (i-1)*6, y, string.byte(str:sub(i,i)), positionToRainbowColor(colorIndex, #str), 0, 1)
	end
end

function _M.drawText(x,y, str, color, bg)
	bg = bg or 0
	for i=1,#str do  
		drawPanelChar(x + (i-1)*6, y, string.byte(str:sub(i,i)), color, bg, 1)
	end
end

function _M.onLoop(dt)
	_M.uiTimer = _M.uiTimer - dt
	clearPanelBuffer()

	local selected = _M.colors[_M.selected]
	
		_M.drawText(27, 9, "G: "..string.format("%3d", _M.green), color565(0, selected ~= "green" and 255 or 0, 0), color565(0, selected == "green" and 255 or 0, 0))
	drawPanelRect(26, 8, 38, 9, color565(100,255,100))
	_M.drawText(27, 1, "R: "..string.format("%3d", _M.red),   color565(selected ~= "red" and 255 or 0, 0, 0),   color565(selected == "red" and 255 or 0, 0, 0  ))
	drawPanelRect(26, 0, 38, 9, color565(255,100,100))
	_M.drawText(27, 17, "B: "..string.format("%3d", _M.blue), color565(0, 0, selected ~= "blue" and 255 or 0),  color565(0, 0, selected == "blue" and 255 or 0 ))
	drawPanelRect(26, 16, 38, 9, color565(100,100,255))



	_M.drawText(12, 25, "0x"..string.format("%.2x%.2x%.2x", _M.red, _M.green, _M.blue), color565(_M.red, _M.green, _M.blue), 0)
	if readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and _M.uiTimer < 0 then 
		_M.uiTimer = 5
		_M[selected] = _M[selected] +1
		if _M[selected] > 255 then 
			_M[selected] = 255
		end
	end
	if readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and _M.uiTimer < 0 then 
		_M.uiTimer = 5
		_M[selected] = _M[selected] -1
		if _M[selected] < 0 then 
			_M[selected] = 0
		end
	end

	if readButtonStatus(BUTTON_DOWN) == BUTTON_JUST_PRESSED then 
		_M.selected = _M.selected +1
		if _M.selected > 3 then 
			_M.selected = 1
		end
	end

	if readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED then 
		_M.selected = _M.selected -1
		if _M.selected < 1 then 
			_M.selected = 3
		end
	end

	_M.rainbowShiftTimer = _M.rainbowShiftTimer - dt 
	if _M.rainbowShiftTimer < 0 then 
		_M.rainbowShiftTimer = 90
		_M.rainbowShift = _M.rainbowShift + 1
	end
	



	drawPanelFillCircle(10, 10, 9, color565(_M.red, _M.green, _M.blue))
	drawPanelCircle(10, 10, 10, color565(255,255,255))

	local reading = readLidar()
	if reading > 1200 then 
		reading = 1200
	end
	if reading < 0 then 
		reading = 0
	end

	if hasLidar() then
		_M.drawTextRainbow(64, 0, "Lidar: on", _M.rainbowShift)
		_M.drawText(64, 15, "Reading:", color565(255,255,255))
		_M.drawTextRainbow(64, 25, string.format("%.4dmm", reading), _M.rainbowShift)

		drawPanelRect(64, 8, 60, 6, color565(255,255,255))
		drawPanelFillRect(65,  9, (reading/1200)*59, 4, color565(200,200,200))
	else
		local reading = readLidar()
		_M.drawText(64, 0, "Lidar: off", color565(255,255,255))
		_M.drawText(64, 15, "Reading:", color565(255,255,255))
		_M.drawText(64, 25, string.format("%.4dmm", reading), color565(200,200,200))
	end
	
	

	flipPanelBuffer()
	if readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED and readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED and readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED and readButtonStatus(BUTTON_UP) == BUTTON_PRESSED and readButtonStatus(BUTTON_DOWN) == BUTTON_PRESSED then 
		_M.shouldStop = true
		return true
	end
end 

function _M.onClose()
end

return _M


