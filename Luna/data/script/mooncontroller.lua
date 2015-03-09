ID = "Script"

local function lorenz_attractor(x, y, z, dt)
	local p = 10;
	local r = 28;
	local b = 8/3;

	local dx = (-p*x + p*y) * dt; 
	local dy = (-x*z + r*x - y) * dt;
	local dz = ( x*y - b*z) * dt;

	return { x+dx, y+dy, z+dz };
end

local g_init_position = { 0.1, 0.1, 0.1 }
function get_state(scene_time_second)
	local p = g_init_position;
	new = lorenz_attractor(p[1], p[2], p[3], 0.0005)
	g_init_position = { new[1], new[2], new[3] }
	local elapsed = math.min(scene_time_second, 204-155)
	return { posx=0, posy=elapsed*0.278, posz=0 }
--	return {
--		posx = new[1], posy = new[2]+ (scene_time_second)*0.1, posz = new[3],
--	}

end
