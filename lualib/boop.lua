local expressions = require("expressions")
local GetCurrentFrame = expressions.GetCurrentFrame
local StackExpression = expressions.StackExpression
local IsFrameFromAnimation = expressions.IsFrameFromAnimation

local _M = {
    isOnLidar = false,
    config = {
       triggerStart = 100,
       triggerStop = 150,
    },
    readyToTrigger  = false,
    isOnLidar = false,
    proximityTimer = 500
}

function _M.Load(filename)
    local fp, err = io.open(filename, "r")
    if not fp then 
        error("Failed to load "..filename..": "..tostring(err))
    end
    local content = fp:read("*a")
    fp:close()
    json.filename = filename
    _M.config = json.decode(content)
    if type(_M.config["triggerStart"]) ~= "number" then 
        error("Field 'triggerStart' should be an number")
    end
    if type(_M.config["triggerStop"]) ~= "number" then 
        error("Field 'triggerStop' should be an number")
    end

    if _M.config["transitionIn"] then
        if type(_M.config["transitionIn"]) ~= "string" then 
            error("Field 'triggerStop' should be an string")
        end
        local exp = expressions.GetExpression(_M.config["transitionIn"])
        if not exp then 
            error("On boop 'transitionIn' is defined the ".._M.config["transitionIn"].." but thats not a valid expression")
        end
        if not exp.transition then 
            error("On boop 'transitionIn' the animation should have transition=true")
        end
    end

    if _M.config["transitionOut"] then
        if type(_M.config["transitionOut"]) ~= "string" then 
            error("Field 'triggerStop' should be an string")
        end
        local exp = expressions.GetExpression(_M.config["transitionOut"])
        if not exp then 
            error("On boop 'transitionOut' is defined the ".._M.config["transitionOut"].." but thats not a valid expression")
        end
        if not exp.transition then 
            error("On boop 'transitionOut' the animation should have transition=true")
        end
    end

    if _M.config["boopAnimation"] then
        if type(_M.config["boopAnimation"]) ~= "string" then 
            error("Field 'boopAnimation' should be an string")
        end
        local exp = expressions.GetExpression(_M.config["boopAnimation"])
        if not exp then 
            error("On boop 'boopAnimation' is defined the ".._M.config["boopAnimation"].." but thats not a valid expression")
        end
        if exp.transition then 
            error("On boop 'boopAnimation' the animation should have transition=false")
        end
    end
    if _M.config["transictionOnlyOnAnimation"] then
        if type(_M.config["transictionOnlyOnAnimation"]) ~= "string" then 
            error("Field 'transictionOnlyOnAnimation' should be an string")
        end
        local exp = expressions.GetExpression(_M.config["transictionOnlyOnAnimation"])
        if not exp then 
            error("On boop 'transictionOnlyOnAnimation' is defined the ".._M.config["transictionOnlyOnAnimation"].." but thats not a valid expression")
        end
        if exp.transition then 
            error("On boop 'transictionOnlyOnAnimation' the animation should have transition=false")
        end
    end

    if _M.config["transictionInOnlyOnSpecificFrame"] then
        if type(_M.config["transictionInOnlyOnSpecificFrame"]) ~= "number" then 
            error("Field 'transictionInOnlyOnSpecificFrame' should be an number")
        end
    end 
    
    if _M.config["triggerMinIgnore"] then
        if type(_M.config["triggerMinIgnore"]) ~= "number" then 
            error("Field 'triggerMinIgnore' should be an number")
        end
    else
        _M.config.triggerMinIgnore = 40
    end

    if _M.config["transictionOutOnlyOnSpecificFrame"] then
        if type(_M.config["transictionOutOnlyOnSpecificFrame"]) ~= "number" then 
            error("Field 'transictionOutOnlyOnSpecificFrame' should be an number")
        end
    end
end

function _M.reset()
    _M.isOnLidar = false
end

function _M.manageBoop(dt)
    if hasLidar() then 
        local config = _M.config
        local distance = readLidar()

        if not _M.isOnLidar then 
            if distance <= config.triggerStart and (distance >= config.triggerMinIgnore or distance >= config.triggerStop) then 
                _M.proximityTimer = _M.proximityTimer - dt  
                if _M.proximityTimer < 0 then  
                    _M.readyToTrigger = true
                end
            else 
                _M.proximityTimer = 500
                _M.readyToTrigger = false
            end
            if _M.readyToTrigger then 
                local isOnCorrectFrame = true
                if config.transictionOnlyOnAnimation then  
                    if not IsFrameFromAnimation(getPanelCurrentFace(), config.transictionOnlyOnAnimation)  then  
                        isOnCorrectFrame = false
                    end
                end
                if config.transictionInOnlyOnSpecificFrame then 
                    if config.transictionInOnlyOnSpecificFrame ~= GetCurrentFrame() then  
                        isOnCorrectFrame = false
                    end
                end

                if isOnCorrectFrame then
                    _M.isOnLidar = true
                    if config.transitionOut then
                        StackExpression(config.transitionOut)
                    end
                    StackExpression(config.boopAnimationName)
                    if config.transitionIn then
                        StackExpression(config.transitionIn)
                    end
                end
            end
        elseif distance >= config.triggerStop and _M.isOnLidar then
            _M.readyToTrigger = false
            _M.proximityTimer = 500
            if IsFrameFromAnimation(getPanelCurrentFace(), config.boopAnimationName)  then  
                if config.transictionOutOnlyOnSpecificFrame then 
                    if config.transictionOutOnlyOnSpecificFrame ~= GetCurrentFrame() then  
                        return
                    end
                end
                _M.isOnLidar = false   
                popPanelAnimation()       
            end
        end
    end
end

return _M