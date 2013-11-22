local mektoub_data = {
  skeleton = "mektoub/tr_mo_mektoub_selle.iqe",
  animations = {
    DEATH = "mektoub/tr_mo_mektoub_mort.iqe",
    IDLE = "mektoub/tr_mo_mektoub_selle_idle.iqe",
    WALK = "mektoub/tr_mo_mektoub_selle_marche.iqe",
    STRAFE_RIGHT = "mektoub/tr_mo_mektoub_selle_straf_droite.iqe",
    STRAFE_LEFT = "mektoub/tr_mo_mektoub_selle_straf_gauche.iqe",
    TURN_RIGHT = "mektoub/tr_mo_mektoub_selle_tournedroite.iqe",
    TURN_LEFT = "mektoub/tr_mo_mektoub_selle_tournegauche.iqe",
  },
  skin_components = {
    "mektoub/tr_mo_mektoub_selle.iqe"
  }
}

function make_mektoub()
  return make_unit_with_data(mektoub_data)
end
