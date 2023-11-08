// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "physics_system.hpp"
#include "ai_system.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <bitset>
#include <iostream>
#include <map>

// Game configuration
const size_t MAX_ENEMIES = 15;
const size_t MAX_FOLLOWING_ENEMIES = MAX_ENEMIES/5;
const size_t MAX_SPITTERS = 3;
const size_t ENEMY_DELAY_MS = 2000 * 3;
const size_t SPITTER_DELAY_MS = 10000 * 3;
const size_t SPITTER_PROJECTILE_DELAY_MS = 5000;
const uint MAX_JUMPS = 2;
const float BASIC_SPEED = 200.0;
const float JUMP_INITIAL_SPEED = 350.0;
const int ENEMY_SPAWN_HEIGHT_IDLE_RANGE = 50;
const float DDF_PUNISHMENT = 30.0;
// Global Variables (?)
vec2 mouse_pos = {0,0};
bool WorldSystem::pause = false;
bool WorldSystem::isTitleScreen = true;
std::bitset<2> motionKeyStatus("00");
vec3 player_color;
std::vector<std::vector<char>> grid_vec = create_grid();

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

// Create the fish world
WorldSystem::WorldSystem()
	: points(0), next_enemy_spawn(0.f), next_spitter_spawn(0.f)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{
	// Destroy all sound
	destroy_sound();
	
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

	// Initialize all sound
	if (init_sound())
		return nullptr;

	return window;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{
	this->renderer = renderer_arg;
	
	// Play background music
	play_music();

	// Set all states to default
	if (isTitleScreen) {
		create_title_screen();
	}
	
	
}


void WorldSystem::create_title_screen() 
{
	glfwSetWindowTitle(window, "Titan's Trial");
	ScreenState& screen = registry.screenStates.components[0];
	screen.screen_darken_factor = 0;
	isTitleScreen = true;
	
	pause = false;
	
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());
	
	createTitleText(renderer, { window_width_px / 2, 150 });
	createButton(renderer, { window_width_px / 2, 450 }, TEXTURE_ASSET_ID::PLAY, [&]() {restart_game(); });
	createButton(renderer, { window_width_px / 2, 550 }, TEXTURE_ASSET_ID::QUIT, [&]() {exit(0); });
	
}



// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	// internal data update section
	if (registry.players.get(player_hero).invulnerable_timer > 0.0f)
	{
		float expectedTimer = registry.players.get(player_hero).invulnerable_timer - elapsed_ms_since_last_update;
		if (expectedTimer <= 0.0f) {
			registry.players.get(player_hero).invulnerable_timer = 0.0f;
			registry.colors.get(player_hero) = player_color;
		}
		else
		{
			registry.players.get(player_hero).invulnerable_timer = expectedTimer;
		}
	}
	ddf = min(ddf + elapsed_ms_since_last_update / 1000.0f, 350.0f);
	if (ddf >= 260)
		ddl = 2;
	else if (ddf >= 130 && ddf < 260)
		ddl = 1;
	else
		ddl = 0;

	// Updating window title
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	title_ss << "; Dynamic Difficulty Level: " << ddl;
	title_ss << "; Dynamic Difficulty Factor: " << ddf;
	title_ss << "; Player HP: " << registry.players.get(player_hero).hp;
	title_ss << "; Invlunerable Time: " << registry.players.get(player_hero).invulnerable_timer / 1000.0f;
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
					play_sound(SOUND_EFFECT::HERO_DEAD);

					Motion &motion = registry.motions.get(player_hero);
					motion.angle = M_PI / 2;
					motion.velocity = vec2(0, 100);
					registry.colors.get(player_hero) = vec3(1, 0, 0);
				}
		}

		if (motion.position.x + abs(motion.scale.x) < 0.f || motion.position.x - abs(motion.scale.x) > window_width_px || motion.position.y - abs(motion.scale.y) > window_height_px || motion.position.y + abs(motion.scale.y) < 0.f)
		{
			if (!registry.players.has(motion_container.entities[i]) && !registry.weapons.has(motion_container.entities[i]) && !registry.blocks.has(motion_container.entities[i])) // only remove enemies
				registry.remove_all_components_of(motion_container.entities[i]);
		}
		
	}

	// Keep weapons centred on player
	for (Entity weapon: registry.weapons.entities)
		update_weapon(renderer, elapsed_ms_since_last_update * current_speed, weapon, player_hero);

	COLLECTABLE_TYPE equipment_type = registry.players.get(player_hero).equipment_type;
	if (equipment_type == COLLECTABLE_TYPE::DASH_BOOTS)
		update_dash_boots(elapsed_ms_since_last_update * current_speed, player_hero, motionKeyStatus, BASIC_SPEED);
	else if (equipment_type == COLLECTABLE_TYPE::PICKAXE)
		update_pickaxe(elapsed_ms_since_last_update * current_speed);

	// Animation Stuff	
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

	if (ddl == 0)
	{
		current_enemy_spawning_speed = 1.0f;
		current_spitter_spawning_speed = 0.0f;
	}
	else if (ddl == 1)
	{
		current_enemy_spawning_speed = 1.2f;
		current_spitter_spawning_speed = 0.0f;
	}
	else
	{
		current_enemy_spawning_speed = 1.0f;
		current_spitter_spawning_speed = 1.0f;
	}

	spawn_move_normal_enemies(elapsed_ms_since_last_update);
	spawn_spitter_enemy(elapsed_ms_since_last_update);

	update_collectable_timer(elapsed_ms_since_last_update * current_speed, renderer, ddl);

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

