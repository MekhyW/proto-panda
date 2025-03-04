local expressions = dofile("/lualib/expressions.lua")
local generic = dofile("/lualib/generic.lua")

_G.expressions = expressions

local boop = dofile("/boop.lua")
local menu = dofile("/menu.lua")


local brightness

function onSetup()
    setPoweringMode(POWER_MODE_USB_5V)
    dictLoad()
    startBLE()
    acceptBLETypes("d4d31337-c4c6-c2c3-b4b3-b2b1a4a3a2a1", "d4d3afaf-c4c6-c2c3-b4b3-b2b1a4a3a2a1")
    beginBleScanning()
    brightness = tonumber(dictGet("panel_brightness")) or 64
    print("Panel is: "..dictGet("panel_brightness"))
    setPanelMaxBrighteness( brightness )

    expressions.Load("/expressions.json") 


    ledsBegin(200, tonumber(dictGet("led_brightness") ) or 64) 
    ledsSegmentRange(0, 0, 4)
    ledsSegmentRange(1, 5, 7)
    ledsSegmentRange(2, 8, 8)
    ledsSegmentRange(3, 9, 199)
    ledsSegmentBehavior(0, 1) 
    ledsSegmentBehavior(1, BEHAVIOR_STATIC_RGB, 255, 100, 50)
    ledsSegmentBehavior(2, BEHAVIOR_STATIC_RGB, 30, 30, 50)
    ledsSegmentBehavior(3, BEHAVIOR_PRIDE) 
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
    menu.handleMenu(boop, dt)
end
