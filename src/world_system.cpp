// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <bitset>
#include <iostream>

#include "physics_system.hpp"

// Game configuration
const size_t MAX_ENEMIES = 15;
const size_t MAX_SWORDS = 3;
const size_t ENEMY_DELAY_MS = 2000 * 3;
const size_t SWORD_DELAY_MS = 8000 * 3;
const uint MAX_JUMPS = 2;

const float BASIC_SPEED = 200.0;
const float JUMP_INITIAL_SPEED = 350.0;
const int ENEMY_SPAWN_HEIGHT_IDLE_RANGE = 50;
const float DDF_PUNISHMENT = 30.0;

// Global Variables (?)
vec2 mouse_pos = {0,0};
bool WorldSystem::pause = false;
std::bitset<2> motionKeyStatus("00");

/* 
* ddl = Dynamic Difficulty Level
* (0 <= ddf < 139) -> (ddl = 0)
* (130 <= ddf < 260) -> (ddl = 1)
* (260 <= ddf <= MAX) -> (ddl = 2)
* Now MAX is 300
*/
int ddl = 0; 

/*
* ddf = Dynamic Difficulty Factor
* formula: (points * 10 + absolute_time / 1000)
*/
float ddf = 0.0f;

// When player hits the enemies, set it to 3000.0f
float invlunerable_timer = 0.0f;

