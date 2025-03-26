

local GetCurrentFrame = expressions.GetCurrentFrame
local StackExpression = expressions.StackExpression
local IsFrameFromAnimation = expressions.IsFrameFromAnimation

local _M = {
	isOnLidar = false,
	lidarTrigger = 100,
	lidarUntrigger = 150,
	isOnLidar = false,
}

function _M.manageBoop()
	if hasLidar() then 
        local distance = readLidar()
        if distance < _M.lidarTrigger then 
            if not _M.isOnLidar and GetCurrentFrame() == 1 then 
                _M.isOnLidar = true
                StackExpression("vergonha_transicao_out")
                StackExpression("vergonha")
                StackExpression("vergonha_transicao_in")
            end
        elseif distance >= _M.lidarUntrigger and _M.isOnLidar then
            
            if IsFrameFromAnimation(getPanelCurrentFace(), "vergonha")  then  
                _M.isOnLidar = false   
                popPanelAnimation()       
            end
        end
        oledSetCursor(0, 33)
        oledDrawText(distance.." mm")
    end
end

return _M