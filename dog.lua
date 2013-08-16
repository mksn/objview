function make_dog()
  local u = ov.unit_new();
  ov.unit_set_skeleton(u, ov.skeleton_load("dog/tr_mo_chorani.iqe"));
  ov.unit_add_animation(u, ov.animation_load("dog/tr_mo_chien_idle.iqe"), "IDLE");
  ov.unit_add_animation(u, ov.animation_load("dog/tr_mo_chien_marche.iqe"), "WALK");
  ov.unit_add_skin_component(u, ov.model_load("dog/tr_mo_chorani.iqe"));
  local t = {unit=u, action='IDLE'}
  table.insert(unit_list, t)
  return t
end

d1 = make_dog()

ov.unit_set_position(d1.unit, 1, 0, 0)
