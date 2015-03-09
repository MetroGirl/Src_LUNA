ID = "Script"

local g_init_pos = { 2.0, 2.0, -2.0 }
local g_init_at = { 0.0, 1.0, 0.0 }
local g_time = 0.0

local function greets_camera(scene_time_second)
	local elapsed = scene_time_second - 75
	return {
	 posx = g_init_pos[1] - 0.5 + 0.3 * elapsed,
	 posy = g_init_pos[2] + 1.5 + math.sin(scene_time_second),
	 posz = g_init_pos[3] + math.cos(scene_time_second), 
	 atx = g_init_at[1] - 0.5 + 0.3 * elapsed,
	 aty = g_init_at[2],
	 atz = g_init_at[3],
	}
end

local function ending_camera(scene_time_second)
	local y = math.min(scene_time_second, 204)
	if 169 < scene_time_second then
		return {
		posx = g_init_pos[1],
		posy = g_init_pos[2] + 0.3 * (y-169),
		posz = g_init_pos[3],
		atx = g_init_at[1],
		aty = g_init_at[2] + 0.36 * (y-169),
		atz = g_init_at[3],
	}
	end 
	return {
		posx = g_init_pos[1],
		posy = g_init_pos[2],
		posz = g_init_pos[3],
		atx = g_init_at[1],
		aty = g_init_at[2],
		atz = g_init_at[3],
	}
end

function get_position(scene_time_second, scene_time_ratio) 
	local time = g_time
	g_time = g_time + 0.1
	if scene_time_second < 75 then
	return {
	 posx = g_init_pos[1],-- + 0.3 * time,
	 posy = g_init_pos[2],
	 posz = g_init_pos[3], 
	 atx = g_init_at[1],-- + 0.3 * time,
	 aty = g_init_at[2],
 	 atz = g_init_at[3],
 	}
 	elseif scene_time_second < 104 then
 		return greets_camera(scene_time_second) 
 	elseif 155 < scene_time_second then
 		return ending_camera(scene_time_second)
 	end
 	return {
	 posx = g_init_pos[1],-- + 0.3 * time,
	 posy = g_init_pos[2],
	 posz = g_init_pos[3], 
	 atx = g_init_at[1],-- + 0.3 * time,
	 aty = g_init_at[2],
 	 atz = g_init_at[3],
 	}
end
