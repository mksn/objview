-- test

current_animation = 'IDLE'

function make_human()
  local humon = ov.unit_new();
  ov.unit_set_skeleton(humon, ov.skeleton_load("human/ge_hom_skel.iqe"));
  ov.unit_add_animation(humon, ov.animation_load("human/fy_hom_ab_marche.iqe"), 'WALK');
  ov.unit_add_animation(humon, ov.animation_load("human/fy_hom_ab_idle.iqe"), 'IDLE');
  ov.unit_add_skin_component(humon, ov.model_load("human/fy_hom_armor00_armpad.iqe"));
  ov.unit_add_skin_component(humon, ov.model_load("human/fy_hom_armor00_bottes.iqe"));
  ov.unit_add_skin_component(humon, ov.model_load("human/fy_hom_armor00_gilet.iqe"));
  ov.unit_add_skin_component(humon, ov.model_load("human/fy_hom_armor00_hand.iqe"));
  ov.unit_add_skin_component(humon, ov.model_load("human/fy_hom_armor00_pantabottes.iqe"));
  ov.unit_add_skin_component(humon, ov.model_load("human/fy_hom_cheveux_basic01.iqe"));
  ov.unit_add_skin_component(humon, ov.model_load("human/fy_hom_visage.iqe"));
  ov.unit_add_bone_component(humon, ov.model_load("human/fy_wea_dague.iqe"), "box_arme");
  ov.unit_add_bone_component(humon, ov.model_load("human/fy_wea_grand_bouclier.iqe"), "box_bouclier");
  return humon
end

h1 = make_human()
h2 = make_human()

function update(time)
  ov.unit_animate(h1, current_animation, time)
  ov.unit_animate(h2, 'IDLE', time)
end

function draw()
  ov.unit_draw(h1)
  ov.unit_draw(h2)
end
