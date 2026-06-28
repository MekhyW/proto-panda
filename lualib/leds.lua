local _M = {}

local configloader = require("configloader")

local behaviorMap = {
    ['none'] = BEHAVIOR_NONE,
    ['pride'] = BEHAVIOR_PRIDE,
    ['rotate'] = BEHAVIOR_ROTATE,
    ['random'] = BEHAVIOR_RANDOM_COLOR,
    ['fade_cycle'] = BEHAVIOR_FADE_CYCLE,
    ['rotate_fade_cycle'] = BEHAVIOR_ROTATE_FADE_CYCLE,
    ['color_rgb'] = BEHAVIOR_STATIC_RGB,
    ['color_hsv'] = BEHAVIOR_STATIC_HSV,
    ['random_blink'] = BEHAVIOR_RANDOM_BLINK,
    ['icon_x'] = BEHAVIOR_ICON_X,
    ['icon_y'] = BEHAVIOR_ICON_I,
    ['icon_v'] = BEHAVIOR_ICON_V,
    ['rotate_sine_v'] = BEHAVIOR_ROTATE_SINE_V,
    ['rotate_sine_s'] = BEHAVIOR_ROTATE_SINE_S,
    ['rotate_sine_h'] = BEHAVIOR_ROTATE_SINE_H,
    ['fade_in'] = BEHAVIOR_FADE_IN,
    ['noise'] = BEHAVIOR_NOISE,
}
--[[
    Parameter Truth Table for LED Behaviors
    
    mode                    | param1    | param2      | param3         | param4
    ------------------------|-----------|-------------|----------------|------------------
    none                    | -         | -           | -              | -
    pride                   | -         | -           | -              | -
    rotate                  | -         | -           | -              | speed (ms)
    random_color            | -         | -           | -              | -
    fade_cycle              | hue       | speed       | min_brightness  | -
    rotate_fade_cycle       | hue       | speed       | min_brightness  | rotate_speed
    color_rgb               | red       | green       | blue           | -
    color_hsv               | hue       | saturation  | value          | -
    random_blink            | base_hue  | hue_variance| brightness     | blink_speed
    icon_x                  | -         | -           | -              | -
    icon_y                  | -         | -           | -              | -
    icon_v                  | -         | -           | -              | -
    rotate_sine_v           | hue       | saturation  | speed          | -
    rotate_sine_s           | hue       | brightness  | speed          | -
    rotate_sine_h           | sat       | brightness  | speed          | -
    fade_in                 | hue       | saturation  | step           | delay
    noise                   | -         | -           | step           | delay

    Values are 0-255 except where noted (speed/delay are milliseconds)

    Example:

    "leds": { 
        "pin_mode": "double",
        "groups":[
            {
                "pin_side": "left",
                "led_count": 32,
                "mode": "color_rgb",
                "r": 255,
                "g": 120,
                "b": 20
            },
            {
                "pin_side": "right",
                "led_count": 32,
                "mode": "color_hsv",
                "r": 255,
                "g": 120,
                "b": 20
            }
        ]

    }
--]]

local modeParameters = {
    none = {},
    pride = {},
    rotate = { [4] = "speed" },
    random_color = {},
    fade_cycle = { [1] = "hue", [2] = "speed", [3] = "min_brightness" },
    rotate_fade_cycle = { [1] = "hue", [2] = "speed", [3] = "min_brightness", [4] = "rotate_speed" },
    color_rgb = { [1] = "r", [2] = "g", [3] = "b" },
    color_hsv = { [1] = "h", [2] = "s", [3] = "v" },
    random_blink = { [1] = "base_hue", [2] = "hue_variance", [3] = "brightness", [4] = "blink_speed" },
    icon_x = {},
    icon_y = {},
    icon_v = {},
    rotate_sine_v = { [1] = "hue", [2] = "saturation", [3] = "speed" },
    rotate_sine_s = { [1] = "hue", [2] = "brightness", [3] = "speed" },
    rotate_sine_h = { [1] = "sat", [2] = "brightness", [3] = "speed" },
    fade_in = { [1] = "hue", [2] = "saturation", [3] = "step", [4] = "delay" },
    noise = { [3] = "step", [4] = "delay" },
}

local baseFields = { "mode", "led_count", "comment", "_comment", "pin_side" }

