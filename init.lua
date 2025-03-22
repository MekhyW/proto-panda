local expressions = require("expressions")
local scripts = require("scripts")
local generic = require("generic")
local boop = require("boop")
local menu = require("menu")

local brightness

function onSetup()
    setPoweringMode(POWER_MODE_USB_5V)
    dictLoad()
    startBLE()
    acceptBLETypes("d4d31337-c4c6-c2c3-b4b3-b2b1a4a3a2a1", "d4d3afaf-c4c6-c2c3-b4b3-b2b1a4a3a2a1")
    beginBleScanning()
    brightness = tonumber(dictGet("panel_brightness")) or 64
    print("Panel is: "..dictGet("panel_brightness").." led is "..dictGet("led_brightness"))
    setPanelMaxBrighteness( brightness )

    expressions.Load("/expressions.json") 
    scripts.Load("/scripts.json") 


    ledsBeginDual(25, 25, tonumber(dictGet("led_brightness") ) or 64) 
    ledsSegmentRange(0, 0, 24)
    ledsSegmentRange(1, 25, 49)
    ledsSegmentBehavior(0, BEHAVIOR_PRIDE) 
    ledsSegmentBehavior(1, BEHAVIOR_PRIDE) 
    menu.setup()
    panelPowerOn()
end

function onPreflight()
    gentlySetPanelBrighteness(brightness)
    setPanelManaged(true)
    expressions.Next()
end

function onLoop(dt)
    boop.manageBoop()
    if not scripts.Handle(dt) then
        return
    end
    menu.handleMenu(boop, dt)
end

