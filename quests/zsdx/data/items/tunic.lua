local item = ...

function item:on_created()

  self:set_savegame_variable("i1128")
end

function item:on_variant_changed(variant)
  -- The possession state of the tunic determines the built-in ability "tunic".
  self:get_game():set_ability("tunic", variant)
end

