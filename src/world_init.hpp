#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include <map>
#include <vector>

// These are hard coded to the dimensions of the entity texture

const float CHARACTER_SCALING = 3.0f;
const float EXPLOSION_SCALING = 2.0f;

const vec2 ENEMY_BB = vec2(26.f, 30.f) * CHARACTER_SCALING;
const vec2 SWORD_BB = vec2(21.f, 50.f);
const vec2 GUN_BB = vec2(32.f, 32.f);
const vec2 ROCKET_LAUNCHER_BB = vec2(82.f, 28.f) * .8f;
const vec2 ROCKET_BB = vec2(43.f, 7.f);
const vec2 GRENADE_LAUNCHER_BB = vec2(41.f, 18.f) * 1.3f;
const vec2 GRENADE_BB = vec2(18.f, 19.f);
const vec2 HEART_BB = vec2(16.f, 16.f) * 2.f;
const vec2 WINGED_BOOTS_BB = vec2(1489.f, 1946.f) * .02f;
const vec2 DASH_BOOTS_BB = vec2(27.f, 30.f) * 1.2f;
const vec2 PICKAXE_BB = vec2(55.f, 80.f) * .5f;
const vec2 SPITTER_BULLET_BB = vec2(16.f, 16.f) * 3.f;
const vec2 HELPER_BB = vec2(1058, 532) / 3.f;

