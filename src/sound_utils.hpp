#pragma once

#include <iostream>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "common.hpp"

uint init_sound();
void destroy_sound();

enum class SOUND_EFFECT {
    HERO_DEAD = 0,
    HERO_JUMP = HERO_DEAD + 1,
    SWORD_SWING = HERO_JUMP + 1,
    BULLET_SHOOT = SWORD_SWING + 1,
    GUN_LEVER = BULLET_SHOOT + 1,
    HEAL = GUN_LEVER + 1,
    PICKAXE = HEAL + 1,
    DASH = PICKAXE + 1,
    EQUIPMENT_DROP = DASH + 1,
    BUTTON_CLICK = EQUIPMENT_DROP + 1
};

void play_music();
void play_sound(SOUND_EFFECT id);