function make_dog()
  local u = make_unit()
  u:set_skeleton("dog/tr_mo_chorani.iqe")
  u:add_animation("dog/tr_mo_chien_idle.iqe", "IDLE")
  u:add_animation("dog/tr_mo_chien_marche.iqe", "WALK")
  u:add_skin_component("dog/tr_mo_chorani.iqe")
  return u
end
