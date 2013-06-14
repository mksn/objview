-- test

unit_list = {}

function make_human()
  local u = ov.unit_new();
  ov.unit_set_skeleton(u, ov.skeleton_load("human/ge_hom_skel.iqe"));
  ov.unit_add_animation(u, ov.animation_load("human/fy_hom_ab_marche.iqe"), 'WALK');
  ov.unit_add_animation(u, ov.animation_load("human/fy_hom_ab_idle.iqe"), 'IDLE');
  ov.unit_add_skin_component(u, ov.model_load("human/fy_hom_armor00_armpad.iqe"));
  ov.unit_add_skin_component(u, ov.model_load("human/fy_hom_armor00_bottes.iqe"));
  ov.unit_add_skin_component(u, ov.model_load("human/fy_hom_armor00_gilet.iqe"));
  ov.unit_add_skin_component(u, ov.model_load("human/fy_hom_armor00_hand.iqe"));
  ov.unit_add_skin_component(u, ov.model_load("human/fy_hom_armor00_pantabottes.iqe"));
  ov.unit_add_skin_component(u, ov.model_load("human/fy_hom_cheveux_basic01.iqe"));
  ov.unit_add_skin_component(u, ov.model_load("human/fy_hom_visage.iqe"));
  ov.unit_add_bone_component(u, ov.model_load("human/fy_wea_dague.iqe"), "box_arme");
  ov.unit_add_bone_component(u, ov.model_load("human/fy_wea_grand_bouclier.iqe"), "box_bouclier");
  local t = {unit=u, action='IDLE'}
  table.insert(unit_list, t)
  return t
end

h1 = make_human()
h2 = make_human()

ov.unit_set_position(h1.unit, 0, 0, 0)
ov.unit_set_position(h2.unit, 2, 0, 0)

current_unit = 1

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
  for i,u in ipairs(unit_list) do
    ov.unit_animate(u.unit, u.action, time)
  end
  ov.unit_set_rotation(h2.unit, time)
end

function draw()
  for i,u in ipairs(unit_list) do
    ov.unit_draw(u.unit)
  end
end
