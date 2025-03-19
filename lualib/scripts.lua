local _M = {
    previous_state = {},
    running_script = false
}

local json = dofile("/lualib/json.lua")

function _M.Load(filepath)
    local fp, err = io.open(filepath, "r")
    if not fp then 
        error("Failed to load "..filename..": "..tostring(err))
    end
    local content = fp:read("*a")
    fp:close()
    json.filename = filename
    _M.scripts = json.decode(content)
    for a,c in pairs(_M.scripts) do  
    	dofile(c.file)
    end
end

function _M.StoreState() 
	_M.previous_state[#_M.previous_state+1] = {
		managed = isPanelManaged(),
		brightness = getPanelBrighteness()
	}
end

function _M.GetScripts() 
	return _M.scripts
end

function _M.PopState() 
	local res = _M.previous_state[#_M.previous_state]
	if not res then 
		return
	end
	setPanelManaged(res.managed)
	setPanelBrighteness(res.brightness)
	_M.previous_state[#_M.previous_state] = nil
end

function _M.Handle(dt) 
	if not _M.running_script then 
		return true
	end

	local success, res = pcall(_M.running_script.onLoop, dt)
	if not success or _M.running_script.shouldStop then 
		if not success then
			print("Error running script: "..tostring(res))
		end
		_M.running_script.onClose()
		_M.running_script = nil
		_M.PopState() 
	end

	return false	
end

function _M.StartScript(id) 
	local res = _M.scripts[id]
	if not res then  
		return false
	end
	_M.StoreState() 
	local data = dofile(res.file)
	_M.running_script = data
	_M.running_script.onSetup(res)
end


return _M