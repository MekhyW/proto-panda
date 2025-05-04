local _M = {}

local json = require("json")

function _M.Load(filename)
	local fp, err = io.open(filename, "r")
	if not fp then 
		error("Failed to load "..filename..": "..tostring(err))
	end
	local content = fp:read("*a")
	fp:close()
	json.filename = filename
	_M.config = json.decode(content)

	if not _M.config.expressions or #_M.config.expressions == 0 then  
		error("Config is missing 'expressions'")
	end

	if not _M.config.scripts or #_M.config.scripts == 0 then  
		error("Config is missing 'scripts'")
	end

	return
end

function _M.Get()
	return _M.config
end

return _M