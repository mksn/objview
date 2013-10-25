local dog_data = {
  skeleton = "dog/tr_mo_chorani.iqe",
  animations = {
    IDLE = "dog/tr_mo_chien_idle.iqe",
    WALK = "dog/tr_mo_chien_marche.iqe",
  },
  skin_components = {
    "dog/tr_mo_chorani.iqe"
  }
}

function make_dog()
  return make_unit_with_data(dog_data)
end