// deal with normal eneimies' spawning and moving
void WorldSystem::spawn_move_following_enemies(float elapsed_ms_since_last_update)
{
	next_enemy_spawn -= elapsed_ms_since_last_update * current_enemy_spawning_speed;
	if (registry.enemies.components.size() < MAX_FOLLOWING_ENEMIES && next_enemy_spawn < 0.f)
	{
		// Reset timer
		next_enemy_spawn = (ENEMY_DELAY_MS / 2) + uniform_dist(rng) * (ENEMY_DELAY_MS / 2);
		srand(time(0));
		float squareFactor = rand() % 2 == 0 ? 0.0005 : -0.0005;
		Entity newEnemy = createEnemy(renderer, find_index_from_map(vec2(7, 4)), 0.0, vec2(0.0, 0.0), vec2(ENEMY_BB_WIDTH/2, ENEMY_BB_HEIGHT/2));
		registry.enemies.get(newEnemy).follows = true;
		registry.colors.emplace(newEnemy);
		registry.colors.get(newEnemy) = vec3(0.f, 1.f, 0.f);

		std::vector<std::vector<char>> vec = grid_vec;
		registry.enemies.get(newEnemy).path = dfs_follow_start(vec, find_map_index(registry.motions.get(newEnemy).position), find_map_index(registry.motions.get(player_hero).position));
		registry.enemies.get(newEnemy).cur_dest = find_index_from_map(registry.enemies.get(newEnemy).path.back());
		registry.enemies.get(newEnemy).path.pop_back();
	}
	float dist;

	Motion& hero_motion = registry.motions.get(player_hero);
	for (uint i = 0; i < registry.enemies.entities.size(); i++) {
		Entity enemy = registry.enemies.entities[i];
		Motion& enemy_motion = registry.motions.get(enemy);
		Enemies& enemy_reg = registry.enemies.get(enemy);
		
		if(enemy_reg.follows)
		{
			dist = ((enemy_reg.cur_dest).x - enemy_motion.position.x) * ((enemy_reg.cur_dest).x - enemy_motion.position.x) + ((enemy_reg.cur_dest).y - enemy_motion.position.y) * ((enemy_reg.cur_dest).y - enemy_motion.position.y);
			if (sqrt(dist) > 10) {
				vec2 following_direction = enemy_reg.cur_dest - enemy_motion.position;
				following_direction = following_direction / sqrt(dot(following_direction, following_direction));
				enemy_motion.velocity = following_direction * (BASIC_SPEED / 4.f);
			}
			else if (enemy_reg.path.size() == 0) {
				std::vector<std::vector<char>> vec = grid_vec;
				enemy_reg.path = dfs_follow_start(vec, find_map_index(enemy_motion.position), find_map_index(hero_motion.position));
				enemy_reg.cur_dest = find_index_from_map(enemy_reg.path.back());
				enemy_reg.path.pop_back();
			}
			else {
				enemy_reg.cur_dest = find_index_from_map(enemy_reg.path.back());
				enemy_reg.path.pop_back();

				enemy_motion.velocity = vec2(0, 0);
			}
		}
	}
}

