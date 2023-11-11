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
const float GUN_BB_WIDTH = 1.f * 32;
const float GUN_BB_HEIGHT = 1.f * 32;
const float ROCKET_LAUNCHER_BB_WIDTH = .8f * 82;
const float ROCKET_LAUNCHER_BB_HEIGHT = .8f * 28;
const float ROCKET_BB_WIDTH = 1.f * 43;
const float ROCKET_BB_HEIGHT = 1.f * 7;
const float GRENADE_LAUNCHER_BB_WIDTH = 1.3f * 41;
const float GRENADE_LAUNCHER_BB_HEIGHT = 1.3f * 18;
const float GRENADE_BB_WIDTH = 1.f * 18;
const float GRENADE_BB_HEIGHT = 1.f * 19;
const float EXPLOSION_BB_WIDTH = 1.f * 92;
const float EXPLOSION_BB_HEIGHT = 1.f * 100;
const float HEART_BB_WIDTH = 2.f * 16;
const float HEART_BB_HEIGHT = 2.f * 16;
const float WINGED_BOOTS_BB_WIDTH = .02f * 1489;
const float WINGED_BOOTS_BB_HEIGHT = .02f * 1946;
const float DASH_BOOTS_BB_WIDTH = 1.2f * 27;
const float DASH_BOOTS_BB_HEIGHT = 1.2f * 30;
const float PICKAXE_BB_WIDTH = .5f * 55;
const float PICKAXE_BB_HEIGHT = .5f * 80;
const float SPITTER_BB_WIDTH = 16.f * 3.f;
const float SPITTER_BB_HEIGHT = 24.f * 3.f;
const float SPITTER_BULLET_BB_WIDTH = 16.f * 3.f;
const float SPITTER_BULLET_BB_HEIGHT = 16.f * 3.f;

// the player
Entity createHero(RenderSystem *renderer, vec2 pos);
// the enemy
Entity createEnemy(RenderSystem *renderer, vec2 position, float angle, vec2 velocity, vec2 scale);
// spitter enemy
Entity createSpitterEnemy(RenderSystem *renderer, vec2 pos);
// spitter enemy bullet
Entity createSpitterEnemyBullet(RenderSystem *renderer, vec2 pos, float angle);
// the sword
Entity createSword(RenderSystem *renderer, vec2 position);
// the gun
Entity createGun(RenderSystem* renderer, vec2 position);
// the bullet
Entity createBullet(RenderSystem* renderer, vec2 position, float angle);

Entity createRocketLauncher(RenderSystem* renderer, vec2 position);

Entity createRocket(RenderSystem* renderer, vec2 position, float angle);

Entity createGrenadeLauncher(RenderSystem* renderer, vec2 position);

Entity createGrenade(RenderSystem* renderer, vec2 position, float angle);

Entity createExplosion(RenderSystem* renderer, vec2 position, float size);

Entity createHeart(RenderSystem* renderer, vec2 position);

Entity createWingedBoots(RenderSystem* renderer, vec2 position);

Entity createDashBoots(RenderSystem* renderer, vec2 position);

Entity createPickaxe(RenderSystem* renderer, vec2 position);
// the background
Entity createBackground(RenderSystem* renderer);
// the helper text during pause
Entity createHelperText(RenderSystem* renderer);
Entity createBlock(RenderSystem* renderer, vec2 pos, vec2 size);
// the ui button
Entity createButton(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID type, std::function<void ()> callback, bool visibility = true);
Entity createWeaponHitBox(RenderSystem* renderer, vec2 pos, vec2 size);
Entity createTitleText(RenderSystem* renderer, vec2 pos);
