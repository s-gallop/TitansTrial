#pragma once

#include <vector>
#include <random>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "render_system.hpp"

void collect(Entity weapon, Entity hero);
void rotate_weapon(Entity weapon, vec2 mouse_pos);
void update_weapon(RenderSystem* renderer, float elapsed_ms, Entity weapon, Entity hero);
void update_collectable_timer(float elapsed_ms, RenderSystem* render, int ddl);
void do_weapon_action(RenderSystem* renderer, Entity weapon);
void use_pickaxe(Entity hero, uint direction, size_t max_jumps);
void update_lodged(Entity hero, std::bitset<2> motionKeyStatus);
void check_dash_boots(Entity hero, uint direction);
void update_dash_boots(float elapsed_ms, Entity hero, std::bitset<2> motionKeyStatus, float speed);