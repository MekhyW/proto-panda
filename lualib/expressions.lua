local configloader = require("configloader")

local _M = {
	animations = {},
	by_name = {},
	by_frame = {},
	count = 0,
}

function _M.Load()
	content = nil
	local conf = configloader.Get()
	for id ,b in pairs(conf.expressions) do 
		local offset = nil 
		if b.frame_offset and type(b.frames) == "number" then 
			offset = b.frame_offset
		elseif b.frames and type(b.frames) == "string" then 
			offset = getFrameOffsetByName(b.frames)
		end
		if offset then 
			b.frame_offset = offset
			local animation = b.animation
			if type(b.animation) ~= 'table' then  
				if b.animation == "auto" then  
					b.animation = {}
					animation = b.animation
					if not b.frames then  
						error("Cannot use 'frames=auto' when there is no alias defined")
					end
					local count = getFrameCountByName(b.frames)
					if count == 0 then  
						error("Using frame group '"..b.frames.."' returned 0 frames. Are you sure this alias has loaded frames?")
					end
					for i=1,count do  
						b.animation[i] = i
					end
				else 
					error("Animation can only e numeric array or 'auto'")
				end
			end
			for a,c in pairs(b.animation) do 
				animation[a] = c + offset
				_M.by_frame[c + offset] = id
			end
			b.animation = animation
		end
		b.frame_offset = b.frame_offset or 0
		b.duration = tonumber(b.duration) or 250
		b.name = b.name or "expression "..id

		if b.onEnter then 
			local f = load(b.onEnter)
			if not f then 
				error("Error loading 'onEnter', contains syntax error")
			end
			b.onEnter = f
		end
		if b.onLeave then 
			local f = load(b.onLeave)
			if not f then 
				error("Error loading 'onLeave', contains syntax error")
			end
			b.onLeave = f
		end

		_M.by_name[b.name] = id
		if not b.transition then 
			_M.count = _M.count+1
		end
	end
	_M.animations = conf.expressions
	return true
end

function _M.GetExpression(name)
	if type(name) == 'string' then 
		name = _M.by_name[name] or name
	end
	return _M.animations[name] 
end


function _M.GetExpressionName(name)
	return _M.by_name[name]
end

function _M.GetCurrentExpressionId()
	local storage = getCurrentAnimationStorage()
	if storage == -1 then
		local frameId = getPanelCurrentFace()
		local animId = _M.by_frame[frameId]
		if not animId then 
			return -1, frameId
		end
		return animId, frameId
	else 
		return storage, getPanelCurrentFace()	
	end
end

function _M.GetCurrentFrame()
	local expId, frameId = _M.GetCurrentExpressionId()
	local exp = _M.animations[expId]
	if not exp then 
		return -1
	end
	return frameId - (exp.frame_offset or 0)
end

function _M.GetAnimationNameByFace(faceid)
	local idx = _M.by_frame[faceid]
	if not idx then 
		return "none"
	end
	local aux = _M.animations[idx]
	if not aux then 
		return "none"
	end
	return aux.name
end


function _M.Next(id)
	if not id then
		id = _M.GetCurrentExpressionId()
	end
	if (id == -1) then 
		id = 0
	end
	id = id +1 
	if id > #_M.animations then 
		id = 1
	end
	if _M.animations[id] and _M.animations[id].transition then 
		return _M.Next(id)
	end
	return _M.SetExpression(id)
end

function _M.Previous(id)
	if not id then
		id = _M.GetCurrentExpressionId()
	end
	if (id == -1) then 
		id = #_M.animations+1
	end
	id = id -1 
	if id <= 0 then 
		id = #_M.animations
	end
	if _M.animations[id] and _M.animations[id].transition then 
		return _M.Previous(id)
	end
	return _M.SetExpression(id)
end


function _M.SetExpression(id)
	local aux = _M.GetExpression(id)
	if aux then 
		local repeats = aux.repeats or -1
		local allDrop = true
		if aux.transition then 
			repeats = 1
			allDrop = false
		end
		if _M.previousExpression and _M.previousExpression.onLeave then 
			_M.previousExpression.onLeave()
		end
		if tonumber(id) then
			setPanelAnimation(aux.animation, aux.duration, repeats, allDrop, tonumber(id))
		else 
			setPanelAnimation(aux.animation, aux.duration, repeats, false)
		end
		if aux.onEnter then 
			aux.onEnter()
		end
		if aux.intro then
			_M.StackExpression(aux.intro)
		end
		if _M.previousExpression and _M.previousExpression.outro then  
			_M.StackExpression(_M.previousExpression.outro)
		end
		_M.previousExpression = aux
	end
end

function _M.IsFrameFromAnimation(frameId, id)
	local aux = _M.GetExpression(id)
	if aux then 
		for i,b in pairs(aux.animation) do 
			if b == frameId then
				return true
			end
		end
	end
	return false
end

function _M.StackExpression(id)
	local aux = _M.GetExpression(id)
	if aux then 
		local repeats = aux.repeats or -1
		if aux.transition then 
			repeats = 1
		end
		if tonumber(id) then
			setPanelAnimation(aux.animation, aux.duration, repeats, false, tonumber(id))
		else 
			setPanelAnimation(aux.animation, aux.duration, repeats, false)
		end
	else
		print("Unknown ID: "..tostring(id))
	end
end

function _M.GetExpressions()
	local res = {}
	for __, data in pairs(_M.animations) do 
		if not data.transition then
			res[#res+1] = data.name
		end
	end
	return res
end

function _M.GetExpressionCount()
	return _M.count
end


return _M