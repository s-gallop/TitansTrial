// internal
#include "ai_system.hpp"

const float RATIO_WIDTH = 78.f;
const float OFFSET_WIDTH = 10.f;
const float RATIO_HEIGHT = 90.f;
const float OFFSET_HEIGHT = -25.f;

void AISystem::step(float elapsed_ms)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
}


vec2 find_map_index(vec2 pos) {
	pos.x = (pos.x - OFFSET_WIDTH) / RATIO_WIDTH;
	pos.y = (pos.y + OFFSET_HEIGHT) / RATIO_HEIGHT;

	return round(pos) - 1.f;
}

vec2 find_index_from_map(vec2 pos) {
	//printf("before: %f, %f==========", pos.x, pos.y);
	pos.x = (int)pos.x * RATIO_WIDTH + OFFSET_WIDTH;
	pos.y = (int)pos.y * RATIO_HEIGHT - OFFSET_HEIGHT;
	//printf("AFTER: %f, %f\n", pos.x, pos.y);

	return pos;
}

std::list<vec2> dfs_follow_helper(std::vector<std::vector<char>>& vec, vec2 pos, std::list<vec2> path) {
	if (vec[pos.y][pos.x] == 'g') return path;
	path.push_front(vec2(pos));
	vec[pos.y][pos.x] = 'v';
	std::list<vec2> ret_path(300, vec2(1, 1));
	std::list<vec2> temp_path(300, vec2(1, 1));

	//visit right
	if (pos.x + 1.f < vec[0].size() && pos.x - 1 >= 0 && vec[pos.y][pos.x + 1.f] != 'v' && vec[pos.y][pos.x + 1.f] != 'b') {
		ret_path = dfs_follow_helper(vec, vec2(pos.x + 1.f, pos.y), path);
	}
	//visit left
	if (pos.x - 1.f < vec[0].size() && pos.x - 1.f >= 0 && vec[pos.y][pos.x - 1.f] != 'v' && vec[pos.y][pos.x - 1.f] != 'b') {
		temp_path = dfs_follow_helper(vec, vec2(pos.x - 1.f, pos.y), path);
	}
	if (ret_path.size() == 0) ret_path = temp_path;
	if (ret_path.size() > temp_path.size()) ret_path = temp_path;
	//visit up
	if (pos.y - 1.f < vec.size() && pos.y - 1.f >= 0 && vec[pos.y - 1.f][pos.x] != 'v' && vec[pos.y - 1.f][pos.x] != 'b') {
		temp_path = dfs_follow_helper(vec, vec2(pos.x, pos.y - 1.f), path);
	}
	if (ret_path.size() == 0) ret_path = temp_path;
	if (ret_path.size() > temp_path.size()) ret_path = temp_path;
	//visit down
	if (pos.y + 1.f < vec.size() && pos.y + 1.f >= 0 && vec[pos.y + 1.f][pos.x] != 'v' && vec[pos.y + 1.f][pos.x] != 'b') {
		temp_path = dfs_follow_helper(vec, vec2(pos.x, pos.y + 1.f), path);
	}
	if (ret_path.size() == 0) ret_path = temp_path;
	if (ret_path.size() > temp_path.size()) ret_path = temp_path;

	return ret_path;
}

std::list<vec2> dfs_follow_start(std::vector<std::vector<char>>& vec, vec2 pos_chase, vec2 pos_prey) {
	vec[pos_prey.y][pos_prey.x] = 'g';
	std::list<vec2> path;
	path = dfs_follow_helper(vec, pos_chase, path);
	return path;
}

std::vector<std::vector<char>> create_grid() {
	std::vector <char> line(14, 'n');
	std::vector<std::vector<char> > vect(9, line);

	std::vector<char> temp = { 'n', 'n', 'n', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'b', 'n', 'n', 'n'};
	vect[2] = temp;
	temp = { 'b', 'b', 'b', 'b', 'n', 'n', 'n', 'n', 'n', 'n', 'b', 'b', 'b', 'b' };
	vect[4] = temp;
	temp = {  'n', 'n', 'b', 'b', 'b', 'n', 'n', 'n', 'n', 'b', 'b', 'b', 'n', 'n' };
	vect[6] = temp;
	temp = {  'b', 'b', 'b', 'n', 'n', 'b', 'b', 'b', 'b', 'n', 'n', 'b', 'b', 'b' };
	vect[8] = temp;

	return vect;
}