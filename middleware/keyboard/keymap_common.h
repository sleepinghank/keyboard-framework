#pragma once

#include <stdint.h>
#include "keyboard.h"


// translates key to keycode
uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key);
