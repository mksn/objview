require 'mektoub'
require 'dog'
require 'human'
require 'reiter'

m = make_mektoub()
r = make_reiter(m)
g = make_group {m, r}
show_unit(g)

add_prop("data/vegetation/trees/fo_s2_birch.iqe", 2, 0, 0, 0)
add_prop("data/vegetation/trees/fo_s2_birch.iqe", -2, 0, 0, 0)
add_prop("data/vegetation/trees/fo_s2_birch.iqe", 2, 2, 0, 0)
add_prop("data/vegetation/trees/fo_s2_birch.iqe", 4, -2, 0, 0)
add_prop("data/vegetation/trees/fo_s2_birch.iqe", 2, 0, 0, 0)