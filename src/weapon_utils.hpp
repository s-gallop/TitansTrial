#pragma once

#include <vector>
#include <random>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "render_system.hpp"

void rotate_weapon(Entity weapon, vec2 mouse_pos);
void update_weapon_pos(Entity weapon, Entity hero);
void update_weapon_timer(float elapsed_ms, RenderSystem* render);
void do_weapon_action(Entity weapon);