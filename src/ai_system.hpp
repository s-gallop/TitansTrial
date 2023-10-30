#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 3
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class AISystem
{


public:
	void step(float elapsed_ms);

	friend vec2 AISystem::find_map_index(vec2 pos);

	friend vec2 AISystem::find_index_from_map(vec2 pos);

	friend std::list<vec2> AISystem::dfs_follow_start(std::vector<std::vector<char>>& vec, vec2 pos_chase, vec2 pos_prey);

	friend std::vector<std::vector<char>> AISystem::create_grid();

private:
	std::list<vec2> AISystem::dfs_follow_helper(std::vector<std::vector<char>>& vec, vec2 pos, std::list<vec2> path);
};

