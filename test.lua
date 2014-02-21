require 'mektoub'
require 'dog'
require 'human'
require 'reiter'

m = make_mektoub()
r = make_reiter(m)
g = make_group {m, r}
show_unit(g)

add_prop("data/vegetation/trees/fo_s2_birch.iqe", 2, 0, 0, 0)
add_prop("data/vegetation/trees/fo_s2_birch.iqe", -2, 0, 0, 140)
add_prop("data/vegetation/trees/fo_s2_birch.iqe", 2, 2, 0, 20)
add_prop("data/vegetation/trees/fo_s3_birch.iqe", 4, -2, 0, 30)
add_prop("data/vegetation/trees/fo_s2_birch.iqe", -3, 1, 0, 50)

local grasses = {
  "data/vegetation/micro/ju_buissonplateau.iqe",
  "data/vegetation/micro/ju_champia.iqe",
  "data/vegetation/micro/ju_champib.iqe",
  "data/vegetation/micro/ju_champipetita.iqe",
  "data/vegetation/micro/ju_debrisa.iqe",
  "data/vegetation/micro/ju_debrisb.iqe",
  "data/vegetation/micro/ju_debrisc.iqe",
  "data/vegetation/micro/ju_debrisd.iqe",
  "data/vegetation/micro/ju_debrisfeuillesa.iqe",
  "data/vegetation/micro/ju_fleura.iqe",
  "data/vegetation/micro/ju_fleurb.iqe",
  "data/vegetation/micro/ju_fougerea.iqe",
  "data/vegetation/micro/ju_fougereabis.iqe",
  "data/vegetation/micro/ju_fougereb.iqe",
  "data/vegetation/micro/ju_herbefleura.iqe",
  "data/vegetation/micro/ju_herbefleurb.iqe",
  "data/vegetation/micro/ju_herbefleurc.iqe",
  "data/vegetation/micro/ju_herbefleurd.iqe",
  "data/vegetation/micro/ju_herbefleure.iqe",
  "data/vegetation/micro/ju_herbehautea.iqe",
  "data/vegetation/micro/ju_herbeplantea.iqe",
  "data/vegetation/micro/ju_herbeseche.iqe",
  "data/vegetation/micro/ju_herbesechehautea.iqe",
  "data/vegetation/micro/ju_herbevertea.iqe",
  "data/vegetation/micro/ju_herbeverteb.iqe",
  "data/vegetation/micro/ju_lianesola.iqe",
  "data/vegetation/micro/ju_moussetombantea.iqe",
  "data/vegetation/micro/ju_palma.iqe",
  "data/vegetation/micro/ju_tuberculea.iqe",
  "data/vegetation/micro/ju_vegeta.iqe",
  "data/vegetation/micro/ju_vegetb.iqe",
  "data/vegetation/micro/ju_vegetc.iqe",
  "data/vegetation/micro/ju_vegetd.iqe",
  "data/vegetation/micro/ju_vegetgooa.iqe",
  "data/vegetation/micro/ju_vegetmaraisa.iqe",
}

for x = -10,10 do
  for y = -10,10 do
    local r = math.random() * 360;
    local p = math.floor(math.random() * #grasses) + 1
    add_prop(grasses[p], x + math.random() - 0.5, y + math.random() - 0.5, 0, r)
  end
end
