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

-- table 
local g_time = 0.0
local g_previous_position = { 0.01, 0.01, 0.01 }

local function greets_emitter(scene_time_second, emitcount)
	local elapsed = scene_time_second - 75
	return {
	 posx = 1 + elapsed * 0.3, --current[1]*0.5,
	 posy = math.sin(scene_time_second*2)*0.2, --current[2]*0.5,
	 posz = 0,
	 count = emitcount, -- per frame emit
	 radius = 0.3,
	}
end

local g_color_emitters = {}

local function color_emitter(scene_time_second)
	local elapsed = scene_time_second - 104
	return g_color_emitters
end

function get_initial_position()
	return g_previous_position;
end

local g_emit_count = 1024 * 2-- * 512

function get_emitter(scene_time_second, scene_time_ratio) 
	local emit_count = g_emit_count
	--g_emit_count = 0
 	local time = g_time
	g_time = g_time + 0.1
	local current = g_previous_position;
	g_previous_position = lorenz_attractor(
		g_previous_position[1],
		g_previous_position[2],
		g_previous_position[3], 0.005)

	if scene_time_second < 5 then
		return nil
	elseif scene_time_second < 75 then
		return {
     	 	posx = 0, --time * 0.3, --current[1]*0.5,
     	 	posy = 0, -- + math.sin(time*0.5)*0.2, --current[2]*0.5,
     	 	posz = 0, --current[3]*0.5,
   	 		count = emit_count, -- per frame emit
  	 		radius = 0.1,
  	 	}
	elseif scene_time_second < 104 then
		return greets_emitter(scene_time_second, emit_count*8)
	elseif scene_time_second < 150 then
		return color_emitter(scene_time_second)
	elseif 150 < scene_time_second then
		return nil
	end

 	return {
	 posx = 0, --time * 0.3, --current[1]*0.5,
	 posy = 0, -- + math.sin(time*0.5)*0.2, --current[2]*0.5,
	 posz = 0, --current[3]*0.5,
	 count = emit_count, -- per frame emit
	 radius = 0.1,
	}

end

local function begin_trigger_emitter(scene_time_second)
	return {
  	 posx = 0, --time * 0.3, --current[1]*0.5,
	 posy = 0, -- + math.sin(time*0.5)*0.2, --current[2]*0.5,
	 posz = 0, --current[3]*0.5,
	 count = 1024 * 256, -- per frame emit
	 radius = 0.1,
	 lifetime = 240,
	}
end

local g_fft_count = 1
local g_fft_position_table = (function()
	local theta = 0
	local t = {}
	for i=1, 8 do
		local theta = (2*3.14159) * (i/8)
		table.insert(t, { math.cos(theta), math.sin(theta), 0 })
	end
	return t
end)()

local function fft_trigger_emitter(scene_time_second)
	local p = g_fft_position_table[g_fft_count]
	g_fft_count = g_fft_count + 1
	return {
  	 posx = p[0],--math.random()-0.5, --time * 0.3, --current[1]*0.5,
	 posy = p[1],--math.random()-0.5, -- + math.sin(time*0.5)*0.2, --current[2]*0.5,
	 posz = p[2],--math.random()-0.5, --current[3]*0.5,
	 count = 65536,--4096*4, --65536*0.5,
	 radius = 0.015,
	 lifetime = 396,
	}
end

local function birth_trigger_emitter(scene_time_second) 
	 return {
  	 posx = math.random()*2-1, --time * 0.3, --current[1]*0.5,
	 posy = math.random()*2-1, -- + math.sin(time*0.5)*0.2, --current[2]*0.5,
	 posz = math.random()*2-1, --current[3]*0.5,
	 count = 65536, -- per frame emit
	 radius = 0.015,
	 lifetime = 620,
	}
end

local g_color_count = 0
local function color_trigger_emitter(scene_time_second)
	local d = 2 / 8
	local x = -1 + d * g_color_count
	local z = -1 + d * g_color_count
	table.insert(g_color_emitters, {
		posx = x, posy = 0, posz = z,
		count = 2048, radius = 0.08, lifetime = 312,
		index=g_color_count+1,
		})
	g_color_count = g_color_count + 1
end

function add_triggers()
	return { 0, 5.2, 7, 8.8, 10.6, 12.4, 14.2, 33, 34, 35.5, 36.8, 40.1, 41.2, 42.5, 43, 44.4, 46.1, 47, 49, 51.3, 54,
	54.9, 58.1, 59.3, 60, 61.3, 64, 64.2, --[[67.3, 69,]] 70, 102.4,
	104.2, 106, 107.8, 109.6,
	111.4, 113.2, 115, 116.8, }
end

function get_trigger_emitter(scene_time_second)
	if scene_time_second < 1 then
		return begin_trigger_emitter(scene_time_second)
	elseif scene_time_second < 30 then
		return fft_trigger_emitter(scene_time_second)
	elseif scene_time_second < 75 then
		return birth_trigger_emitter(scene_time_second)
	elseif scene_time_second < 118 then
		return color_trigger_emitter(scene_time_second)
	end
end

function get_shader(scene_time_second)
	if scene_time_second < 5 then
		return "particle_simulate_a"
	elseif scene_time_second < 17 then
		return "particle_simulate"
	elseif scene_time_second < 104 then
		return "particle_simulate"
	elseif scene_time_second < 118 then
		return "particle_no_simulate"
	elseif scene_time_second < 155 then
		return "particle_simulate_color"
	elseif 155 < scene_time_second then
		return "particle_simulate_moon"
	end
	return "particle_simulate"
end

function get_render_shader(scene_time_second)
	if scene_time_second < 155 then
		return "particle_render"
	end
	return "particle_moon_render"
end
