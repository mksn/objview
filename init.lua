require 'ov-core'

sun = {
  position = {-1, -2, 2, 0},
  color = {0.3, 0.3, 0.3},
  attenuation = {1, 0, 0}
}

highlight = {
  position = {0, 0, 0, 1},
  color = {0.5, 1, 0.5},
  attenuation = {1, 0, 1}
}

light_list = { sun, highlight }

unit_list = {}
prop_list = {}

current_unit = 1

