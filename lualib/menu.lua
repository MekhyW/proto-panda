MODE_MAIN_MENU = 0
MODE_FACE_MENU = 1
MODE_SETTINGS_MENU = 2
MODE_FACE_QUICK = 3
MODE_CHANGE_PANEL_BRIGHTNESS = 4
MODE_CHANGE_LED_BRIGHTNESS = 5
MODE_SCRIPTS = 6

local scripts = require("scripts")
local ui = require("ui")
local generic = require("generic")

MAX_INTERFACE_ICONS = 5
MENU_SPACING = 13

local _M = {
    selected = 0,
    timer = 0,
    quit_timer=0,
    mode = MODE_MAIN_MENU,
    displayTime = 2000,
    settings={},
    maxTextShowSize = 20,
    shader = false
}


function _M.setup(expressions)
    _M.brigthness = tonumber(dictGet("panel_brightness")) or 64
    _M.led_brightness = tonumber(dictGet("led_brightness")) or 64
    _M.settings_icon = oledCreateIcon({0x00, 0x00, 0x16, 0x80, 0x3f, 0xc0, 0x7f, 0xe0, 0x39, 0xc0, 0x70, 0xe0, 0x70, 0xe0, 0x39, 0xc0, 0x7f, 0xe0, 0x3f, 0xc0, 0x16, 0x80, 0x00, 0x00}, 12, 12)
    _M.face_icon = oledCreateIcon({0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x21, 0xc0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x20, 0x15, 0x40, 0x2a, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 12, 12)
    _M.enterMainMenu()
    _M.face_selection_style = dictGet("face_selection_style") == "GRID" and "GRID" or "QUICK"

    --settings ui
    _M.settings = ui.generateUi("Press < To back", nil, _M.enterMainMenu)

    _M.settings.addElement(function() return "Rainbow ["..(_M.shader and "ON" or "OFF").."]" end, function()
        _M.shader = not _M.shader
        setRainbowShader(_M.shader)
    end)

    _M.settings.addElement(function() return "Panel Brightness [".._M.brigthness.."]" end, _M.enterPanelBrightnessMenu)

    _M.settings.addElement(function() return "Panel Brightness [".._M.led_brightness.."]" end, _M.enterLedBrightnessMenu)

    _M.settings.addElement(function() return "Faces [".._M.face_selection_style.."]" end,  function()
        if _M.face_selection_style == "GRID" then 
            _M.face_selection_style = "QUICK"
        else
            _M.face_selection_style = "GRID"
        end
        dictSet("face_selection_style", _M.face_selection_style)
        dictSave()
    end)

    _M.settings.addElement(function() return "Rebuild bulk file" end,  function()
        setPanelManaged(false)
        ledsSetBrightness(0)
        setPanelBrightness(0)
        panelPowerOff()
        noTone() --Since the beep is managed by the same core thats running the composing, the beep wouldn't stop. So we kill it right now
        composeBulkFile()
        generic.displayWarning("Restart", "System will reboot", 2000)
        restart()
    end)
    
    _M.settings.addElement(function() return "Reset default" end,  function()
        dictFormat()
        _M.face_selection_style = "GRID"
        _M.brigthness = 64
        _M.led_brightness = 64
        dictSet("face_selection_style", "GRID")
        dictSet("led_brightness", 64)
        dictSet("panel_brightness", 64)
        ledsGentlySeBrightness(_M.led_brightness)
        gentlySetPanelBrightness(_M.brigthness)
        dictSave()
        _M.enterMainMenu()
    end)
    --scripts ui
    local scriptList = scripts.GetScripts()
    _M.scripts = ui.generateUi("Press < To back", nil, _M.enterMainMenu)
    for i,b in pairs(scriptList) do  
        local name = scriptList[i].name
        _M.scripts.addElement(function() return name end,  function(idx)
            scripts.StartScript(idx) 
        end)
    end

end

function _M.enterMainMenu()
    _M.mode = MODE_MAIN_MENU
    _M.selected = 0
end

function _M.enterSettingMenu()
    _M.mode = MODE_SETTINGS_MENU
    _M.settings.onEnter()
end

function _M.enterScriptsMenu()
    _M.mode = MODE_SCRIPTS
    _M.scripts.onEnter()
end

function _M.enterPanelBrightnessMenu()
    _M.mode = MODE_CHANGE_PANEL_BRIGHTNESS
end

function _M.enterLedBrightnessMenu()
    _M.mode = MODE_CHANGE_LED_BRIGHTNESS
end

function _M.enterFaceMenu()
    if _M.face_selection_style == "GRID" then
        _M.mode = MODE_FACE_MENU
    else 
        _M.mode = MODE_FACE_QUICK
    end
    _M.quit_timer = 2000
    _M.selected = 1
end


function _M.draw()
    oledClearScreen()

    if _M.mode == MODE_MAIN_MENU then
        if _M.selected == 0 then
            oledDrawRect(67, 0, 60, 15, 1)
        elseif _M.selected == 1 then
            oledDrawRect(67, 15, 60, 15, 1)
        else 
            oledDrawRect(67, 31, 60, 15, 1)
        end
        oledSetCursor(68, 3)
        oledDrawText("Faces")
        oledSetCursor(68, 16)
        oledDrawText("Setting")
        oledSetCursor(68, 32)
        oledDrawText("Scripts")
        oledDrawRect(111, 1, 13, 13, 1)
        oledDrawRect(111, 16, 13, 13, 1)
        oledDrawRect(111, 33, 13, 13, 1)

        oledDrawIcon(112, 16, _M.settings_icon)
        oledDrawIcon(112, 32, _M.settings_icon)
        oledDrawIcon(112, 2, _M.face_icon)
        oledFaceToScreen(0, 0) 
        oledDrawBottomBar()
        oledDisplay()
    elseif _M.mode == MODE_FACE_QUICK then
        local id = expressions.GetCurrentExpressionId()
        oledSetCursor(0, 1)
        if id and id > 0 then 
            local aux = expressions.GetExpression(id)
            if aux then
                oledDrawText(aux.name)
            else 
                oledDrawText("Error?")
            end
        else 
            oledDrawText("Custom?")
        end
        oledFaceToScreen(32, 14)
        oledDrawBottomBar()
        if (_M.quit_timer < 2000) then 
            local rad =  - ((_M.quit_timer-2000) / 2000 ) * 128
            oledDrawFilledCircle(64,32 , rad, 0)
            oledDrawCircle(64,32 , rad, 1)
            local sin = math.sin(math.rad(45)) * rad
            local cos = math.cos(math.rad(45)) * rad
            oledDrawLine(64,32, 64 + sin, 32 + cos,1)
            oledDrawLine(64,32, 64 + sin, 32 - cos,1)
            oledDrawLine(64,32, 64 - sin, 32 - cos,1)
            oledDrawLine(64,32, 64 - sin, 32 + cos,1)
        end
        oledDisplay()
    elseif _M.mode == MODE_FACE_MENU then
        local xOffset = 0
        local yOffset = 0
        local skipIdx = 0
        local count = (MAX_INTERFACE_ICONS*2)
        if _M.selected > count then  
            local steps = _M.selected 
            while steps > count do 
                skipIdx = skipIdx + count
                steps = steps - count
            end
        end

        for idx, name in pairs(expressions.GetExpressions()) do 
            local realIdx = idx
            idx = idx - skipIdx
            if idx > 0 then
                oledSetCursor(1 + xOffset, 1 + (idx-1) * MENU_SPACING + yOffset)
                if #name > 10 then 
                    name = name:sub(1, 10)
                end
                oledDrawRect( 1 + xOffset, (idx-1) * MENU_SPACING + yOffset, 60, 10, 1)
                if _M.selected == realIdx then 
                    oledDrawRect( 0 + xOffset, (idx-1) * MENU_SPACING  - 1 + yOffset, 62, 12, 1)
                    oledDrawRect( 2 + xOffset, (idx-1) * MENU_SPACING  + 1 + yOffset, 58, 8, 1)
                end
                oledDrawText(name)
                if idx%MAX_INTERFACE_ICONS == 0 then 
                    xOffset = xOffset + 64
                    yOffset = yOffset - MENU_SPACING * MAX_INTERFACE_ICONS
                    if xOffset >= 127 then 
                        break
                    end
                end
            end
        end

        if _M.timer > 0 then
            local ratio = (_M.timer / _M.displayTime) * 64
            oledDrawRect(30, 13, 68, 3, 1)
            oledDrawLine(30, 14, 30 + ratio, 15, 1)
            oledDrawLine(30, 15, 30 + ratio, 15, 1)
            oledFaceToScreen(32, 16)
        end

        if (_M.quit_timer < 2000) then 
            local rad =  - ((_M.quit_timer-2000) / 2000 ) * 128
            oledDrawFilledCircle(64,32 , rad, 0)
            oledDrawCircle(64,32 , rad, 1)
            local sin = math.sin(math.rad(45)) * rad
            local cos = math.cos(math.rad(45)) * rad
            oledDrawLine(64,32, 64 + sin, 32 + cos,1)
            oledDrawLine(64,32, 64 + sin, 32 - cos,1)
            oledDrawLine(64,32, 64 - sin, 32 - cos,1)
            oledDrawLine(64,32, 64 - sin, 32 + cos,1)
        end

        oledDisplay()
    elseif _M.mode == MODE_SETTINGS_MENU then
        _M.settings.draw()
    elseif _M.mode == MODE_CHANGE_PANEL_BRIGHTNESS then 
        oledSetCursor(8, 2)
        local percent = (_M.brigthness/255) * 1000
        oledSetFontSize(2)
        oledDrawText("Brightness\n"..(math.floor(percent)/10).."%")
        oledSetFontSize(1)
        oledSetCursor(48, 64-8)
        oledDrawText("< +  [OK] - >")
        oledDisplay()
    elseif _M.mode == MODE_CHANGE_LED_BRIGHTNESS then 
        oledSetCursor(8, 2)
        local percent = (_M.led_brightness/255) * 1000
        oledSetFontSize(2)
        oledDrawText("Brightness\n"..(math.floor(percent)/10).."%")
        oledSetFontSize(1)
        oledSetCursor(48, 64-8)
        oledDrawText("< +  [OK] - >")
        oledDisplay()
    elseif _M.mode == MODE_SCRIPTS then 
        _M.scripts.draw()
    end
end


function _M.handleMenu(dt)
    --Clear internal screen

    if _M.mode == MODE_MAIN_MENU then 
        _M.handleMainMenu(dt)
    elseif _M.mode == MODE_FACE_MENU then 
        _M.handleFaceMenu(dt)
    elseif _M.mode == MODE_SETTINGS_MENU then 
        if not _M.settings.handle(dt) then 
            return
        end  
    elseif _M.mode == MODE_FACE_QUICK then 
        _M.handleFaceQuickMenu(dt)
    elseif _M.mode == MODE_CHANGE_PANEL_BRIGHTNESS then 
        _M.handleBrightnessMenu(dt)
    elseif _M.mode == MODE_CHANGE_LED_BRIGHTNESS then 
        _M.handleLedBrightnessMenu(dt)
    elseif _M.mode == MODE_SCRIPTS then 
        if not _M.scripts.handle(dt) then 
            return
        end    
    end

    _M.draw()
end


function _M.handleMainMenu()
    if readButtonStatus(BUTTON_DOWN) == BUTTON_JUST_PRESSED then 
        _M.selected = _M.selected +1
        toneDuration(540, 100)
         if _M.selected > 2 then  
            _M.selected = 0
        end
    end
    if readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then 
        toneDuration(440, 100)
        if _M.selected == 0 then
            _M.enterFaceMenu()
        elseif _M.selected == 1 then 
            _M.enterSettingMenu()
        elseif _M.selected == 2 then 
            _M.enterScriptsMenu()
        end

        return
    end

    if readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED then 
        _M.selected = _M.selected -1
        toneDuration(340, 100)
        if _M.selected < 0 then  
            _M.selected = 2
        end
    end
end

function _M.handleBrightnessMenu(dt)
    if readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then 
        _M.enterSettingMenu()
        dictSet("panel_brightness", tostring(_M.brigthness))
        dictSave()
        return
    end
    _M.timer = _M.timer - dt

    if readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED then
        if (_M.timer < 0) then 
            _M.timer = 10
            toneDuration(340, 10)
            _M.brigthness = _M.brigthness - 1
            if (_M.brigthness  < 0) then 
                _M.brigthness  = 0
            end
            
            setPanelBrightness(_M.brigthness)
        end
    end

    if readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED then
        if (_M.timer < 0) then 
            _M.timer = 10
            toneDuration(540, 10)
            _M.brigthness = _M.brigthness + 1
            if (_M.brigthness  > 255) then 
                _M.brigthness  = 255
            end

            setPanelBrightness(_M.brigthness)
        end
    end
end

function _M.handleLedBrightnessMenu(dt)
    if readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_PRESSED then 
        _M.enterSettingMenu()
        dictSet("led_brightness", tostring(_M.led_brightness))
        dictSave()
        return
    end
    _M.timer = _M.timer - dt

    if readButtonStatus(BUTTON_LEFT) == BUTTON_PRESSED then
        if (_M.timer < 0) then 
            _M.timer = 10
            toneDuration(340, 10)
            _M.led_brightness = _M.led_brightness - 1
            if (_M.led_brightness  < 0) then 
                _M.led_brightness  = 0
            end
            ledsSetBrightness(_M.led_brightness)
        end
    end

    if readButtonStatus(BUTTON_RIGHT) == BUTTON_PRESSED then
        if (_M.timer < 0) then 
            _M.timer = 10
            toneDuration(540, 10)
            _M.led_brightness = _M.led_brightness + 1
            if (_M.led_brightness  > 255) then 
                _M.led_brightness  = 255
            end
            ledsSetBrightness(_M.led_brightness)
        end
    end
end


function _M.handleFaceQuickMenu(dt)
    if readButtonStatus(BUTTON_LEFT) == BUTTON_JUST_PRESSED then
        expressions.Previous()
        toneDuration(340, 10)
    end

    if readButtonStatus(BUTTON_RIGHT) == BUTTON_JUST_PRESSED then
        expressions.Next()
        toneDuration(540, 10)
    end

    if readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED then 
        _M.quit_timer = _M.quit_timer - dt 
        if (_M.quit_timer <= 0) then 
            _M.enterMainMenu()
            toneDuration(440, 10)
            return
        end
    else 
        _M.quit_timer = 3000
    end
end


function _M.handleFaceMenu(dt)
    _M.timer = _M.timer - dt
    if readButtonStatus(BUTTON_LEFT) == BUTTON_JUST_PRESSED then 
        if _M.selected < MAX_INTERFACE_ICONS then
            toneDuration(340, 10)
            local maxExpressions = expressions.GetExpressionCount()
            local offsetBy5 = _M.selected%MAX_INTERFACE_ICONS
            _M.selected = maxExpressions-(MAX_INTERFACE_ICONS-offsetBy5)
        else
            _M.selected = _M.selected-MAX_INTERFACE_ICONS
        end
    end

    if readButtonStatus(BUTTON_RIGHT) == BUTTON_JUST_PRESSED then 
        local maxExpressions = expressions.GetExpressionCount()
        toneDuration(540, 10)
        _M.selected = _M.selected+MAX_INTERFACE_ICONS
        if _M.selected > maxExpressions then 
            _M.selected = _M.selected%MAX_INTERFACE_ICONS
        end
    end

    if readButtonStatus(BUTTON_UP) == BUTTON_JUST_PRESSED then 
        toneDuration(540, 10)
        if (_M.selected%MAX_INTERFACE_ICONS == 1) then 
            _M.selected = _M.selected +(MAX_INTERFACE_ICONS-1)
        else
            _M.selected = _M.selected -1
        end
    end
    if readButtonStatus(BUTTON_DOWN) == BUTTON_JUST_PRESSED then 
        toneDuration(340, 10)
        if (_M.selected%MAX_INTERFACE_ICONS == 0) then 
            _M.selected = _M.selected -(MAX_INTERFACE_ICONS-1)
        else 
            _M.selected = _M.selected +1
        end
    end

    if readButtonStatus(BUTTON_CONFIRM) == BUTTON_PRESSED then 
        _M.quit_timer = _M.quit_timer - dt 
        if (_M.quit_timer <= 0) then 
            _M.enterMainMenu()
            return
        end
    else 
        _M.quit_timer = 3000
    end

    if readButtonStatus(BUTTON_CONFIRM) == BUTTON_JUST_RELEASED then 
        local exps = expressions.GetExpressions()
        expressions.SetExpression(exps[_M.selected])
        _M.timer = _M.displayTime
        toneDuration(440, 10)

    end

end



return _M