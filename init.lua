local expressions = require("expressions")
local models = require("models")
local scripts = require("scripts")
local generic = require("generic")
local menu = require("menu")
local boop = require("boop")
local configloader = require("configloader")
local drivers = require("drivers")
local input = require("input")
local leds = require("leds")
local fft = require("fft")
--local overlays = require("overlays")


function onSetup()

    dictLoad()

    local seed = tonumber(dictGet("random_seed")) or millis()
    seed = seed + millis()
    math.randomseed(seed)
    print("Random seed is "..seed)
    dictSet("random_seed", tostring(seed))
    if dictGet("created") ~= "1" then
        menu.setDictDefaultValues()
    end
    dictSave()
    

    configloader.Load()
    input.Load()
    models.Load()
    expressions.Load() 
    scripts.Load() 
    boop.Load()
    generic.displaySplashMessage("Starting:\nFFT")
    fft.load()
    generic.displaySplashMessage("Starting:\nLeds")
    leds.begin()
    generic.displaySplashMessage("Starting:\nMenu") 
    menu.setup()
    generic.displaySplashMessage("Starting:\nOverlays") 
    overlays.setup()

end

function onPreflight()
    ledsSetManaged(true)
    setPanelManaged(true)
    expressions.Next()
    if not configloader.Get().starting_animation then
        expressions.SetExpression(configloader.Get().starting_animation)
    end

    input.Start() 
    setPoweringMode(BUILT_IN_POWER_MODE)
    ledsGentlySeBrightness(tonumber(dictGet("led_brightness") ) or 64)
    gentlySetPanelBrightness(tonumber(dictGet("panel_brightness")) or 64)


end

function onLoop(dt)
    overlays.update(dt)
    drivers.update()
    input.update()
    expressions.update()
    if not scripts.Handle(dt) then
        return
    end
    menu.handleMenu(dt)
end
