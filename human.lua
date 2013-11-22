local human_data = {
  skeleton = "human/ge_hom_skel.iqe",
  animations = {
    WALK = "human/fy_hom_ab_marche.iqe",
    IDLE = "human/fy_hom_ab_idle.iqe"
  },
  skin_components = {
    "human/fy_hom_armor00_armpad.iqe",
    "human/fy_hom_armor00_bottes.iqe",
    "human/fy_hom_armor00_gilet.iqe",
    "human/fy_hom_armor00_hand.iqe",
    "human/fy_hom_armor00_pantabottes.iqe",
    "human/fy_hom_cheveux_basic01.iqe",
    "human/fy_hom_visage.iqe",
    "human/fy_wea_dague.iqe", "box_arme",
    "human/fy_wea_grand_bouclier.iqe", "box_bouclier"
  }
}

function make_human()
  return make_unit_with_data(human_data)
end
