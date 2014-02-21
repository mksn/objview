local mektoub_data = {
  skeleton = "data/monsters/cheval/tr_mo_mektoub_selle.iqe",
  animations = {
    IDLE = "data/monsters/cheval/anims/tr_mo_mektoub_selle_idle.iqe",
    WALK = "data/monsters/cheval/anims/tr_mo_mektoub_selle_marche.iqe",
    STRAFE_RIGHT = "data/monsters/cheval/anims/tr_mo_mektoub_selle_straf_droite.iqe",
    STRAFE_LEFT = "data/monsters/cheval/anims/tr_mo_mektoub_selle_straf_gauche.iqe",
    TURN_RIGHT = "data/monsters/cheval/anims/tr_mo_mektoub_selle_tournedroite.iqe",
    TURN_LEFT = "data/monsters/cheval/anims/tr_mo_mektoub_selle_tournegauche.iqe",
    DEATH = "data/monsters/cheval/anims/tr_mo_mektoub_mort.iqe",
  },
  skin_components = {
    "data/monsters/cheval/tr_mo_mektoub_selle.iqe"
  }
}

function make_mektoub()
  return make_unit_with_data(mektoub_data)
end

function new_mektoub()
  m = make_mektoub()
  show_unit(m)
  return m
end