// Create the fish world
WorldSystem::WorldSystem()
	: points(0), next_enemy_spawn(0.f), next_sword_spawn(1000.f)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (hero_dead_sound != nullptr)
		Mix_FreeChunk(hero_dead_sound);
	if (hero_kill_sound != nullptr)
		Mix_FreeChunk(hero_kill_sound);
    if (sword_swing_sound != nullptr)
        Mix_FreeChunk(sword_swing_sound);
    if (hero_jump_sound != nullptr)
        Mix_FreeChunk(hero_jump_sound);
    if (button_click_sound != nullptr)
        Mix_FreeChunk(button_click_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace
{
	void glfw_err_cb(int error, const char *desc)
	{
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window()
{
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Titan's Trial", nullptr, nullptr);
	if (window == nullptr)
	{
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1}); };
    auto cursor_click_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2)
    { ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
    glfwSetMouseButtonCallback(window, cursor_click_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	hero_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav").c_str());
	hero_kill_sound = Mix_LoadWAV(audio_path("salmon_eat.wav").c_str());
	sword_swing_sound = Mix_LoadWAV(audio_path("sword_swing.wav").c_str());
	hero_jump_sound = Mix_LoadWAV(audio_path("hero_jump.wav").c_str());
    button_click_sound = Mix_LoadWAV(audio_path("button_click.wav").c_str());


	if (background_music == nullptr || hero_dead_sound == nullptr || hero_kill_sound == nullptr || sword_swing_sound == nullptr || hero_jump_sound == nullptr || button_click_sound ==
                                                                                                                                                                         nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n %s\n %s\n %s\n make sure the data directory is present",
				audio_path("music.wav").c_str(),
				audio_path("salmon_dead.wav").c_str(),
				audio_path("salmon_eat.wav").c_str(),
				audio_path("sword_swing.wav").c_str(),
				audio_path("hero_jump.wav").c_str(),
                audio_path("button_click.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	// internal data update section
	if (invlunerable_timer > 0.0f) invlunerable_timer = max(invlunerable_timer - elapsed_ms_since_last_update, 0.0f);
	ddf += elapsed_ms_since_last_update / 1000.0f;
	if (ddf >= 260) ddl = 2;
	else if (ddf >= 130 && ddf < 260) ddl = 1;
	else ddl = 0;

	// Updating window title
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	title_ss << "; Dynamic Difficulty Level: " << ddl;
	title_ss << "; Dynamic Difficulty Factor: " << ddf;
	title_ss << "; Player HP: " << registry.players.get(player_hero).hp;
	title_ss << "; Invlunerable Time: " << invlunerable_timer / 1000.0f;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto &motion_container = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motion_container.components.size() - 1; i >= 0; --i)
	{
		Motion &motion = motion_container.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f || motion.position.x - abs(motion.scale.x) > window_width_px || motion.position.y - abs(motion.scale.y) > window_height_px)
		{
			if (!registry.players.has(motion_container.entities[i])) // only remove enemies
				registry.remove_all_components_of(motion_container.entities[i]);
		}

		if (motion.position.y > window_height_px - 5)
		{
			if (registry.players.has(motion_container.entities[i]))
				if (!registry.deathTimers.has(motion_container.entities[i]))
				{
					// Scream, reset timer, and make the hero fall
					registry.deathTimers.emplace(motion_container.entities[i]);
					for (Entity weapon : registry.weapons.entities)
					{
						registry.remove_all_components_of(weapon);
					}
					Mix_PlayChannel(-1, hero_dead_sound, 0);

					Motion &motion = registry.motions.get(player_hero);
					motion.angle = M_PI / 2;
					motion.velocity = vec2(0, 100);
					registry.colors.get(player_hero) = vec3(1, 0, 0);
				}
		}
	}

	// Keep weapons centred on player
	for (Entity entity : registry.weapons.entities)
	{
		Motion &weaponMot = registry.motions.get(entity);
		weaponMot.position = registry.motions.get(player_hero).position;
		uint &swingState = registry.weapons.get(entity).swing;
		switch (swingState)
		{
		case 1:
			weaponMot.angle -= M_PI / 32;
			if (weaponMot.angle <= weaponMot.angleBackup - M_PI / 4)
			{
				swingState = 3;
				float angleBackup = weaponMot.angleBackup;
				vec2 hitBoxPos = weaponMot.position + weaponMot.positionOffset * mat2({cos(angleBackup), -sin(angleBackup)}, {sin(angleBackup), cos(angleBackup)});
				float hbScale = .9 * max(weaponMot.scale.x, weaponMot.scale.y);
				registry.weapons.get(entity).hitBoxes.push_back(createWeaponHitBox(hitBoxPos, {hbScale, hbScale}));
				if (!registry.weaponHitBoxes.get(registry.weapons.get(entity).hitBoxes.front()).soundPlayed) {
					registry.weaponHitBoxes.get(registry.weapons.get(entity).hitBoxes.front()).soundPlayed = true;
					Mix_PlayChannel(-1, sword_swing_sound, 0);
				}
			}
			break;
		case 2:
			weaponMot.angle += M_PI / 32;
			if (weaponMot.angle >= weaponMot.angleBackup + M_PI / 4)
			{
				swingState = 4;
				float angleBackup = weaponMot.angleBackup;
				vec2 hitBoxPos = weaponMot.position + weaponMot.positionOffset * mat2({cos(angleBackup), -sin(angleBackup)}, {sin(angleBackup), cos(angleBackup)});
				float hbScale = .9 * max(weaponMot.scale.x, weaponMot.scale.y);
				registry.weapons.get(entity).hitBoxes.push_back(createWeaponHitBox(hitBoxPos, {hbScale, hbScale}));
				if (!registry.weaponHitBoxes.get(registry.weapons.get(entity).hitBoxes.front()).soundPlayed) {
					registry.weaponHitBoxes.get(registry.weapons.get(entity).hitBoxes.front()).soundPlayed = true;
					Mix_PlayChannel(-1, sword_swing_sound, 0);
				}
			}
			break;
		case 3:
			weaponMot.angle += M_PI / 16;
			if (weaponMot.angle >= weaponMot.angleBackup + M_PI / 4)
			{
				swingState = 0;
				weaponMot.angleBackup = weaponMot.angle;
				for (Entity hitBox : registry.weapons.get(entity).hitBoxes)
				{
					registry.remove_all_components_of(hitBox);
				}
				registry.weapons.get(entity).hitBoxes.clear();
			}
			break;
		case 4:
			weaponMot.angle -= M_PI / 16;
			if (weaponMot.angle <= weaponMot.angleBackup - M_PI / 4)
			{
				swingState = 0;
				weaponMot.angleBackup = weaponMot.angle;
				for (Entity hitBox : registry.weapons.get(entity).hitBoxes)
				{
					registry.remove_all_components_of(hitBox);
				}
				registry.weapons.get(entity).hitBoxes.clear();
			}
		}
	}
	vec2 playerVelocity = registry.motions.get(player_hero).velocity;
	AnimationInfo &playerAnimation = registry.animated.get(player_hero);
	if (playerVelocity.y > 0)
	{
		playerAnimation.curState = 3;
	}
	else if (playerVelocity.y < 0)
	{
		playerAnimation.curState = 2;
	}
	else if (playerVelocity.x != 0)
	{
		playerAnimation.curState = 1;
	}
	else
	{
		playerAnimation.curState = 0;
	}

	if (ddl == 0) current_enemy_spawning_speed = 1.f;
	else if (ddl == 1) current_enemy_spawning_speed = 5.f;
	else current_enemy_spawning_speed = 10.f;

	spawn_move_normal_enemies(elapsed_ms_since_last_update);

	next_sword_spawn -= elapsed_ms_since_last_update * current_speed * 2;
	if (registry.swords.components.size() <= MAX_SWORDS && next_sword_spawn < 0.f)
	{
		// Reset timer
		next_sword_spawn = (SWORD_DELAY_MS / 2) + uniform_dist(rng) * (SWORD_DELAY_MS / 2);
		// Create sword at random position
		float sword_x = uniform_dist(rng) * (window_width_px - 120) + 60;
		float sword_y = uniform_dist(rng) * (window_height_px - 350) + 50;

		createSword(renderer, {sword_x, sword_y});
	}

	// Processing the hero state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState &screen = registry.screenStates.components[0];

	float min_timer_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities)
	{
		// progress timer
		DeathTimer &timer = registry.deathTimers.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < min_timer_ms)
		{
			min_timer_ms = timer.timer_ms;
		}

		// restart the game once the death timer expired
		if (timer.timer_ms < 0)
		{
			registry.deathTimers.remove(entity);
			screen.screen_darken_factor = 0;
			restart_game();
			return true;
		}
	}

	screen.screen_darken_factor = 1 - min_timer_ms / 3000;

	return true;
}