function _M.loadConfig(groupId, conf)
    if type(conf.mode) ~= 'string' then  
        error("Undefined led mode.")
    end
    if not behaviorMap[conf.mode] then  
        error("No such mode: " .. conf.mode)
    end
    
    local paramMapping = modeParameters[conf.mode]
    if not paramMapping then
        error("No parameter definition for mode: " .. conf.mode)
    end
    
    -- Build list of allowed parameter names
    local allowedParams = {}
    for pos, paramName in pairs(paramMapping) do
        table.insert(allowedParams, paramName)
    end
    
    -- Check for missing required parameters
    for pos, paramName in pairs(paramMapping) do
        if conf[paramName] == nil then
            error(string.format("Mode '%s' requires missing parameter: '%s' (param%d)", 
                conf.mode, paramName, pos))
        end
    end
    
    for field, _ in pairs(conf) do
        local isAllowed = false
        for _, baseField in ipairs(baseFields) do
            if field == baseField then
                isAllowed = true
                break
            end
        end

        if not isAllowed then
            for _, paramName in ipairs(allowedParams) do
                if field == paramName then
                    isAllowed = true
                    break
                end
            end
        end
        
        if not isAllowed then
            error(string.format("Mode '%s' does not accept parameter '%s'. Allowed parameters: %s", 
                conf.mode, field, table.concat(allowedParams, ", ")))
        end
    end
    
    local behavior = behaviorMap[conf.mode]
    
    local param1 = 0
    local param2 = 0
    local param3 = 0
    local param4 = 0
    
    for pos, paramName in pairs(paramMapping) do
        local value = conf[paramName]

        local isSpeedParam = (paramName == "speed" or paramName == "rotate_speed" or 
                              paramName == "blink_speed" or paramName == "step" or 
                              paramName == "delay")
        
        if not isSpeedParam then
            if value < 0 or value > 255 then
                error(string.format("Parameter '%s' in mode '%s' must be between 0 and 255 (got %d)", 
                    paramName, conf.mode, value))
            end
        end
        
        if pos == 1 then
            param1 = value
        elseif pos == 2 then
            param2 = value
        elseif pos == 3 then
            param3 = value
        elseif pos == 4 then
            param4 = value
        end
    end
    log("Set led group "..groupId.." behavior as "..behavior.." with "..param1..', '..param2..", "..param3..', '..param4)
    ledsSegmentBehavior(groupId, behavior, param1, param2, param3, param4)
end



function _M.begin()
	local leds =  configloader.Get().leds

	if not leds then 
		generic.displaySplashMessage("No leds defined")
		delay(1000)
		return
	end

	if not leds.groups then 
		error("Leds should be defined with 'groups'")
	end

	if #leds.groups == 0 then  
		generic.displaySplashMessage("No leds defined")
		delay(1000)
		return
	end
	if #leds.groups >= MAX_LED_GROUPS then  
		error("Maximum number os led groups allowed are "..MAX_LED_GROUPS)
		return
	end

	local groupOffsets = {}


	local countLeft = 0
	local countRight = 0
	local totalLeds = 0

	for i,b in pairs(leds.groups) do  
		if b.pin_side == 'right' and leds.pin_mode == 'single' then  
			error("Led group "..(i-1).." has pin_side right, but pin_mode is single.")
		end
		local ledC = tonumber(b.led_count)
		if not ledC or ledC == 0 then  
			error("Led group "..(i-1).." has invaid led_count value. Only positive numbers allowed")
		end

		if b.pin_side == 'right' then  
			countRight = countRight + ledC
		elseif b.pin_side == 'left' then  
			countLeft = countLeft + ledC

		else 
			error("Led group "..(i-1).." has invalid pin_side value. Allowed: left, right")
		end
		local prev = totalLeds
		totalLeds = totalLeds + ledC

		groupOffsets[i] = {prev, totalLeds-1}
		
	end

	if leds.pin_mode == 'single' then
		ledsBegin(countLeft, 0)
	elseif leds.pin_mode == 'double' then 
		ledsBeginDual(countLeft, countRight, 0) 
	else
		error("Invalid led mode "..leds.mode)
	end

    ledsDisplay()
    for i,b in pairs(leds.groups) do
    	log('Led segment '..(i-1).." range is "..groupOffsets[i][1].." to "..groupOffsets[i][2])
    	ledsSegmentRange(i-1, groupOffsets[i][1], groupOffsets[i][2])
    	_M.loadConfig(i-1, b) 
    end
   

       
end


return _M