/*
 * Copyright (C) 2009 Christopho, Solarus - http://www.solarus-engine.org
 * 
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "entities/PickableItemFairy.h"
#include "movements/FairyMovement.h"
#include "Game.h"
#include "Equipment.h"
#include "Sprite.h"

/**
 * Creates a pickable fairy.
 * The layer is automatically set to LAYER_HIGH.
 * @param x x coordinate of the fairy to create
 * @param y y coordinate of the fairy to create
 */
PickableItemFairy::PickableItemFairy(int x, int y):
  PickableItem(LAYER_HIGH, x, y, PickableItem::FAIRY, 0) {

}

/**
 * Destructor.
 */
PickableItemFairy::~PickableItemFairy(void) {

}

/**
 * Returns whether this entity can have collisions with entities even if
 * they are not on the same layer.
 * @return true if this entity can collide with entities that are on another layer
 */
bool PickableItemFairy::has_layer_independent_collisions(void) {
  return true;
}

/**
 * Initializes the fairy's movement.
 * This is a redefinition of PickableItem::initialize_movement()
 * to create the specific movement of a fairy.
 */
void PickableItemFairy::initialize_movement(void) {

  falling_height = FALLING_NONE; // a fairy does not fall
  set_movement(new FairyMovement());
}

/**
 * Updates the fairy.
 * This function is called repeatedly by the map.
 * This is a redefinition of PickableItem::update() to update
 * the position of the shadow depending on the fairy's position.
 */
void PickableItemFairy::update(void) {

  PickableItem::update();

  shadow_x = get_x();
  shadow_y = get_y();

  FairyMovement *movement = (FairyMovement*) get_movement();
  if (movement->get_x_speed() >= 0) {
    get_sprite()->set_current_direction(0);
  }
  else {
    get_sprite()->set_current_direction(1);
  }
}

/**
 * Gives the item to the player.
 */
void PickableItemFairy::give_item_to_player(void) {

  // play the sound
  if (features[subtype].sound != "") {
    game->play_sound(features[subtype].sound);
  }

  // give the fairy
  Equipment *equipment = game->get_equipment();
  equipment->found_fairy();
}
