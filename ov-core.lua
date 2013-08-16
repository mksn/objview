-- core functions for objview lua implementation

function select_next_unit()
  current_unit = current_unit + 1
  if current_unit > #unit_list then
    current_unit = 1
  end
end

function set_current_action(action)
  unit_list[current_unit].action = action
end

function update(time)
  if unit_list[current_unit] then
    local x, y, z = ov.unit_get_position(unit_list[current_unit].unit)
    highlight.position = {x, y, z + 2, 1}
  end

  for i,u in ipairs(unit_list) do
    ov.unit_animate(u.unit, u.action, time)
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
    ov.unit_draw(u.unit)
  end

  for i,light in ipairs(light_list) do
    gl.light_disable(i)
  end
end
