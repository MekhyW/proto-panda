local _M = {
	var = "sussy"
}

function _M.map(x, in_min, in_max, out_min, out_max) 
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end


return _M