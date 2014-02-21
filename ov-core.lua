-- core functions for objview lua implementation

require "table"

prop_mt = {}
prop_mt.__index = prop_mt

function prop_mt:set_position(x, y, z)
  ov.prop_set_position(self.prop, x, y, z)
end

function prop_mt:set_rotation(a)
  ov.prop_set_rotation(self.prop, a)
end

function prop_mt:get_position(x, y, z)
  return ov.prop_get_position(self.prop)
end

function prop_mt:get_rotation()
  ov.prop_get_rotation(self.prop)
end

function prop_mt:draw()
  ov.prop_draw(self.prop)
end

function make_prop(filename)
  local t = {
    prop = ov.prop_new(filename),
  }
  return setmetatable(t, prop_mt)
end

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

function unit_mt:attach_model(model, bone)
  ov.unit_attach_model(self.unit, model.unit, bone)
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

function unit_mt:get_rotation()
  ov.unit_get_rotation(self.unit)
end

function unit_mt:reset_action()
  self.action_time = 0;
end

function unit_mt:set_action(action)
  self.action = action
  self.action_time = 0;
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

function unit_mt:dump(action, prefix)
  action(string.format("local %s_data = {", prefix))
  action(string.format("  skeleton = %q,", ov.get_skeleton_name(self.unit)))
  action("  skin_components = {")
  local s = ov.get_skin_component_table(self.unit)
  for i,model in pairs(s) do
    action(string.format("    %q,", model))
  end
  action("  },")
  local a = ov.get_animations_table(self.unit)
  action("  animations = {")
  for a, anim in pairs(a) do
    action(string.format("    %s = %q,", a, anim))
  end
  action("  }")
  action("}")
end

function unit_mt:save(mdlname, filename)
  file = io.open(filename, "w")
  local function fun(x)
    return file:write(x,'\n')
  end
  file:write(string.format(" -- %s model setup --\n\n", mdlname))
  self:dump(fun, mdlname)
  file:write("\n")
  file:write(string.format("function make_%s()\n", mdlname))
  file:write(string.format("  make_unit_with_data(%s_data)\n", mdlname))
  file:write("end\n")
  io.close(file)
end

function unit_mt:__tostring()
  local buf = {}
  local cat = function(s)
    table.insert(buf, s)
  end
  self:dump(cat)
  return table.concat(buf, "\n")
end

function unit_mt:echo()
  print(" -- Current Model --")
  print(" ")
  print(self)
end

function unit_mt:draw()
  ov.unit_draw(self.unit)
end

function make_unit()
  local t = {
    unit = ov.unit_new(),
    action = 'IDLE',
    action_time = 0
  }
  return setmetatable(t, unit_mt)
end

function make_unit_with_data(data)
  local u = make_unit()
  u:set_skeleton(data.skeleton)
  for action, anim in pairs(data.animations) do
    u:add_animation(anim, action)
  end
  if data.skin_components then
    for i,model in ipairs(data.skin_components) do
      u:add_skin_component(model)
    end
  end
  if data.bone_components then
    for i,bonepair in ipairs(data.bone_components) do
      u:add_bone_component(bonepair[1], bonepair[2])
    end
  end
  return u
end

-- Group unit to sync animations of children:

group_mt = {}
group_mt.__index = group_mt

function group_mt:set_position(x, y, z)
  self.position = { x, y, z }
  for i,u in ipairs(self.children) do
    u:set_position(x, y, z)
  end
end

function group_mt:get_position()
  return table.unpack(self.position)
end

function group_mt:set_rotation(a)
  for i,u in ipairs(self.children) do
    u:set_rotation(a)
  end
end

function group_mt:reset_action()
  for i,u in ipairs(self.children) do
    u:reset_action()
  end
end

function group_mt:set_action(action)
  for i,u in ipairs(self.children) do
    u:set_action(action)
  end
end

function group_mt:step_action(delta)
  for i,u in ipairs(self.children) do
    u:step_action(delta)
  end
end

function group_mt:animate()
  for i,u in ipairs(self.children) do
    u:animate()
  end
end

function group_mt:draw()
  for i,u in ipairs(self.children) do
    u:draw()
  end
end

function make_group(children)
  local t = {
    children = children,
    position = { 0, 0, 0 }
  }
  return setmetatable(t, group_mt)
end

-- Misc

function show_prop(t)
  table.insert(prop_list, t)
end

function add_prop(filename, x, y, z, r)
  local p = make_prop(filename);
  p:set_position(x, y, z)
  p:set_rotation(r)
  show_prop(p)
  return p
end

function show_unit(t)
  table.insert(unit_list, t)
end

function select_next_unit()
  current_unit = current_unit + 1
  if current_unit > #unit_list then
    current_unit = 1
  end
end

function set_current_action(action)
  if unit_list[current_unit] then
    unit_list[current_unit]:set_action(action)
  end
end

function reset_current_action()
  if unit_list[current_unit] then
    unit_list[current_unit]:reset_action()
  end
end

function update(delta)
  if unit_list[current_unit] then
    local x, y, z = unit_list[current_unit]:get_position()
    highlight.position = {x, y, z + 2, 1}
  end

  for i,u in ipairs(unit_list) do
    u:step_action(delta)
    u:animate()
  end
end

function draw()
  for i,light in ipairs(light_list) do
    gl.light_enable(i)
    gl.light_set_position(i, table.unpack(light.position))
    gl.light_set_color(i, table.unpack(light.color))
    gl.light_set_attenuation(i, table.unpack(light.attenuation))
  end

  for i,p in ipairs(prop_list) do
    p:draw()
  end

  for i,u in ipairs(unit_list) do
    u:draw()
  end

  for i,light in ipairs(light_list) do
    gl.light_disable(i)
  end
end
