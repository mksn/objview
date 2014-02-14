local dog_data = {
  skeleton = "data/monsters/chien/tr_mo_chorani.iqe",
  animations = {
    IDLE = "data/monsters/chien/anims/tr_mo_chien_idle.iqe",
    WALK = "data/monsters/chien/anims/tr_mo_chien_marche.iqe",
  },
  skin_components = {
    "data/monsters/chien/tr_mo_chorani.iqe"
  }
}

function make_dog()
  return make_unit_with_data(dog_data)
end
