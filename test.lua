require 'mektoub'
require 'dog'
require 'human'
require 'reiter'

m = make_mektoub()
r = make_reiter(m)
g = make_group {m, r}
show_unit(g)
