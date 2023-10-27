#pragma once

#include <vector>
#include <random>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "render_system.hpp"

void collect_weapon(Entity weapon, Entity hero);
void rotate_weapon(Entity weapon, vec2 mouse_pos);
void update_weapon(RenderSystem* renderer, float elapsed_ms, Entity weapon, Entity hero);
void update_collectable_timer(float elapsed_ms, RenderSystem* render);
void do_weapon_action(RenderSystem* renderer, Entity weapon);