// deal with normal eneimies' spawning and moving
void WorldSystem::spawn_move_normal_enemies(float elapsed_ms_since_last_update)
{
	next_enemy_spawn -= elapsed_ms_since_last_update * current_enemy_spawning_speed;
	if (registry.enemies.components.size() < MAX_ENEMIES && next_enemy_spawn < 0.f)
	{
		// Reset timer
		next_enemy_spawn = (ENEMY_DELAY_MS / 2) + uniform_dist(rng) * (ENEMY_DELAY_MS / 2);
		srand(time(0));
		float squareFactor = rand() % 2 == 0 ? 0.0005 : -0.0005;
		int leftHeight = ENEMY_SPAWN_HEIGHT_IDLE_RANGE + rand() % (window_height_px - ENEMY_SPAWN_HEIGHT_IDLE_RANGE * 2);
		int rightHeight = ENEMY_SPAWN_HEIGHT_IDLE_RANGE + rand() % (window_height_px - ENEMY_SPAWN_HEIGHT_IDLE_RANGE * 2);
		float curveParameter = (float)(rightHeight - leftHeight - window_width_px * window_width_px * squareFactor) / window_width_px;
		Entity newEnemy = createEnemy(renderer, vec2(window_width_px, rightHeight), 0.0, vec2(0.0, 0.0), vec2(ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT));
		TestAI &enemyTestAI = registry.testAIs.get(newEnemy);
		enemyTestAI.departFromRight = true;
		enemyTestAI.a = (float)squareFactor;
		enemyTestAI.b = curveParameter;
		enemyTestAI.c = (float)leftHeight;
	}

	auto &testAI_container = registry.testAIs;
	for (uint i = 0; i < testAI_container.size(); i++)
	{
		TestAI &testAI = testAI_container.components[i];
		Entity entity = testAI_container.entities[i];
		Motion &motion = registry.motions.get(entity);
		if (testAI.departFromRight && motion.position[0] < 0)
		{
			float squareFactor = rand() % 2 == 0 ? 0.0005 : -0.0005;
			int rightHeight = ENEMY_SPAWN_HEIGHT_IDLE_RANGE + rand() % (window_height_px - ENEMY_SPAWN_HEIGHT_IDLE_RANGE * 2);
			motion.position = vec2(0.0, testAI.c);
			float curveParameter = (float)(rightHeight - testAI.c - window_width_px * window_width_px * squareFactor) / window_width_px;
			testAI.departFromRight = false;
			testAI.a = (float)squareFactor;
			testAI.b = curveParameter;
		}
		else if (!testAI.departFromRight && motion.position[0] > window_width_px)
		{
			float squareFactor = rand() % 2 == 0 ? 0.0005 : -0.0005;
			int rightHeight = testAI.a * window_width_px * window_width_px + testAI.b * window_width_px + testAI.c;
			int leftHeight = ENEMY_SPAWN_HEIGHT_IDLE_RANGE + rand() % (window_height_px - ENEMY_SPAWN_HEIGHT_IDLE_RANGE * 2);
			motion.position = vec2(window_width_px, rightHeight);
			float curveParameter = (float)(rightHeight - leftHeight - window_width_px * window_width_px * squareFactor) / window_width_px;
			testAI.departFromRight = true;
			testAI.a = (float)squareFactor;
			testAI.b = curveParameter;
			testAI.c = (float)leftHeight;
		}
		float gradient = 2 * testAI.a * motion.position[0] + testAI.b;
		float basicFactor = sqrt(BASIC_SPEED * BASIC_SPEED / (gradient * gradient + 1));
		float direction = testAI.departFromRight ? -1.0 : 1.0;
		motion.velocity = direction * vec2(basicFactor, gradient * basicFactor);
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
	// global variables at this .cpp to reset, don't forget it!
	motionKeyStatus.reset();
	ddl = 0;
	ddf = 0.0f;
	invlunerable_timer = 0.0f;

	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_enemy_spawning_speed = 1.f;
	current_speed = 1.f;
	points = 0;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();
	// add bg
	createBackground();
	// Create a new hero
	player_hero = createHero(renderer, {100, 200});
	registry.colors.insert(player_hero, {1, 0.8f, 0.8f});
	int background_pixels_width = 768;
	int background_pixels_height = 432;

	int base_height = ceil(16 * window_height_px / background_pixels_height);
	int base_width = ceil(16 * window_width_px / background_pixels_width);

	// bottom line
	createBlock({window_width_px / 2, window_height_px + 100}, {window_width_px, base_height / 2});
	// left line
	createBlock({-base_width, window_height_px / 2}, {base_width * 6, window_height_px});
	// right line
	createBlock({window_width_px + base_width, window_height_px / 2}, {base_width * 6, window_height_px});
	// top line
	createBlock({window_width_px / 2, 0}, {window_width_px, base_height / 2});

	// left middle platform
	createBlock({base_width * 8 - 16, base_height * 12 + 8}, {base_width * 11, base_height * 2});

	// top middle platform
	createBlock({window_width_px / 2, base_height * 6}, {base_width * 26, base_height * 2});

	// right middle platform
	createBlock({window_width_px - base_width * 8 + 16, base_height * 12 + 8}, {base_width * 11, base_height * 2});

	// bottom middle left platform
	createBlock({base_width * 13, base_height * 18 + 8}, {base_width * 10, base_height * 2});

	// bottom middle right platform
	createBlock({window_width_px - base_width * 13, base_height * 18 + 8}, {base_width * 10, base_height * 2});

	// bottom left padding platform
	createBlock({base_width * 6 + 8, window_height_px - base_height * 3}, {base_width * 9, base_height * 4});

	// bottom right padding platform
	createBlock({window_width_px - base_width * 7 + 16, window_height_px - base_height * 3}, {base_width * 9, base_height * 4});

	// bottom center padding platform
	createBlock({window_width_px / 2, window_height_px - base_height * 2}, {base_width * 14, base_height * 2});

    create_pause_screen();
}

// Adds whatever's needed in the pause screen
void WorldSystem::create_pause_screen() {
    createButton({18, 18}, TEXTURE_ASSET_ID::MENU, [&](){change_pause();});
    createButton({window_width_px / 2, window_height_px / 2}, TEXTURE_ASSET_ID::QUIT, [&]() {exit(0);}, false);
    createHelperText();
}

// Compute collisions between entities
void WorldSystem::handle_collisions()
{
	// Loop over all collisions detected by the physics system
	auto &collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++)
	{
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;

		if (registry.players.has(entity))
		{
			Player& player = registry.players.get(entity);

			// Checking Player - Enemies collisions
			if (registry.enemies.has(entity_other) && invlunerable_timer <= 0.0f)
			{
				// remove 1 hp
				player.hp -= 1;
				invlunerable_timer = 3000.0f;
				Mix_PlayChannel(-1, hero_dead_sound, 0);
				ddf = max(ddf - (player.hp_max - player.hp) * DDF_PUNISHMENT, 0.0f);

				// initiate death unless already dying
				if (player.hp == 0 && !registry.deathTimers.has(entity))
				{
					registry.deathTimers.emplace(entity);
					for (Entity weapon : registry.weapons.entities)
					{
						registry.remove_all_components_of(weapon);
					}

					Motion &motion = registry.motions.get(player_hero);
					motion.angle = M_PI / 2;
					motion.velocity = vec2(0, 100);
					registry.colors.get(player_hero) = vec3(1, 0, 0);
				}
			}
			// Checking Player - Sword collision
			else if (registry.swords.has(entity_other))
			{
				if (!registry.deathTimers.has(entity))
				{
					registry.remove_all_components_of(entity_other);
					if (!registry.players.get(player_hero).hasWeapon)
					{
						createWeaponSword(renderer);
						registry.players.get(player_hero).hasWeapon = 1;
					}
				}
			}
			else if (registry.blocks.has(entity_other))
			{
				if ((registry.motions.get(entity).position.y < registry.motions.get(entity_other).position.y + registry.motions.get(entity_other).scale.y / 2) ||
					(registry.motions.get(entity).position.x < registry.motions.get(entity_other).position.x - registry.motions.get(entity_other).scale.x / 2) ||
					(registry.motions.get(entity).position.x > registry.motions.get(entity_other).position.x + registry.motions.get(entity_other).scale.x / 2))
				{
					registry.players.get(entity).jumps = MAX_JUMPS;
				}
			}
		}
		else if (registry.swords.has(entity))
		{

			if (registry.blocks.has(entity_other))
			{
				registry.gravities.remove(entity);
				registry.motions.get(entity).velocity = vec2(0, 0);

				if (registry.motions.get(entity).position.y > 600 && (registry.motions.get(entity).position.x < 190 || registry.motions.get(entity).position.x > 1010))
				{
					registry.motions.get(entity).position = vec2(registry.motions.get(entity).position.x, registry.motions.get(entity_other).position.y - 90);
				}
				else
				{
					registry.motions.get(entity).position = vec2(registry.motions.get(entity).position.x, registry.motions.get(entity_other).position.y - 55);
				}
			}
		}
		else if (registry.weaponHitBoxes.has(entity))
		{
			if (registry.enemies.has(entity_other))
			{
				if (!registry.deathTimers.has(player_hero))
				{
					registry.remove_all_components_of(entity_other);
					++points;
					ddf += 10.0f;
				}
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return bool(glfwWindowShouldClose(window));
}

void motion_helper(Motion &playerMotion)
{
	float rightFactor = motionKeyStatus.test(0) ? 1 : 0;
	float leftFactor = motionKeyStatus.test(1) ? -1 : 0;
	playerMotion.velocity[0] = BASIC_SPEED * (rightFactor + leftFactor);
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        change_pause();
        Mix_PlayChannel(-1, button_click_sound, 0);
    }

	if (!registry.deathTimers.has(player_hero))
	{
		Motion &playerMotion = registry.motions.get(player_hero);

		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			motionKeyStatus.set(0);
			playerMotion.scale.x = abs(playerMotion.scale.x);
		}
		else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			motionKeyStatus.reset(0);
			if (motionKeyStatus.test(1))
				playerMotion.scale.x = -abs(playerMotion.scale.x);
		}
		else if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			motionKeyStatus.set(1);
			playerMotion.scale.x = -abs(playerMotion.scale.x);
		}
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			motionKeyStatus.reset(1);
			if (motionKeyStatus.test(0))
				playerMotion.scale.x = abs(playerMotion.scale.x);
		}

		motion_helper(playerMotion);

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			if (registry.players.get(player_hero).jumps > 0)
			{
				playerMotion.velocity[1] = -JUMP_INITIAL_SPEED;
				registry.players.get(player_hero).jumps--;
				Mix_PlayChannel(-1, hero_jump_sound, 0);
			}
		}

		if (key == GLFW_KEY_SPACE)
		{
			for (Entity entity : registry.weapons.entities)
			{
				uint &swingState = registry.weapons.get(entity).swing;
				if (swingState == 0)
				{
					float weaponAngle = registry.motions.get(entity).angle;
					swingState = (weaponAngle < 0 || weaponAngle > M_PI) ? 2 : 1;
				}
			}
		}
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_B)
	{
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	/*
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
	{
		current_speed -= 0.1f;
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
	{
		current_speed += 0.1f;
	}
	current_speed = fmax(0.f, current_speed);
	*/
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{
	if (!registry.deathTimers.has(player_hero) && !pause)
	{
		for (Entity entity : registry.weapons.entities)
		{
			if (registry.weapons.get(entity).swing == 0)
			{
				Motion &motion = registry.motions.get(entity);
				motion.angle = atan2(mouse_position.y - motion.position.y, mouse_position.x - motion.position.x) + M_PI / 2;
				motion.angleBackup = motion.angle;
			}
		}
	}
    mouse_pos = mouse_position;
}
void WorldSystem::on_mouse_click(int key, int action, int mods){
    // button click check
    for(Entity entity : registry.buttons.entities) {
        Motion &button = registry.motions.get(entity);
        Button &buttonInfo = registry.buttons.get(entity);
        RenderRequest &buttonRender = registry.renderRequests.get(entity);
        if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE && buttonInfo.clicked == true) {
            buttonInfo.clicked = false;
            buttonInfo.callback();
        }
        if (abs(button.position.x - mouse_pos.x) < button.scale.x / 2 && abs(button.position.y - mouse_pos.y) < button.scale.y / 2) {
            if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS && buttonRender.visibility) {
                buttonInfo.clicked = true;
                Mix_PlayChannel(-1, button_click_sound, 0);
            }
        }
    }
}

// pause/unpauses game and show pause screen entities
void WorldSystem::change_pause() {
    pause = !pause;
    for (Entity e : registry.showWhenPaused.entities) {
        registry.renderRequests.get(e).visibility = pause;
    }
}
