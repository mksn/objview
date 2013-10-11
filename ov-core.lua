-- core functions for objview lua implementation

unit_mt = {}

unit_mt.__index = unit_mt

function unit_mt:set_skeleton(skel)
  ov.unit_set_skeleton(self.unit, ov.skeleton_load(skel))
end

function unit_mt:add_animation(anim, action)
  ov.unit_add_animation(self.unit, ov.animation_load(anim), action)
end

function unit_mt:add_skin_component(model)
  ov.unit_add_skin_component(self.unit, ov.model_load(model))
end

function unit_mt:add_bone_component(model, bone)
  ov.unit_add_bone_component(self.unit, ov.model_load(model), bone)
end

function unit_mt:set_position(x, y, z)
  ov.unit_set_position(self.unit, x, y, z)
end

function unit_mt:set_rotation(a)
  ov.unit_set_rotation(self.unit, a)
end

function unit_mt:get_position(x, y, z)
  return ov.unit_get_position(self.unit)
end

function unit_mt:step_action(delta)
  self.action_time = self.action_time + delta
  local duration = ov.unit_get_animation_duration(self.unit, self.action)
  while self.action_time >= duration do
    self.action_time = self.action_time - duration
  end
end

function unit_mt:action_duration()
  return ov.unit_get_animation_duration(self.unit, self.action)
end

function unit_mt:animate()
  ov.unit_animate(self.unit, self.action, self.action_time)
end

function unit_mt:draw(x, y, z)
  ov.unit_draw(self.unit)
end

function make_unit()
  local t = {
    unit = ov.unit_new(),
    action = 'IDLE',
    action_time = 0;
  }
  table.insert(unit_list, t)
  return setmetatable(t, unit_mt)
end

function make_unit_with_data(data)
  local u = make_unit()
  u:set_skeleton(data.skeleton)
  for action, anim in pairs(data.animations) do
    u:add_animation(anim, action)
  end
  for i,model in ipairs(data.skin_components) do
    u:add_skin_component(model)
  end
  return u
end

function select_next_unit()
  current_unit = current_unit + 1
  if current_unit > #unit_list then
    current_unit = 1
  end
end

function set_current_action(action)
  unit_list[current_unit].action = action
end

function update(delta)
  if unit_list[current_unit] then
    local x, y, z = ov.unit_get_position(unit_list[current_unit].unit)
    highlight.position = {x, y, z + 2, 1}
  end

  for i,u in ipairs(unit_list) do
    u:step_action(delta)
    u:animate(u.action_time)
  end
end

function draw()
  for i,light in ipairs(light_list) do
    gl.light_enable(i)
    gl.light_set_position(i, table.unpack(light.position))
    gl.light_set_color(i, table.unpack(light.color))
    gl.light_set_attenuation(i, table.unpack(light.attenuation))
  end

  for i,u in ipairs(unit_list) do
    u:draw()
  end

  for i,light in ipairs(light_list) do
    gl.light_disable(i)
  end
end
