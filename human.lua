local human_data = {
  skeleton = "data/actors/ge_hom_skel.iqe",
  animations = {
    WALK = "data/actors/anims/fy_hom_ab_marche.iqe",
    IDLE = "data/actors/anims/fy_hom_ab_idle.iqe"
  },
  skin_components = {
    "data/actors/armor/fy_hom_armor00_armpad.iqe",
    "data/actors/armor/fy_hom_armor00_bottes.iqe",
    "data/actors/armor/fy_hom_armor00_gilet.iqe",
    "data/actors/armor/fy_hom_armor00_hand.iqe",
    "data/actors/armor/fy_hom_armor00_pantabottes.iqe",
    "data/actors/cheveux/fy_hom_cheveux_basic01.iqe",
    "data/actors/visage/fy_hom_visage.iqe",
  },
  bone_components = {
    {"data/actors/weapons/fy_wea_dague.iqe", "box_arme"},
    {"data/actors/weapons/fy_wea_grand_bouclier.iqe", "box_bouclier"},
  }
}

function make_human()
  return make_unit_with_data(human_data)
end
