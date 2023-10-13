#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float ENEMY_BB_WIDTH = 3.f * 26.f;
const float ENEMY_BB_HEIGHT = 3.f * 30.f;
const float SWORD_BB_WIDTH = 1.f * 21.f;
const float SWORD_BB_HEIGHT = 1.f * 50.f;
const float HERO_BB_WIDTH = 15.f * 3.f;
const float HERO_BB_HEIGHT = 16.f * 3.f;

// the player
Entity createHero(RenderSystem *renderer, vec2 pos);
// the enemy
Entity createEnemy(RenderSystem *renderer, vec2 position, float angle, vec2 velocity, vec2 scale);
// the sword
Entity createSword(RenderSystem *renderer, vec2 position);
// the sword weapon
Entity createWeaponSword(RenderSystem *renderer);
// the background
Entity createBackground();

Entity createBlock(vec2 pos, vec2 size);

Entity createWeaponHitBox(vec2 pos, vec2 size);
