local _M = {
    by_name = {},
    loaded = {},
    active = {},

    enabled = true,
    modes = {},
    starttup = {},
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

    for i,element in pairs(_M.loaded) do  
        for _, obj in pairs(element.objects) do 
            if _M.starttup[obj.mode] then  
                _M.starttup[obj.mode](obj)
            end
        end
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
        elseif type(elem.transparency_color) == 'string' then
            local hex = elem.transparency_color:gsub('#', '')
            local r = tonumber(hex:sub(1, 2), 16)
            local g = tonumber(hex:sub(3, 4), 16)
            local b = tonumber(hex:sub(5, 6), 16)

            if r and g and b then
                local col565 = color565(r, g, b)
                el.sprite:SetTransparencyColor(col565)
            end
        end
        if type(elem.behavior) ~= 'table' then
            error("Overlay "..id.." at element "..edi.." requires field 'behavior'")
        end

        if not _M.modes[elem.behavior.mode] then  
            error("Overlay "..id.." at element "..edi.." has a invalid behavior mode")
        end

        if elem.behavior.x and elem.behavior.y then  
            el.sprite:SetPosition(elem.behavior.x, elem.behavior.y)
        end

        el.frames = frames
        el.mode = elem.behavior.mode
        el.behavior = elem.behavior
        element.objects[#element.objects+1] = el
        el.id = #element.objects
        if elem.clones then 
            for i=1,elem.clones do
                local clone = {
                    frames=el.frames,
                    mode=el.mode,
                    behavior=el.behavior,
                }
                clone.sprite = el.sprite:Clone()
                element.objects[#element.objects+1] = clone
                clone.id = #element.objects
            end
        end
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

_M.starttup["random_flashing"] = function(obj)
    obj.nextSpawn = millis() + math.random(obj.behavior.interval_min or 10, obj.behavior.interval_max or 100)
    obj.sprite:setVisibility(false)
    obj.behavior.anim_speed = obj.behavior.anim_speed or 20
    obj.nextFrame = millis() + obj.behavior.anim_speed
    obj.alive = false
end

_M.starttup["frame_by_fft_level"] = function(element) end



_M.modes["random_flashing"] = function(obj, dt)
    local time = millis()
    
    if not obj.alive then
        if obj.nextSpawn < time then 
            obj.alive = true
            obj.sprite:SetPosition(math.random(obj.behavior.min_x,obj.behavior.max_x), math.random(obj.behavior.min_y,obj.behavior.max_y))
            obj.nextFrame = time + obj.behavior.anim_speed
            obj.sprite:setVisibility(true)
        end
    else
        if obj.nextFrame < time then  
            obj.nextFrame = time + obj.behavior.anim_speed
            if obj.sprite:NextFrame() == 0 then  
                obj.alive = false
                obj.sprite:setVisibility(false)
                obj.nextSpawn = millis() + math.random(obj.behavior.interval_min or 10, obj.behavior.interval_max or 100)
            end
        end
    end
    
end

_M.modes["frame_by_fft_level"] = function(obj, dt)
    local energy = 0
    local setting = obj.behavior
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