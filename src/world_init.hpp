#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float ENEMY_BB_WIDTH = 0.4f * 620.f;
const float ENEMY_BB_HEIGHT = 0.4f * 430.f;

// the player
Entity createSalmon(RenderSystem* renderer, vec2 pos);
// the prey
Entity createFish(RenderSystem* renderer, vec2 position);
// the enemy
Entity createEnemy(RenderSystem* renderer, vec2 position);
// the background
Entity createBackground();
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// a pebble
Entity createPebble(vec2 pos, vec2 size);


