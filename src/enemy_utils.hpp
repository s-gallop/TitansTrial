#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "components.hpp"
#include "world_init.hpp"
#include "world_system.hpp"


void move_firelings(RenderSystem* renderer);

void move_boulder(RenderSystem* renderer);

void move_ghouls(RenderSystem* renderer, Entity player_hero);

void move_tracer(float elapsed_ms_since_last_update, Entity player_hero);

void move_spitters(float elapsed_ms_since_last_update, RenderSystem* renderer);

void boss_action_decision(Entity boss, RenderSystem* renderer);
void boss_action_teleport(Entity boss);
void boss_action_swipe(Entity boss);
void boss_action_summon(Entity boss, RenderSystem* renderer);

