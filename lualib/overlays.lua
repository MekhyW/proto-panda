local _M = {
    by_name = {},
    loaded = {},
    active = {},

    enabled = true,
    modes = {}
}



function _M.setup()
    local configloader = require("configloader")
    local overlays = configloader.Get().overlays
    if not overlays then  
        return
    end
    if type(overlays) ~= 'table' then  
        error("'overlays' should be a table")
    end

    for i,b in pairs(overlays) do  
        _M.loadSingleOverlay(i, b)
    end
end


function _M.loadSingleOverlay(id, data) 
    if type(data.name) ~= 'string' then
        error("Overlay "..id.. " need a string 'name'")
    end

    if type(data.elements) ~= 'table' then
        error("Overlay "..id.. " need a table 'elements'")
    end

    if _M.by_name[data.name] ~= nil then  
        error("Overlay "..id.. " has a duplicated name '"..data.name.."'")
    end

    local element = {
        objects = {}
    }

    for edi, elem in pairs(data.elements) do  
        if type(elem.sprites) ~= 'table' then 
            error("Overlay "..id.." at element "..edi.." requires a list of 'sprites'")
        end
        local el = {}
        local frames = 0
        el.sprite = Sprite()
        --Load sprites
        for sid, sprName in pairs(elem.sprites) do  
            if type(sprName) ~= 'string' then 
                error("Overlay "..id.." at element "..edi.." at sprite '"..sid.."' need to be a string and valid path")
            end
            if el.sprite:LoadFromPng(sprName) == -1 then  
                error("Overlay "..id.." at element "..edi.." at sprite '"..sid.."' failed to load '"..sprName.."'")
            end
            frames = frames +1
        end

        if type(elem.transparency_color) == 'number' then
            el.sprite:SetTransparencyColor(elem.transparency_color)
        end

        if type(elem.animation) ~= 'table' then
            error("Overlay "..id.." at element "..edi.." requires field 'animation'")
        end

        if not _M.modes[elem.animation.mode] then  
            error("Overlay "..id.." at element "..edi.." has a invalid animation mode")
        end

        if elem.animation.x and elem.animation.y then  
            el.sprite:SetPosition(elem.animation.x, elem.animation.y)
        end

        el.frames = frames
        el.mode = elem.animation.mode
        el.animation = elem.animation
        element.objects[edi] = el
    end

    element.id = id
    element.name = name
    _M.by_name[data.name] = element
    _M.loaded[id] = element
end


function _M.setEnabled(en)
    _M.enabled = en
    if _M.enabled then  
        for i,b in pairs(_M.active) do  
            for i, ob in pairs(b.objects) do  
                setOverlaySprite(ob.sprite)
            end
        end
    else
        clearAllOverlaySprites()
    end
end

_M.modes["fft"] = function(obj, dt)
    local energy = 0
    local setting = obj.animation
    local frameCount = obj.frames
    for b = setting.band_start, setting.band_end do
        energy = energy + getBandValueFft(b)
    end

    obj.mouthSmoothed = obj.mouthSmoothed or 0

    local tau = (energy > obj.mouthSmoothed) and (setting.attack or 0.05) or (setting.release or 0.2)
    local alpha = 1 - math.exp(-dt / tau)
    obj.mouthSmoothed = obj.mouthSmoothed * (1 - alpha) + energy * alpha

    local newLevel = 0
    if obj.mouthSmoothed > setting.frist_frame_threshold then
        local norm = (obj.mouthSmoothed - setting.min_energy) / (setting.max_energy - setting.min_energy)
        if norm < 0 then norm = 0 end
        if norm > 1 then norm = 1 end
        newLevel = math.floor(norm * frameCount)
        if newLevel > frameCount - 1 then newLevel = frameCount - 1 end
        if newLevel < 1 then newLevel = 1 end
    end

    obj.mouthLevel = newLevel
    obj.sprite:SetFrameId(obj.mouthLevel)
end


function _M.update(dt)
    for i,element in pairs(_M.active) do  
        for _, obj in pairs(element.objects) do
            _M.modes[obj.mode](obj, dt)
        end
    end
end


function _M.enableOverlay(name)
    local ov = _M.by_name[name]
    if not ov then  
        return false
    end

    forceRedrawEachFrame(true)

    _M.active[ov.id] = ov
    if _M.enabled then
        for i, ob in pairs(ov.objects) do  
            setOverlaySprite(ob.sprite)
        end
    end
end

function _M.disableOverlay(name)
    local ov = _M.by_name[name]

    if not ov then  
        return false
    end

    _M.active[ov.id] = nil
    for i, ob in pairs(ov.objects) do  
        clearOverlaySprite(ob.sprite)
    end

    local count = 0
    for i,b in pairs(_M.active) do 
        count = count +1
    end
    if count == 0 then 
        forceRedrawEachFrame(false)
    end

end

function _M.disableAllOverlays()
    for i, ob in pairs(_M.loaded) do  
        _M.disableOverlay(ob.name)
    end
end
    


return _M