const std::map<TEXTURE_ASSET_ID, vec2 > ASSET_SIZE = {
        { TEXTURE_ASSET_ID::SPITTER_ENEMY, {16 * CHARACTER_SCALING, 24 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::QUIT,{204, 56} },
        { TEXTURE_ASSET_ID::QUIT_PRESSED,{204, 56} },
        { TEXTURE_ASSET_ID::ALMANAC,{204, 56} },
        { TEXTURE_ASSET_ID::ALMANAC_PRESSED,{204, 56} },
        { TEXTURE_ASSET_ID::BACK,{204, 56} },
        { TEXTURE_ASSET_ID::BACK_PRESSED,{204, 56} },
        { TEXTURE_ASSET_ID::MENU,{30, 32} },
        { TEXTURE_ASSET_ID::MENU_PRESSED,{30, 32} },
        { TEXTURE_ASSET_ID::PLAY, {204, 56}},
        { TEXTURE_ASSET_ID::PLAY_PRESSED, {204, 56}},
        { TEXTURE_ASSET_ID::SWORD_HELPER, {101, 9}},
        { TEXTURE_ASSET_ID::GUN_HELPER, {153, 9}},
        { TEXTURE_ASSET_ID::GRENADE_HELPER, {314, 9}},
        { TEXTURE_ASSET_ID::ROCKET_HELPER, {150, 9}},
        { TEXTURE_ASSET_ID::WINGED_BOOTS_HELPER, {142, 9}},
        { TEXTURE_ASSET_ID::PICKAXE_HELPER, {199, 9}},
        { TEXTURE_ASSET_ID::DASH_BOOTS_HELPER, {178, 9}},
        { TEXTURE_ASSET_ID::TITLE_TEXT, {600, 120}},
        { TEXTURE_ASSET_ID::HERO, {15*CHARACTER_SCALING, 16*CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::FIRE_ENEMY, {19 * CHARACTER_SCALING, 22 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::GHOUL_ENEMY, {13 * CHARACTER_SCALING, 20 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::FOLLOWING_ENEMY, {12 * CHARACTER_SCALING, 12 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::SPITTER_ENEMY, {16*CHARACTER_SCALING, 24*CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::EXPLOSION, {60, 55}},
        { TEXTURE_ASSET_ID::EXPLOSION, {60, 55}},
        { TEXTURE_ASSET_ID::PLAYER_HEART, {40, 40}},
        { TEXTURE_ASSET_ID::PARALLAX_LAVA, {1200, 42}},
};

const std::map<TEXTURE_ASSET_ID, vec2 > SPRITE_SCALE = {
        { TEXTURE_ASSET_ID::HERO, {52*CHARACTER_SCALING, 21*CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::FIRE_ENEMY, {65 * CHARACTER_SCALING, 50 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::GHOUL_ENEMY, {50 * CHARACTER_SCALING, 28 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::FOLLOWING_ENEMY, {30 * CHARACTER_SCALING, 30 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::SPITTER_ENEMY, {57*CHARACTER_SCALING, 39*CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::EXPLOSION, {100, 92}},
        { TEXTURE_ASSET_ID::PARALLAX_LAVA, {1200, 800}},
};

const float BULLET_MESH_SCALE = 4.0f;

const std::map<TEXTURE_ASSET_ID, vec2 > SPRITE_OFFSET = {
        { TEXTURE_ASSET_ID::HERO, {10*CHARACTER_SCALING, -1*CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::FIRE_ENEMY, { 0 * CHARACTER_SCALING, -6 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::GHOUL_ENEMY, { 0 * CHARACTER_SCALING, -2 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::FOLLOWING_ENEMY, { -1 * CHARACTER_SCALING, -2 * CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::SPITTER_ENEMY, {-10*CHARACTER_SCALING, -6*CHARACTER_SCALING}},
        { TEXTURE_ASSET_ID::EXPLOSION, {0, -8}},
        { TEXTURE_ASSET_ID::PARALLAX_LAVA, {0, -378}},
};

const std::map<TEXTURE_ASSET_ID, AnimationInfo> ANIMATION_INFO = {
        {TEXTURE_ASSET_ID::HERO, {
                13,
                {9, 1, 8, 4, 4, 4, 16, 4, 8, 4, 14, 2, 8},
                0,
                16
        }},
        {TEXTURE_ASSET_ID::FIRE_ENEMY, {
            6,
            {4, 4, 5, 5, 5, 5},
            0,
            5
        }},
       {TEXTURE_ASSET_ID::GHOUL_ENEMY, {
            6,
            {4, 9, 7, 4, 7, 11},
            1,
            11
        }},
        {TEXTURE_ASSET_ID::FOLLOWING_ENEMY, {
            5,
            {6, 4, 6, 6, 5},
            0,
            6
        }},
        {TEXTURE_ASSET_ID::SPITTER_ENEMY, {
            5,
            {6, 7, 8, 3, 8},
            0,
            9
        }},
        {TEXTURE_ASSET_ID::EXPLOSION, {
            1,
            {6},
            0,
            6
        }},
        {TEXTURE_ASSET_ID::SPITTER_ENEMY_BULLET, {
            1,
            {4},
            0,
            4
        }}
};
// the player
Entity createHero(RenderSystem *renderer, vec2 pos);
// the enemy
Entity createFireEnemy(RenderSystem *renderer, vec2 position);
// the ghoul enemy
Entity createGhoul(RenderSystem* renderer, vec2 position);
// the following & teleporting enemy
Entity createFollowingEnemy(RenderSystem* renderer, vec2 position);
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

Entity createGrenade(RenderSystem* renderer, vec2 position, vec2 velocity);

Entity createExplosion(RenderSystem* renderer, vec2 position, float size);

Entity createHeart(RenderSystem* renderer, vec2 position);

Entity createWingedBoots(RenderSystem* renderer, vec2 position);

Entity createDashBoots(RenderSystem* renderer, vec2 position);

Entity createPickaxe(RenderSystem* renderer, vec2 position);

// the parallax backgrounds
Entity createParallaxItem(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID texture_id);
// the helper text during pause
Entity createHelperText(RenderSystem* renderer);
Entity createToolTip(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID type);
Entity createBlock(RenderSystem* renderer, vec2 pos, vec2 size, std::vector<std::vector<char>>& grid);
// the ui button
Entity createButton(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID type, std::function<void ()> callback, bool visibility = true);
Entity createWeaponHitBox(RenderSystem* renderer, vec2 pos, vec2 size);
Entity createTitleText(RenderSystem* renderer, vec2 pos);
Entity createLine(RenderSystem* renderer, vec2 pos, vec2 offset, vec2 scale, float angle);
Entity createPlayerHeart(RenderSystem* renderer, vec2 pos);
Entity createPowerUpIcon(RenderSystem* renderer, vec2 pos);
Entity createDifficultyBar(RenderSystem* renderer, vec2 pos);
Entity createDifficultyIndicator(RenderSystem* renderer, vec2 pos);
Entity createScore(RenderSystem* renderer, vec2 pos);
Entity createNumber(RenderSystem* renderer, vec2 pos);