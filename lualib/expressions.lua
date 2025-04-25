local _M = {
	animations = {},
	by_name = {},
	by_frame = {},
	count = 0,
}

local json = require("json")

function _M.Load(filename)
	local fp, err = io.open(filename, "r")
	if not fp then 
		error("Failed to load "..filename..": "..tostring(err))
	end
	local content = fp:read("*a")
	fp:close()
	json.filename = filename
	local raw_expressions = json.decode(content)
	content = nil
	for id ,b in pairs(raw_expressions) do 
		local offset = nil 
		if b.frame_offset and type(b.use_alias) == "number" then 
			offset = b.frame_offset
		elseif b.use_alias and type(b.use_alias) == "string" then 
			offset = getFrameAliasByName(b.use_alias)
		end
		if offset then 
			b.frame_offset = offset
			local frames = b.frames
			if type(b.frames) ~= 'table' then  
				if not b.frames == "auto" then  
					b.frames = {}
					if not b.use_alias then  
						error("Cannot use 'frames=auto' when there is no alias defined")
					end
					local count = getFrameCountByAlias(b.use_alias)
					if count == 0 then  
						error("Using alias '"..b.use_alias.."' returned 0 frames. Are you sure this alias has loaded frames?")
					end
					for i=1,count do  
						b.frames[i] = i
					end
				end
			end
			for a,c in pairs(b.frames) do 
				frames[a] = c + offset
				_M.by_frame[c + offset] = id
			end
			b.frames = frames
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
	_M.animations = raw_expressions
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
			setPanelAnimation(aux.frames, aux.duration, repeats, allDrop, tonumber(id))
		else 
			setPanelAnimation(aux.frames, aux.duration, repeats, false)
		end
		if aux.onEnter then 
			aux.onEnter()
		end
		_M.previousExpression = aux
	end
end

function _M.IsFrameFromAnimation(frameId, id)
	local aux = _M.GetExpression(id)
	if aux then 
		for i,b in pairs(aux.frames) do 
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
			setPanelAnimation(aux.frames, aux.duration, repeats, false, tonumber(id))
		else 
			setPanelAnimation(aux.frames, aux.duration, repeats, false)
		end
	else
		print("Unknown ID: "..id)
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