void WorldSystem::spawn_spitter_enemy(float elapsed_ms_since_last_update) {
	next_spitter_spawn -= elapsed_ms_since_last_update * current_spitter_spawning_speed;
	if (registry.spitterEnemies.components.size() < MAX_SPITTERS && next_spitter_spawn < 0.f)
	{
		next_spitter_spawn = (SPITTER_DELAY_MS / 2) + uniform_dist(rng) * (SPITTER_DELAY_MS / 2);
		float x_pos = uniform_dist(rng) * (window_width_px - 120) + 60;
		float y_pos = uniform_dist(rng) * (window_height_px - 350) + 50;
		createSpitterEnemy(renderer, { x_pos, y_pos });
	}

	auto &spitterEnemy_container = registry.spitterEnemies;
	for (uint i = 0; i < spitterEnemy_container.size(); i++)
	{
		SpitterEnemy &spitterEnemy = spitterEnemy_container.components[i];
		spitterEnemy.timeUntilNextShotMs -= elapsed_ms_since_last_update * current_speed;
		Entity entity = spitterEnemy_container.entities[i];
		Motion &motion = registry.motions.get(entity);
		// AnimationInfo &animation = registry.animated.get(entity);
		if (spitterEnemy.bulletsRemaining > 0 && spitterEnemy.timeUntilNextShotMs <= 0.f)
		{
			// attack animation
			// animation.curState = 1;
			// create bullet at same position as enemy
			Entity spitterBullet = createSpitterEnemyBullet(renderer, motion.position, motion.angle);
			float absolute_scale_x = abs(registry.motions.get(entity).scale[0]);
			if (registry.motions.get(spitterBullet).velocity[0] < 0.0f)
				registry.motions.get(entity).scale[0] = -absolute_scale_x;
			else
				registry.motions.get(entity).scale[0] = absolute_scale_x;

			spitterEnemy.bulletsRemaining -= 1;
			spitterEnemy.timeUntilNextShotMs = SPITTER_PROJECTILE_DELAY_MS;

			// TODO: fix animation
			// animation.curState = 0;
		}
	}

	// decay spitter bullets
	auto& spitterBullets_container = registry.spitterBullets;
	for (uint i = 0; i < spitterBullets_container.size(); i++)
	{
		SpitterBullet& spitterBullet = spitterBullets_container.components[i];
		Entity entity = spitterBullets_container.entities[i];
		Motion& motion = registry.motions.get(entity);
		// make bullets smaller over time
		motion.scale = vec2(motion.scale.x / spitterBullet.mass, motion.scale.y / spitterBullet.mass);
		spitterBullet.mass -= elapsed_ms_since_last_update / 5000;
		motion.scale = vec2(motion.scale.x * spitterBullet.mass, motion.scale.y * spitterBullet.mass);
		if (spitterBullet.mass <= 0.2)
		{
			spitterBullet.mass = 0;
			spitterBullets_container.remove(entity);
			registry.motions.remove(entity);
		}
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
  isTitleScreen = false;
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;
	current_enemy_spawning_speed = 1.f;
	current_spitter_spawning_speed = 1.f;
	points = 0;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());
	// Debugging for memory/component leaks
	registry.list_all_components();
	// add bg
	
	createBackground(renderer);
	// Create a new hero
	player_hero = createHero(renderer, { 100, 200 });
	registry.colors.insert(player_hero, { 1, 0.8f, 0.8f });
	int background_pixels_width = 768;
	int background_pixels_height = 432;

	int base_height = ceil(16 * window_height_px / background_pixels_height);
	int base_width = ceil(16 * window_width_px / background_pixels_width);


	// global variables at this .cpp to reset, don't forget it!
	motionKeyStatus.reset();
	ddl = 0;
	ddf = 0.0f;
	player_color = registry.colors.get(player_hero);

	// bottom line
	createBlock(renderer, {window_width_px / 2, window_height_px + 100}, {window_width_px, base_height / 2});
	// left line
	createBlock(renderer, {-base_width, window_height_px / 2 - 100}, {base_width * 6, window_height_px});
	// right line
	createBlock(renderer, {window_width_px + base_width, window_height_px / 2 - 100}, {base_width * 6, window_height_px});
	// top line
	createBlock(renderer, {window_width_px / 2, -100.f}, {window_width_px, base_height / 2});

	// left middle platform
	createBlock(renderer, {base_width * 8 - 16, base_height * 12 + 8}, {base_width * 11, base_height * 2});

	// top middle platform
	createBlock(renderer, {window_width_px / 2, base_height * 6}, {base_width * 26, base_height * 2});

	// right middle platform
	createBlock(renderer, {window_width_px - base_width * 8 + 16, base_height * 12 + 8}, {base_width * 11, base_height * 2});

	// bottom middle left platform
	createBlock(renderer, {base_width * 13, base_height * 18 + 8}, {base_width * 10, base_height * 2});

	// bottom middle right platform
	createBlock(renderer, {window_width_px - base_width * 13, base_height * 18 + 8}, {base_width * 10, base_height * 2});

	// bottom left padding platform
	createBlock(renderer, {base_width * 6 + 8, window_height_px - base_height * 3}, {base_width * 9, base_height * 4});

	// bottom right padding platform
	createBlock(renderer, {window_width_px - base_width * 7 + 16, window_height_px - base_height * 3}, {base_width * 9, base_height * 4});

	// bottom center padding platform
	createBlock(renderer, {window_width_px / 2, window_height_px - base_height * 2}, {base_width * 14, base_height * 2});
	
	// Adds whatever's needed in the pause screen
	create_pause_screen();

	//testing screen dimensions
	/*for (int i = 10; i < window_width_px; i += ENEMY_BB_WIDTH) {
		for (int j = -25; j < window_height_px; j += ENEMY_BB_HEIGHT) {
			createEnemy(renderer, vec2(i, j), 0.0, vec2(0.0, 0.0), vec2(ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT));
		}
	}*/
}

