#pragma once

#include <vector>
#include <random>
#include <math.h>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "render_system.hpp"

void collect(Entity weapon, Entity hero);
void update_weapon_angle(RenderSystem* renderer, Entity weapon, vec2 mouse_pos);
void update_equipment(float elapsed_ms, Entity hero);
void update_grenades(RenderSystem* renderer, float elapsed_ms);
void update_lasers(RenderSystem* renderer, float elapsed_ms);
void explode(RenderSystem* renderer, vec2 position, Entity explodable);
void update_explosions(float elapsed_ms);
void update_weapon(RenderSystem* renderer, float elapsed_ms, Entity hero, bool mouse_clicked);
void update_collectable_timer(float elapsed_ms, RenderSystem* render, int ddl);
void do_weapon_action(RenderSystem* renderer, Entity weapon, vec2 mouse_pos);
void use_pickaxe(Entity hero, uint direction, size_t max_jumps);
void disable_pickaxe(Entity hero, uint direction, float disable_time);
void update_pickaxe(float elapsed_ms);
void check_dash_boots(Entity hero, uint direction);
void update_dash_boots(float elapsed_ms, Entity hero, std::bitset<2> motionKeyStatus, float speed);