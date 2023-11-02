#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"

class AISystem
{

public:
	void step(float elapsed_ms);

	AISystem() {}
};

vec2 find_map_index(vec2 pos);

vec2 find_index_from_map(vec2 pos);

std::list<vec2> dfs_follow_start(std::vector<std::vector<char>> &vec, vec2 pos_chase, vec2 pos_prey);

std::vector<std::vector<char>> create_grid();
