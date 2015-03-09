ID = "Script"


-- test
local function lorenz_attractor(x, y, z, dt)
	local p = 10;
	local r = 28;
	local b = 8/3;

	local dx = (-p*x + p*y) * dt; 
	local dy = (-x*z + r*x - y) * dt;
	local dz = ( x*y - b*z) * dt;

	return { x+dx, y+dy, z+dz };
end

local g_previous_position = { 0.01, 0.01, 0.01 }
local g_time = 0.0

local function move(scene_time_second)
	if scene_time_second < 5 then
		return { 0, -0, 0 } 
	elseif 155 < scene_time_second then
		local coord = math.max(math.min(scene_time_second - 169, 204), 0)
		return { 0, 1+coord * 0.278, 0 }
	end
	return { 0, 0, 0 }
end

local function greets_controller(scene_time_second)
	local elapsed = scene_time_second - 75
	return {
	 posx = elapsed * 0.3, --current[1]*0.5,
	 posy = 0, --math.sin(scene_time_second*2), --current[2]*0.5,
	 posz = 0,
	}
end

function get_initial_position()
	return g_previous_position;
end

function get_position(scene_time_second, scene_time_ratio) 
	local current = g_previous_position;
	g_previous_position = lorenz_attractor(
		g_previous_position[1],
		g_previous_position[2],
		g_previous_position[3], 0.005)
	local time = g_time
	g_time = g_time + 0.1

	pos = move(scene_time_second)

	if 75 < scene_time_second and scene_time_second < 104 then
		return greets_controller(scene_time_second)
	end

	return {
	 posx = pos[1], -- 0 + time * 0.3,
	 posy = pos[2], --1 + math.sin(time*0.5)*0.2,
	 posz = pos[3], 
	}
end

function get_pressure_sources(scene_time_second)
	if scene_time_second < 0 then
		return nil
	elseif scene_time_second < 8 then
		return {
		 { posx=0, posy=-1, posz=0, radius=0.2, pressure=1.0 },
		}
	elseif 47.5 < scene_time_second and scene_time_second < 74 then
		return {
		{ posx=0, posy=-1, posz=0, radius=0.5,pressure=0.5},
	}
	elseif 75 < scene_time_second and scene_time_second < 104 then
		local elapsed = scene_time_second - 75
		return {
		{ posx=0.5+elapsed*0.3, posy=math.sin(scene_time_second*2)*0.2,posz=0, radius=0.2,pressure=3},
	}
	elseif 108 < scene_time_second and scene_time_second < 145 then
		return {
		{ posx=0, posy=-1, posz=0, radius=0.5,pressure=0.5},		
		{ posx=-1, posy=-1, posz=-1, radius=0.5,pressure=0.5},		
		{ posx= 1, posy=-1, posz= 1, radius=0.5,pressure=0.5},		
	} 
	elseif 145 < scene_time_second and scene_time_second < 155 then
		local elapsed = scene_time_second - 145
		return {
		{ posx=0, posy=1, posz=0, radius=0.2+elapsed*0.4, pressure=-5-elapsed*0.5 },
	}
	elseif 155 < scene_time_second and scene_time_second < 204 then
		local elapsed = math.max(math.min(scene_time_second-169, 204), 0)
		return {
		{ posx=0, posy=1+elapsed*0.278, posz=0, radius=0.05, pressure=-30 }
	}
	end
	return nil
end

function do_reset(scene_time_second)
	if 105 < scene_time_second and scene_time_second < 106 then
		return true 
	end
--	elseif 156 < scene_time_second and scene_time_second < 157 then
--		return true
--	elseif 200 < scene_time_second then
--		return true
--	end
	return false
end
