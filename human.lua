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
