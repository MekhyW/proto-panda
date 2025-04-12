local _M = {
    previous_state = {},
    running_script = false
}


local json = require("json")
local generic = require("generic")

function _M.parseVersion(version)
	local release, major, minor = version:match("(%d+)%.(%d+)%.(%d+)")
	if not release then 
		release, major = version:match("(%d+)%.(%d+)")
		minor = 0
	end
	release = tonumber(release) or 0
	major = tonumber(major) or 0
	minor = tonumber(minor) or 0
	if release == 0 then 
		error("Invalid panda version: "..tostring(version))
	end
	return release, major, minor
end


function _M.canRun(version)
	local pandaRelease, pandaMajor, pandaMinor = _M.parseVersion(PANDA_VERSION)
	if not pandaRelease then
		error("Invalid panda version")
	end
	
	if not version then 
		return true, "No version defined"
	end

	local scriptRelease, scriptMajor, scriptMinor = _M.parseVersion(version)
	if not scriptRelease then 
		return false, "Failed to parse script version"
	end

	if scriptRelease > pandaRelease then 
		return false, "Require firmware release to be "..scriptRelease..", but got "..pandaRelease..". Required version "..version
	end

	if scriptMajor > pandaMajor then 
		return false, "Require firmware major to be "..scriptMajor..", but got "..pandaMajor..". Required version "..version
	end

	if scriptMinor > pandaMinor then 
		return false, "Require firmware minor to be "..scriptMinor..", but got "..pandaMinor..". Required version "..version
	end

	return true
end

function _M.Load(filepath)
    local fp, err = io.open(filepath, "r")
    if not fp then 
        error("Failed to load "..filename..": "..tostring(err))
    end
    local content = fp:read("*a")
    fp:close()
    json.filename = filename
    _M.scripts = json.decode(content)
    local auxScripts = {}
    for a,c in pairs(_M.scripts) do  
    	local success, data = pcall(dofile, c.file)
    	if not success then 
    		generic.displayWarning("On script: "..c.file, data)
    	else
    		local run, message = _M.canRun(data.VERSION_REQUIRED)
    		if message ~= nil then 
    			generic.displayWarning("On script: "..c.file, message)
    		end
    		if run then 
    			auxScripts[#auxScripts+1] = c
    		end 
    		data = nil
    	end
    end
    _M.scripts = auxScripts
    collectgarbage()
end

function _M.StoreState() 
	_M.previous_state[#_M.previous_state+1] = {
		panel_managed = isPanelManaged(),
		led_managed = ledsIsManaged(),
		led_brightness = ledsGetBrightness(),
		panel_brightness = getPanelBrightness(),
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
	setPanelManaged(res.panel_managed)
	ledsSetManaged(res.led_managed)

	if ledsGetBrightness() ~= res.led_brightness then
		ledsGentlySeBrightness(res.led_brightness, 1, ledsGetBrightness())
	end

	if getPanelBrightness() ~= res.panel_brightness then
		gentlySetPanelBrightness(res.led_brightness, 1, res.panel_brightness)
	end


	_M.previous_state[#_M.previous_state] = nil
end

function _M.Handle(dt) 
	if not _M.running_script then 
		return true
	end

	local success, res = pcall(_M.running_script.onLoop, dt)
	if not success or _M.running_script.shouldStop then 
		if not success then
			generic.displayWarning("Runtime error", res)
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
	oledClearScreen()
	oledDisplay()
	_M.running_script.onSetup(res)
end


return _M