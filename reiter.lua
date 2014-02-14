local reiter_data = {
  skeleton = "human/ge_hom_skel.iqe",
  animations = {
    IDLE = "human/fy_hom_idle_in0_monture.iqe",
    WALK = "human/fy_hom_marche_in0_monture.iqe",
    TURN_LEFT = "human/fy_hom_tournegauche_monture.iqe",
    TURN_RIGHT = "human/fy_hom_tournedroite_monture.iqe",
    STRAFE_LEFT = "human/fy_hom_idle_in0_monture.iqe",
    STRAFE_RIGHT = "human/fy_hom_idle_in0_monture.iqe",
    DEATH = "human/fy_hom_mort_in0_monture.iqe",
  },
  skin_components = {
    "human/fy_hom_armor00_armpad.iqe",
    "human/fy_hom_armor00_bottes.iqe",
    "human/fy_hom_armor00_gilet.iqe",
    "human/fy_hom_armor00_hand.iqe",
    "human/fy_hom_armor00_pantabottes.iqe",
    "human/fy_hom_cheveux_basic01.iqe",
    "human/fy_hom_visage.iqe"
  },
}

function make_reiter(parent)
  local t = make_unit_with_data(reiter_data)
  t:attach_model(parent, "tag.saddle")
  return t
end
