#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

// #define SDL_MAIN_HANDLED
// #include <SDL.h>
// #include <SDL_mixer.h>

#include "render_system.hpp"
#include "sound_utils.hpp"
#include "weapon_utils.hpp"
#include "ai_system.hpp"
#include <map>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();
    // somehow pause it activated once
    static bool pause;
	static bool isTitleScreen;
	// Creates a window
	GLFWwindow *create_window();

	// starts the game
	void init(RenderSystem *renderer);
	
	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms_since_last_update);

	// spawn normal enemies (refactor)
	void spawn_move_normal_enemies(float elapsed_ms_since_last_update);

	// spawn following enemies (refactor)
	void spawn_move_following_enemies(float elapsed_ms_since_last_update);

	// spawn normal enemies (refactor)
	void spawn_spitter_enemy(float elapsed_ms_since_last_update);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over() const;

    static void change_pause();

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
    void on_mouse_click(int button, int action, int mods);

	void motion_helper(Motion& playerMotion);

	// restart level
	void restart_game();


    // creates pause gui
    void create_pause_screen();
	void create_title_screen();
	// OpenGL window handle
	GLFWwindow *window;

	// Number of enemies killed, displayed in the window title
	unsigned int points;

	// Game state
	RenderSystem *renderer;
	float current_enemy_spawning_speed;
	float current_speed;
	float next_enemy_spawn;
	Entity player_hero;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
