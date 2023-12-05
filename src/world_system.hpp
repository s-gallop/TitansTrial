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



// Render configs
const float ANIMATION_SPEED_FACTOR = 10.0f;

// Game configuration
const size_t MAX_FIRE_ENEMIES = 10;
const size_t MAX_FOLLOWING_ENEMIES = 1;
const size_t MAX_GHOULS = 5;
const size_t MAX_SPITTERS = 3;
const float ENEMY_INVULNERABILITY_TIME = 500.f;
const size_t ENEMY_DELAY_MS = 2000 * 3;
const size_t SPITTER_SPAWN_DELAY_MS = 10000 * 3;
const float SPITTER_PROJECTILE_DELAY_MS = 5000.f;
const float INITIAL_SPITTER_PROJECTILE_DELAY_MS = 1000.f;
const float SPITTER_PROJECTILE_REDUCTION_FACTOR = 5000.f;
const float SPITTER_PROJECTILE_MIN_SIZE = 0.3f;
const uint SPITTER_PROJECTILE_AMT = 10;
const uint MAX_JUMPS = 2;
const float BASIC_SPEED = 200.f;
const float JUMP_INITIAL_SPEED = 350.f;
const int ENEMY_SPAWN_HEIGHT_IDLE_RANGE = 50;
const float DDF_PUNISHMENT = 10.f;
const float HEART_START_POS = 70.f;
const float HEART_GAP = 35.f;
const float HEART_Y_CORD = 20.f;
const vec2 POWER_CORD = { 20.f, 60.f };
const vec2 DIFF_BAR_CORD = { 140.f, 750.f };
const vec2 INDICATOR_START_CORD = { 35.f, 710.f };
const vec2 INDICATOR_LEVEL_ONE_CORD = { 85.f, 710.f };
const vec2 INDICATOR_LEVEL_TWO_CORD = { 140.f, 710.f };
const vec2 INDICATOR_LEVEL_THREE_CORD = { 195.f, 710.f };
const vec2 INDICATOR_END_CORD = { 250.f, 710.f };
const float INDICATOR_VECLOCITY = 55.f / 100.f;
const vec2 SCORE_CORD = { 1050.f, 700.f };
const float NUMBER_START_POS = 992.f;
const float NUMBER_GAP = 29.f;
const float NUMBER_Y_CORD = 740.f;
const vec2 DB_FLAME_CORD = { 145.f, 693.f };
const vec2 DB_SATAN_CORD = { 140.f, 730.f };

class WorldSystem
{
public:
	WorldSystem();
    // somehow pause it activated once
    static bool pause;
    static bool debug;
	static bool mouse_clicked;
	static bool isTitleScreen;
	// Creates a window
	GLFWwindow *create_window();

	// starts the game
	void init(RenderSystem *renderer);
	
	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms_since_last_update);
  
	void changeScore(int score);

	TEXTURE_ASSET_ID connectNumber(int digit);

	void update_graphics_all_enemies();

	// spawn normal enemies (refactor)
	void spawn_move_normal_enemies(float elapsed_ms_since_last_update);

	// spawn ghoul enemies (refactor)
	void spawn_move_ghouls(float elapsed_ms_since_last_update);

	// spawn following enemies (refactor)
	void spawn_move_following_enemies(float elapsed_ms_since_last_update);

	// spawn splitter enemies
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

	void clear_enemies();

	void motion_helper(Motion& playerMotion);

	// restart level
	void restart_game();

	void save_game();

	void load_game();

	int save_weapon(Entity weapon);

    // creates pause gui
    void create_pause_screen();
	void create_title_screen();
	void create_almanac_screen();
	void create_inGame_GUIs();
	// OpenGL window handle
	GLFWwindow *window;

	// Number of enemies killed, displayed in the window title
	unsigned int points;
	
	// backgrounds
    void create_parallax_background();
	Entity parallax_background;
	Entity parallax_clouds_far_1;
	Entity parallax_clouds_far_2;
	Entity parallax_clouds_close_1;
	Entity parallax_clouds_close_2;
	Entity parallax_rain_1;
	Entity parallax_rain_2;
	Entity parallax_rain_3;
	Entity parallax_rain_4;
	Entity parallax_moon;
	Entity parallax_lava_1;
	Entity parallax_lava_2;
	Entity parallax_lava_3;

	// Game state
	RenderSystem *renderer;
	float current_speed;
	float current_enemy_spawning_speed;
	float current_spitter_spawning_speed;
	float current_ghoul_spawning_speed;
	float next_enemy_spawn;
	float next_spitter_spawn;
	float next_ghoul_spawn;
	Entity player_hero;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
