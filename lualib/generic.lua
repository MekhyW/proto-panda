local _M = {

}

function _M.map(x, in_min, in_max, out_min, out_max) 
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end

function _M.setAutoPowerMode()
	setPoweringMode(BUILT_IN_POWER_MODE)
	if BUILT_IN_POWER_MODE == POWER_MODE_USB_9V or BUILT_IN_POWER_MODE == POWER_MODE_BATTERY then 
		waitForPower()
	else
		panelPowerOn()
	end
end

function _M.displayWarning(headMessage, message, duration)
	duration = duration or 2000
	print("Warning: "..headMessage.." | "..message.." | "..duration)
	local stop = millis()+duration
	local scroll = _M.scrollingText(message, 20, 250)
	while scroll.cycles <= 0 or stop > millis() do
		oledClearScreen()
		oledSetCursor(0,0)
		oledSetFontSize(1)
		oledDrawText(headMessage)
		oledDrawLine(0, 15, OLED_SCREEN_WIDTH, 15, WHITE)
		oledSetFontSize(2)
		oledSetCursor(0,16)
		scroll:update()
		oledDrawText(scroll:text())
		oledSetFontSize(1)
		oledDisplay()
	end
end

function _M.scrollingText(message, maxSize, stepDuration)
	return {
		message = message,
		maxTextShowSize=maxSize,
		textScrollingPos = 1,
    	textScrollingTimer = 1500,
    	stepDuration=stepDuration,
    	lastDt = millis(),
    	cycles=0,

    	update = function(self, dt)
    		if not dt then 
    			dt = millis()-self.lastDt
    			self.lastDt = millis()
    		end
    		self.textScrollingTimer = self.textScrollingTimer - dt
		    if self.textScrollingTimer <= 0 then
		        self.textScrollingTimer = self.stepDuration
		        self.textScrollingPos = self.textScrollingPos+1
		        if self.textScrollingPos > (#message-(self.maxTextShowSize-1)) then  
		            self.textScrollingTimer = 2500
		        end
		        if self.textScrollingPos > (#message-self.maxTextShowSize) then  
		            self.textScrollingTimer = 1500
		            self.textScrollingPos = 1
		            self.cycles = self.cycles+1
		        end
		    end
    	end,

    	text = function(self)
    		if #self.message > self.maxTextShowSize then
               return self.message:sub(self.textScrollingPos, math.min(self.textScrollingPos+self.maxTextShowSize, #self.message))
            end
            return self.message
    	end,
	}
end

return _M