void WorldSystem::create_pause_screen() {
    createButton(renderer, {18, 18}, TEXTURE_ASSET_ID::MENU, [&](){change_pause();});
    createButton(renderer, {window_width_px / 2, window_height_px / 2}, TEXTURE_ASSET_ID::QUIT, [&]() {create_title_screen(); }, false);
    createHelperText(renderer);
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
			if ((registry.enemies.has(entity_other) || registry.spitterBullets.has(entity_other) || registry.spitterEnemies.has(entity_other)) && registry.players.get(player_hero).invulnerable_timer <= 0.0f && !registry.gravities.get(player_hero).dashing)
			{
				// remove 1 hp
				player.hp -= 1;
				registry.players.get(player_hero).invulnerable_timer = 3000.0f;
				play_sound(SOUND_EFFECT::HERO_DEAD);
				ddf = max(ddf - (player.hp_max - player.hp) * DDF_PUNISHMENT, 0.0f);

				// initiate death unless already dying
				if (player.hp == 0 && !registry.deathTimers.has(entity))
				{
					registry.deathTimers.emplace(entity);
					
					for (Entity weapon : registry.weapons.entities)
						registry.remove_all_components_of(weapon);

					Motion &motion = registry.motions.get(player_hero);
					motion.angle = M_PI / 2;
					motion.velocity = vec2(0, 100);
					registry.colors.get(player_hero) = vec3(1, 0, 0);
				}
			}
			// Checking Player - Collectable collision
			else if (registry.collectables.has(entity_other))
			{
				if (!registry.deathTimers.has(entity))
				{
					collect(entity_other, player_hero);
				}
			}
		}
		else if (registry.weaponHitBoxes.has(entity))
		{
			if (registry.enemies.has(entity_other) || registry.spitterEnemies.has(entity_other))
			{
				if (!registry.deathTimers.has(player_hero))
				{
					registry.remove_all_components_of(entity_other);
					if (registry.bullets.has(entity))
						registry.remove_all_components_of(entity);
					++points;
					ddf += 10.0f;
				}
			} else if (registry.blocks.has(entity_other) && registry.bullets.has(entity)) {
				registry.remove_all_components_of(entity);
			}
		}
		else if (registry.blocks.has(entity))
		{
			if (registry.collectables.has(entity_other) || registry.spitterEnemies.has(entity_other)) {
				registry.gravities.remove(entity_other);
				registry.motions.get(entity_other).velocity = vec2(0, 0);

				if (registry.motions.get(entity_other).position.y > 600 && (registry.motions.get(entity_other).position.x < 190 || registry.motions.get(entity_other).position.x > 1010))
				{
					registry.motions.get(entity_other).position = vec2(registry.motions.get(entity_other).position.x, registry.motions.get(entity).position.y - 90);
				}
				else
				{
					registry.motions.get(entity_other).position = vec2(registry.motions.get(entity_other).position.x, registry.motions.get(entity).position.y - 55);
				}
			}
			else if (registry.players.has(entity_other))
			{
				if ((registry.motions.get(entity_other).position.y <= registry.motions.get(entity).position.y - registry.motions.get(entity).scale.y / 2 - registry.motions.get(entity_other).scale.y / 2))
				{
					registry.players.get(entity_other).jumps = MAX_JUMPS + (registry.players.get(entity_other).equipment_type == COLLECTABLE_TYPE::WINGED_BOOTS ? 1 : 0);
				} else if (registry.motions.get(entity_other).position.x <= registry.motions.get(entity).position.x - registry.motions.get(entity).scale.x / 2 - registry.motions.get(entity_other).scale.x / 2 && 
						motionKeyStatus.test(0) && registry.players.get(entity_other).equipment_type == COLLECTABLE_TYPE::PICKAXE)
				{
					use_pickaxe(player_hero, 0, MAX_JUMPS);
				}
				else if (registry.motions.get(entity_other).position.x >= registry.motions.get(entity).position.x + registry.motions.get(entity).scale.x / 2 + registry.motions.get(entity_other).scale.x / 2 && 
						motionKeyStatus.test(1) && registry.players.get(entity_other).equipment_type == COLLECTABLE_TYPE::PICKAXE)
				{
					use_pickaxe(player_hero, 1, MAX_JUMPS);
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

void WorldSystem::motion_helper(Motion &playerMotion)
{
	std::bitset<2>& lodged = registry.gravities.get(player_hero).lodged;
	float rightFactor = motionKeyStatus.test(0) && !lodged.test(0) && !lodged.test(1) ? 1 : 0;
	float leftFactor = motionKeyStatus.test(1) && !lodged.test(0) && !lodged.test(1) ? -1 : 0;
	playerMotion.velocity[0] = BASIC_SPEED * (rightFactor + leftFactor);
	if (!pause) {
		if (playerMotion.velocity.x < 0)
			playerMotion.scale.x = -1 * abs(playerMotion.scale.x);
		else if (playerMotion.velocity.x > 0)
			playerMotion.scale.x = abs(playerMotion.scale.x);
	}
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{
	if (isTitleScreen) {
		
		return;
	}
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        change_pause();
		play_sound(SOUND_EFFECT::BUTTON_CLICK);
    }

	if (!registry.deathTimers.has(player_hero))
	{
		Motion &playerMotion = registry.motions.get(player_hero);

		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			if (registry.players.get(player_hero).equipment_type == COLLECTABLE_TYPE::DASH_BOOTS)
				check_dash_boots(player_hero, 0);
			motionKeyStatus.set(0);
		}
		else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			motionKeyStatus.reset(0);
			registry.gravities.get(player_hero).lodged.reset(0);
		}
		else if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			if (registry.players.get(player_hero).equipment_type == COLLECTABLE_TYPE::DASH_BOOTS)
				check_dash_boots(player_hero, 1);
			motionKeyStatus.set(1);
		}
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			motionKeyStatus.reset(1);
			registry.gravities.get(player_hero).lodged.reset(1);
		}

		if (!registry.gravities.get(player_hero).dashing)
			motion_helper(playerMotion);

		if (key == GLFW_KEY_W && action == GLFW_PRESS && !pause && !registry.gravities.get(player_hero).dashing)
		{
			if (registry.players.get(player_hero).jumps > 0)
			{
				playerMotion.velocity[1] = -JUMP_INITIAL_SPEED;
				registry.players.get(player_hero).jumps--;
				play_sound(SOUND_EFFECT::HERO_JUMP);
				if (registry.gravities.get(player_hero).lodged.test(0))
					disable_pickaxe(player_hero, 0, JUMP_INITIAL_SPEED / GRAVITY_ACCELERATION_FACTOR);
				else if (registry.gravities.get(player_hero).lodged.test(1))
					disable_pickaxe(player_hero, 1, JUMP_INITIAL_SPEED / GRAVITY_ACCELERATION_FACTOR);
			}
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !pause && !registry.gravities.get(player_hero).dashing)
			for (Entity weapon : registry.weapons.entities)
				do_weapon_action(renderer, weapon);
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);
        pause = false;
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
		for (Entity weapon : registry.weapons.entities)
			rotate_weapon(weapon, mouse_position);
	mouse_pos = mouse_position;
}

void WorldSystem::on_mouse_click(int key, int action, int mods){

    // button click check
    for(Entity entity : registry.buttons.entities) {
        Motion &button = registry.motions.get(entity);
        GameButton &buttonInfo = registry.buttons.get(entity);
        RenderRequest &buttonRender = registry.renderRequests.get(entity);
        if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE && buttonInfo.clicked == true) {
            buttonInfo.clicked = false;
            buttonInfo.callback();
        }
        if (abs(button.position.x - mouse_pos.x) < button.scale.x / 2 && abs(button.position.y - mouse_pos.y) < button.scale.y / 2) {
            if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS && buttonRender.visibility) {
                buttonInfo.clicked = true;
                play_sound(SOUND_EFFECT::BUTTON_CLICK);
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
