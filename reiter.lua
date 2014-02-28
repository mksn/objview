local reiter_data = {
  skeleton = "data/actors/ge_hom_skel.iqe",
  animations = {
    IDLE = "data/actors/anims/fy_hom_idle_in0_monture.iqe",
    WALK = "data/actors/anims/fy_hom_marche_in0_monture.iqe",
    RUN = "data/actors/anims/fy_hom_course_in0_monture.iqe",
    TURN_LEFT = "data/actors/anims/fy_hom_tournegauche_monture.iqe",
    TURN_RIGHT = "data/actors/anims/fy_hom_tournedroite_monture.iqe",
    STRAFE_LEFT = "data/actors/anims/fy_hom_idle_in0_monture.iqe",
    STRAFE_RIGHT = "data/actors/anims/fy_hom_idle_in0_monture.iqe",
    DEATH = "data/actors/anims/fy_hom_mort_in0_monture.iqe",
  },
  skin_components = {
    "data/actors/armor/fy_hom_armor00_armpad.iqe",
    "data/actors/armor/fy_hom_armor00_bottes.iqe",
    "data/actors/armor/fy_hom_armor00_gilet.iqe",
    "data/actors/armor/fy_hom_armor00_hand.iqe",
    "data/actors/armor/fy_hom_armor00_pantabottes.iqe",
    "data/actors/cheveux/fy_hom_cheveux_basic01.iqe",
    "data/actors/visage/fy_hom_visage.iqe"
  },
}

function make_reiter(parent)
  local t = make_unit_with_data(reiter_data)
  if parent then
    t:attach_model(parent, "tag.saddle")
  end
  return t
end

function new_reiter() 
  local m = make_reiter()
  show_unit(m)
  return m
end
