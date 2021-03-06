/*
 * Copyright (C) 2006-2012 Christopho, Solarus - http://www.solarus-games.org
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
#include "hud/HUD.h"
#include "hud/ActionIcon.h"
#include "hud/SwordIcon.h"
#include "hud/PauseIcon.h"
#include "hud/ItemIcon.h"
#include "Game.h"
#include "GameControls.h"

/**
 * @brief Constructor.
 * @param game the current game (cannot be NULL)
 */
HUD::HUD(Game &game):
  game(game),
  nb_elements(0),
  showing_dialog(false) {

  elements[nb_elements++] = NULL;
  elements[nb_elements++] = NULL;
  elements[nb_elements++] = NULL;
  elements[nb_elements++] = new ItemIcon(game, 0, 8, 29);
  elements[nb_elements++] = new ItemIcon(game, 1, 60, 29);
  elements[nb_elements++] = new SwordIcon(game, 10, 29);
  elements[nb_elements++] = new PauseIcon(game, -3, 7);
  elements[nb_elements++] = new ActionIcon(game, 23, 51);
  elements[nb_elements++] = NULL;
  elements[nb_elements++] = NULL;
}

/**
 * @brief Destructor.
 */
HUD::~HUD() {

  for (int i = 0; i < nb_elements; i++) {
    delete elements[i];
  }
}

/**
 * @brief Makes an icon blink if the corresponding game key is being customized.
 */
void HUD::update_blinking() {

  // detect whether a key is being customized
  int index = -1;
  GameControls &controls = game.get_controls();
  if (controls.is_customizing()) {
    GameControls::GameKey key = controls.get_key_to_customize();
    switch (key) {

      case GameControls::ACTION:
        index = 7;
        break;

      case GameControls::SWORD:
        index = 5;
        break;

      case GameControls::PAUSE:
        index = 6;
        break;

      case GameControls::ITEM_1:
        index = 3;
        break;

      case GameControls::ITEM_2:
        index = 4;
        break;

      default:
        break;
    }
  }

  if (index != -1 && elements[index] != NULL) {
    if (!elements[index]->is_blinking()) {
      elements[index]->set_blinking(true);    
    }
  }
  for (int i = 3; i <= 7; i++) {
    if (i != index && elements[i] != NULL && elements[i]->is_blinking()) {
      elements[i]->set_blinking(false);
    }
  }
}

/**
 * @brief Updates the information displayed by HUD.
 */
void HUD::update() {

  // detect when the game is showing a dialog
  if (game.is_showing_dialog() && !showing_dialog) {
    showing_dialog = true;

    // a dialog is shown: hide or move the top-left icons
    elements[3]->set_visible(false); // item 0
    elements[4]->set_visible(false); // item 1
    elements[6]->set_visible(false); // pause icon
    elements[5]->set_position(-11, 17); // sword icon
    elements[7]->set_position(-11, 43); // action icon
  }
  else if (!game.is_showing_dialog()) {

    if (showing_dialog) {
      showing_dialog = false;

      // a message is finished: restore the top-left icons
      elements[3]->set_visible(true); // item 0
      elements[4]->set_visible(true); // item 1
      elements[6]->set_visible(true); // pause icon
      elements[5]->set_position(10, 29); // sword icon
      elements[7]->set_position(23, 51); // action icon
    }
    else {

      // if the hero is below the top-left icons, make them semi-transparent
      const Rectangle &hero_xy = game.get_hero_xy();
      if (elements[6]->get_opacity() == 255 && !game.is_suspended()
          && hero_xy.get_x() < 88 && hero_xy.get_y() < 80) {
        elements[3]->set_opacity(96); // item 0
        elements[4]->set_opacity(96); // item 1
        elements[6]->set_opacity(96); // pause icon
        elements[5]->set_opacity(96); // sword icon
        elements[7]->set_opacity(96); // action icon
      }
      else if (elements[6]->get_opacity() == 96
          && (hero_xy.get_x() >= 88 || hero_xy.get_y() >= 80 || game.is_suspended())) {
        elements[3]->set_opacity(255); // item 0
        elements[4]->set_opacity(255); // item 1
        elements[6]->set_opacity(255); // pause icon
        elements[5]->set_opacity(255); // sword icon
        elements[7]->set_opacity(255); // action icon
      }
    }
  }

  update_blinking();

  // update each element
  for (int i = 0; i < nb_elements; i++) {
    if (elements[i] != NULL) {
      elements[i]->update();
    }
  }
}

/**
 * @brief Draws the HUD.
 * @param dst_surface the destination surface
 */
void HUD::draw(Surface& dst_surface) {

  for (int i = 0; i < nb_elements; i++) {
    if (elements[i] != NULL) {
      elements[i]->draw(dst_surface);
    }
  }
}

