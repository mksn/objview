function make_human()
  local u = make_unit()
  u:set_skeleton("human/ge_hom_skel.iqe")
  u:add_animation("human/fy_hom_ab_marche.iqe", 'WALK')
  u:add_animation("human/fy_hom_ab_idle.iqe", 'IDLE')
  u:add_skin_component("human/fy_hom_armor00_armpad.iqe")
  u:add_skin_component("human/fy_hom_armor00_bottes.iqe")
  u:add_skin_component("human/fy_hom_armor00_gilet.iqe")
  u:add_skin_component("human/fy_hom_armor00_hand.iqe")
  u:add_skin_component("human/fy_hom_armor00_pantabottes.iqe")
  u:add_skin_component("human/fy_hom_cheveux_basic01.iqe")
  u:add_skin_component("human/fy_hom_visage.iqe")
  u:add_bone_component("human/fy_wea_dague.iqe", "box_arme")
  u:add_bone_component("human/fy_wea_grand_bouclier.iqe", "box_bouclier")
  return u
end

local humon_data = {
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

function make_human2()
  return make_unit_with_data(humon_data)
end
