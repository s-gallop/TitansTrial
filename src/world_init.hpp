#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include <map>

// These are ahrd coded to the dimensions of the entity texture
const float ENEMY_BB_WIDTH = 3.f * 26.f;
const float ENEMY_BB_HEIGHT = 3.f * 30.f;
const float SWORD_BB_WIDTH = 1.f * 21.f;
const float SWORD_BB_HEIGHT = 1.f * 50.f;
const float GUN_BB_WIDTH = 1.f * 32;
const float GUN_BB_HEIGHT = 1.f * 32;
const float HEART_BB_WIDTH = 2.f * 16;
const float HEART_BB_HEIGHT = 2.f * 16;
const float WINGED_BOOTS_BB_WIDTH = .02f * 1489;
const float WINGED_BOOTS_BB_HEIGHT = .02f * 1946;
const float DASH_BOOTS_BB_WIDTH = 1.2f * 27;
const float DASH_BOOTS_BB_HEIGHT = 1.2f * 30;
const float PICKAXE_BB_WIDTH = .5f * 55;
const float PICKAXE_BB_HEIGHT = .5f * 80;
const float SPITTER_BULLET_BB_WIDTH = 16.f * 3.f;
const float SPITTER_BULLET_BB_HEIGHT = 16.f * 3.f;


const float CHARACTER_SCALING = 3.0f;

const std::map<TEXTURE_ASSET_ID, vec2 > ASSET_SIZE = {
        { TEXTURE_ASSET_ID::QUIT,{204, 56} },
        { TEXTURE_ASSET_ID::QUIT_PRESSED,{204, 56} },
        { TEXTURE_ASSET_ID::MENU,{30, 32} },
        { TEXTURE_ASSET_ID::MENU_PRESSED,{30, 32} },
        { TEXTURE_ASSET_ID::HELPER,{580, 162} },
        { TEXTURE_ASSET_ID::PLAY, {204, 56}},
        { TEXTURE_ASSET_ID::PLAY_PRESSED, {204, 56}},
        { TEXTURE_ASSET_ID::TITLE_TEXT, {600, 120}},
        { TEXTURE_ASSET_ID::HERO, {15*CHARACTER_SCALING, 16*CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::SPITTER_ENEMY, {16*CHARACTER_SCALING, 24*CHARACTER_SCALING}}
};

const std::map<TEXTURE_ASSET_ID, vec2 > SPRITE_SCALE = {
        { TEXTURE_ASSET_ID::HERO, {52*CHARACTER_SCALING, 21*CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::SPITTER_ENEMY, {57*CHARACTER_SCALING, 39*CHARACTER_SCALING}}
};
const std::map<TEXTURE_ASSET_ID, vec2 > SPRITE_OFFSET = {
        { TEXTURE_ASSET_ID::HERO, {10*CHARACTER_SCALING, -1*CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::SPITTER_ENEMY, {-10*CHARACTER_SCALING, -6*CHARACTER_SCALING}},
};

const std::map<TEXTURE_ASSET_ID, AnimationInfo> ANIMATION_INFO = {
        {TEXTURE_ASSET_ID::HERO, {
                13,
                {9, 1, 8, 4, 4, 4, 16, 4, 8, 4, 14, 2, 8},
                0,
                16
        }},
        {TEXTURE_ASSET_ID::SPITTER_ENEMY, {
            5,
            {6, 7, 8, 3, 8},
            0,
            9
        }}
};
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
