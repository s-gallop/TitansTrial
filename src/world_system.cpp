// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "physics_system.hpp"
#include "ai_system.hpp"
#include "json.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <bitset>
#include <iostream>
#include <map>
#include <fstream>

// Global Variables (?)
vec2 mouse_pos = {0,0};
bool WorldSystem::pause = false;
bool WorldSystem::debug = false;
bool WorldSystem::mouse_clicked = false;
bool WorldSystem::isTitleScreen = true;
std::bitset<2> motionKeyStatus("00");
bool pickupKeyStatus = false;
vec3 player_color;
std::vector<std::vector<char>> grid_vec = create_grid();
std::vector<Entity> player_hearts_GUI = { };
Entity powerup_GUI;
Entity difficulty_bar;
Entity indicator;
std::vector<Entity> score_GUI = { };
std::vector<Entity> following_enemies = { };
std::vector<Entity> db_decorator = { };

json::JSON state;

/* 
* ddl = Dynamic Difficulty Level
* (0 <= ddf < 100) -> (ddl = 0)
* (100 <= ddf < 200) -> (ddl = 1)
* (200 <= ddf < 300) -> (ddl = 2)
* (300 <= ddf < 400) -> (ddl = 3)
* As soon as ddf reaches 400, change to boss level (ddl = 4), const ddf = 499
* When boss is defeated, ddl = 5, ddf increases from 500 to MAX_FLOAT
* ddl still increases and change the hp of enemy (when implemented)
*/
int ddl = 0; 

/*
* ddf = Dynamic Difficulty Factor
* 1000 ms increases 1 ddf (at ddl = 0-3, 5-INF)
* Every enemy death increases 5(?) ddf
*/
float ddf = 0.0f;

// These booleans should control the dialogue when first reaching a new level
bool inBossLevel = false;

// Create the fish world
WorldSystem::WorldSystem()
	: points(0), next_enemy_spawn(0.f), next_spitter_spawn(0.f), next_ghoul_spawn(0.f)
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
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// Create the main window (for rendering, keyboard, and mouse input)
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    float new_width = mode->width;
    float new_height = mode->height;
    window = glfwCreateWindow(new_width, new_height, "Titan's Trial", nullptr, nullptr);
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
	auto window_close_redirect = [](GLFWwindow* wnd)
	{ ((WorldSystem*)glfwGetWindowUserPointer(wnd))->save_game(); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
    glfwSetMouseButtonCallback(window, cursor_click_redirect);
	glfwSetWindowCloseCallback(window, window_close_redirect);

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
	if (ddl != 4) {
		glfwSetWindowTitle(window, "Titan's Trial");
		ScreenState& screen = registry.screenStates.components[0];
		screen.screen_darken_factor = 0;
		isTitleScreen = true;

		pause = false;

		while (registry.motions.entities.size() > 0)
			registry.remove_all_components_of(registry.motions.entities.back());

		//these magic number are just the vertical position of where the buttons are
		createTitleText(renderer, { window_width_px / 2, 150 });
		createButton(renderer, { window_width_px / 2, 450 }, TEXTURE_ASSET_ID::PLAY, [&]() {load_game(); });
		createButton(renderer, { window_width_px / 2, 550 }, TEXTURE_ASSET_ID::ALMANAC, [&]() {create_almanac_screen(); });
		createButton(renderer, { window_width_px / 2, 650 }, TEXTURE_ASSET_ID::QUIT, [&]() {exit(0); });
	}
}

void WorldSystem::create_almanac_screen() {
	isTitleScreen = true;
	pause = false;

	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	Entity helper = createHelperText(renderer);
	Motion& motion = registry.motions.get(helper);
	motion.position = {window_width_px / 2, 150};
	registry.renderRequests.get(helper).visibility = true;

	createSword(renderer, {window_width_px / 6, 300});
	createToolTip(renderer, {window_width_px * 2 / 3, 300}, TEXTURE_ASSET_ID::SWORD_HELPER);
	createGun(renderer, {window_width_px / 6, 350});
	createToolTip(renderer, {window_width_px * 2 / 3, 350}, TEXTURE_ASSET_ID::GUN_HELPER);
	createGrenadeLauncher(renderer, {window_width_px / 6, 400});
	createToolTip(renderer, {window_width_px * 2 / 3, 400}, TEXTURE_ASSET_ID::GRENADE_HELPER);
	createRocketLauncher(renderer, {window_width_px / 6, 450});
	createToolTip(renderer, {window_width_px * 2 / 3, 450}, TEXTURE_ASSET_ID::ROCKET_HELPER);
	createWingedBoots(renderer, {window_width_px / 6, 500});
	createToolTip(renderer, {window_width_px * 2 / 3, 500}, TEXTURE_ASSET_ID::WINGED_BOOTS_HELPER);
	createPickaxe(renderer, {window_width_px / 6, 550});
	createToolTip(renderer, {window_width_px * 2 / 3, 550}, TEXTURE_ASSET_ID::PICKAXE_HELPER);
	createDashBoots(renderer, {window_width_px / 6, 600});
	createToolTip(renderer, {window_width_px * 2 / 3, 600}, TEXTURE_ASSET_ID::DASH_BOOTS_HELPER);

	createButton(renderer, { window_width_px / 2, 700 }, TEXTURE_ASSET_ID::BACK, [&]() {create_title_screen();});
}


// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	for (AnimationInfo& animation: registry.animated.components) {
		if (animation.oneTimeState != -1) {
			animation.oneTimer += elapsed_ms_since_last_update / 1000.f;
		}
	}
	
	// internal data update section
	float expectedTimer = registry.players.get(player_hero).invulnerable_timer - elapsed_ms_since_last_update;
	if (expectedTimer <= 0.0f)
	{
		expectedTimer = 0.0f;
		registry.colors.get(player_hero) = player_color;
		registry.players.get(player_hero).invuln_type = INVULN_TYPE::NONE;
	}
	registry.players.get(player_hero).invulnerable_timer = expectedTimer;

	if (registry.players.get(player_hero).invuln_type == INVULN_TYPE::HIT) 
	{
		for (Entity e : player_hearts_GUI) {
			registry.renderRequests.get(e).used_texture = TEXTURE_ASSET_ID::PLAYER_HEART_STEEL;
		}
	}
	else if (registry.players.get(player_hero).invuln_type == INVULN_TYPE::HEAL)
	{
		for (Entity e : player_hearts_GUI) {
			registry.renderRequests.get(e).used_texture = TEXTURE_ASSET_ID::PLAYER_HEART_HEAL;
		}
	}
	else
	{
		for (Entity e : player_hearts_GUI) {
			registry.renderRequests.get(e).used_texture = TEXTURE_ASSET_ID::PLAYER_HEART;
		}
	}

	switch (registry.players.get(player_hero).equipment_type)
	{
		case COLLECTABLE_TYPE::PICKAXE:
			registry.renderRequests.get(powerup_GUI).used_texture = TEXTURE_ASSET_ID::PICKAXE;
			registry.renderRequests.get(powerup_GUI).visibility = true;
			break;
		case COLLECTABLE_TYPE::WINGED_BOOTS:
			registry.renderRequests.get(powerup_GUI).used_texture = TEXTURE_ASSET_ID::WINGED_BOOTS;
			registry.renderRequests.get(powerup_GUI).visibility = true;
			break;
		case COLLECTABLE_TYPE::DASH_BOOTS:
			registry.renderRequests.get(powerup_GUI).used_texture = TEXTURE_ASSET_ID::DASH_BOOTS;
			registry.renderRequests.get(powerup_GUI).visibility = true;
			break;
		default:
			registry.renderRequests.get(powerup_GUI).used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
			registry.renderRequests.get(powerup_GUI).visibility = false;
	}

	changeScore(points);

	ddf = max(ddf, 0.f);
	if (ddf < 100)
		ddl = 0;
	else if (ddf >= 100 && ddf < 200)
		ddl = 1;
	else if (ddf >= 200 && ddf < 300)
		ddl = 2;
	else if (ddf >= 300 && ddf < 400)
		ddl = 3;
	else if (ddf >= 400 && !inBossLevel)
	{
		ddl = 4;   // Change to boss level
		inBossLevel = true;
		registry.remove_all_components_of(indicator);
		registry.renderRequests.get(difficulty_bar).used_texture = TEXTURE_ASSET_ID::DIFFICULTY_BAR_BOSS;
		db_decorator.push_back(createDBFlame(renderer, DB_FLAME_CORD));
		db_decorator.push_back(createDBSkull(renderer, DIFF_BAR_CORD));
		current_enemy_spawning_speed = 0.f;
		current_spitter_spawning_speed = 0.f;
		current_ghoul_spawning_speed = 0.f;
		clear_enemies();
	}

	if (ddl == 4)
		ddf = 499.0;
	else
		ddf += elapsed_ms_since_last_update / 1000.f;
	
	if (ddl < 4)
		registry.motions.get(indicator).position[0] = 30.f + ddf * INDICATOR_VECLOCITY;

	// Updating window title
	
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	title_ss << "; Dynamic Difficulty Level: " << ddl;
	title_ss << "; Dynamic Difficulty Factor: " << ddf;
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

		if (motion.position.x + abs(motion.scale.x) < 0.f || motion.position.x - abs(motion.scale.x) > window_width_px || motion.position.y - abs(motion.scale.y) > window_height_px || motion.position.y + abs(motion.scale.y) < 0.f)
		{
			Entity e = motion_container.entities[i];
			if (registry.parallaxBackgrounds.has(e)) {
				ParallaxBackground &bg = registry.parallaxBackgrounds.get(e);
				motion.position = bg.resetPosition;
			}
		}
		
		if (motion.position.y < -250 && (registry.bullets.has(motion_container.entities[i]) || registry.rockets.has(motion_container.entities[i])))
			registry.remove_all_components_of(motion_container.entities[i]);
		else if (registry.lasers.has(motion_container.entities[i]) && (motion.position.x > window_width_px + window_width_px / 2.f || motion.position.x < -window_width_px / 2.f || motion.position.y > window_height_px + window_height_px / 2.f || motion.position.y < -window_height_px / 2.f))
			registry.remove_all_components_of(motion_container.entities[i]);
	}

	if (registry.players.get(player_hero).hasWeapon)
		update_weapon(renderer, elapsed_ms_since_last_update * current_speed, player_hero, mouse_clicked);

	update_equipment(elapsed_ms_since_last_update * current_speed, player_hero);
	update_dash_boots(elapsed_ms_since_last_update * current_speed, player_hero, motionKeyStatus, BASIC_SPEED);
	update_pickaxe(elapsed_ms_since_last_update * current_speed);

	update_grenades(renderer, elapsed_ms_since_last_update * current_speed);
	update_explosions(elapsed_ms_since_last_update * current_speed);

	// Animation Stuff
	vec2 playerVelocity = registry.motions.get(player_hero).velocity;
	AnimationInfo &playerAnimation = registry.animated.get(player_hero);
	if (playerVelocity.y > 0)
	{
		playerAnimation.curState = 4;
	}
	else if (playerVelocity.y < 0)
	{
		playerAnimation.curState = 3;
	}
	else if (playerVelocity.x != 0)
	{
		playerAnimation.curState = 2;
	}
	else
	{
		playerAnimation.curState = 0;
	}

	if (ddl == 0)
	{
		current_enemy_spawning_speed = 1.0f;
		current_spitter_spawning_speed = 0.0f;
		current_ghoul_spawning_speed = 0.0f;
	}
	else if (ddl == 1)
	{
		current_enemy_spawning_speed = 1.0f;
		current_ghoul_spawning_speed = 0.8f;
		current_spitter_spawning_speed = 1.0f;
	}
	else if (ddl == 2)
	{
		current_enemy_spawning_speed = 1.0f;
		current_ghoul_spawning_speed = 1.0f;
		current_spitter_spawning_speed = 1.5f;
	}
	else if (ddl == 3)
	{
		current_enemy_spawning_speed = 1.2f;
		current_spitter_spawning_speed = 1.5f;
		current_ghoul_spawning_speed = 1.0f;
	}

	for (int i = 0; i < registry.players.get(player_hero).hp; i++) {
		Entity curHeart = player_hearts_GUI[i];
		registry.renderRequests.get(curHeart).visibility = true;
	}
	for (int i = registry.players.get(player_hero).hp; i < player_hearts_GUI.size(); i++) {
		Entity curHeart = player_hearts_GUI[i];
		registry.renderRequests.get(curHeart).visibility = false;
	}

	spawn_move_normal_enemies(elapsed_ms_since_last_update);
	spawn_move_ghouls(elapsed_ms_since_last_update);
	spawn_spitter_enemy(elapsed_ms_since_last_update);
	update_collectable_timer(elapsed_ms_since_last_update * current_speed, renderer, ddl);
	update_graphics_all_enemies();

	if (ddl == 2 && following_enemies.empty())
	{
		Entity newEnemy = createFollowingEnemy(renderer, find_index_from_map(vec2(12, 8)));
		std::vector<std::vector<char>> vec = grid_vec;
		bfs_follow_start(vec, registry.motions.get(newEnemy).position, registry.motions.get(player_hero).position, newEnemy);
		following_enemies.push_back(newEnemy);
	}
	else if (ddl == 2 && !following_enemies.empty())
	{
		spawn_move_following_enemies(elapsed_ms_since_last_update);
	}
	else if (ddl != 2 && !following_enemies.empty())
	{
		registry.remove_all_components_of(following_enemies[0]);
		following_enemies.clear();
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

void WorldSystem::changeScore(int score)
{
	if (score >= 99999) 
	{
		for (Entity n : score_GUI)
			registry.renderRequests.get(n).used_texture = TEXTURE_ASSET_ID::NINE;
	}
	else
	{
		int mask = 10000;
		for (Entity n : score_GUI)
		{
			int digit = score / mask;
			registry.renderRequests.get(n).used_texture = connectNumber(digit);
			if (digit != 0) {
				score -= digit * mask;
			}
			mask /= 10;
		}
	}
}

TEXTURE_ASSET_ID WorldSystem::connectNumber(int digit)
{
	switch (digit)
	{
	case 0:
		return TEXTURE_ASSET_ID::ZERO;
	case 1:
		return TEXTURE_ASSET_ID::ONE;
	case 2:
		return TEXTURE_ASSET_ID::TWO;
	case 3:
		return TEXTURE_ASSET_ID::THREE;
	case 4:
		return TEXTURE_ASSET_ID::FOUR;
	case 5:
		return TEXTURE_ASSET_ID::FIVE;
	case 6:
		return TEXTURE_ASSET_ID::SIX;
	case 7:
		return TEXTURE_ASSET_ID::SEVEN;
	case 8:
		return TEXTURE_ASSET_ID::EIGHT;
	default:
		return TEXTURE_ASSET_ID::NINE;
	}
}

//update elements of all enemies
void WorldSystem::update_graphics_all_enemies()
{
	for (Entity entity: registry.enemies.entities)
	{ 
		Enemies& enemy = registry.enemies.get(entity);
		AnimationInfo& animation = registry.animated.get(entity);

		if (animation.oneTimeState == enemy.death_animation && (int)floor(animation.oneTimer * ANIMATION_SPEED_FACTOR) == animation.stateFrameLength[enemy.death_animation]) {
			registry.remove_all_components_of(entity);
		} else if (animation.oneTimeState == enemy.hit_animation && (int)floor(animation.oneTimer * ANIMATION_SPEED_FACTOR) == animation.stateFrameLength[enemy.hit_animation]) {
			enemy.hittable = true;
			enemy.hitting = true;
			if (registry.motions.get(entity).velocity.x != 0)
				registry.motions.get(entity).dir = registry.motions.get(entity).velocity.x > 0 ? 1 : -1;
		}
	}
}

// deal with normal eneimies' spawning and moving
void WorldSystem::spawn_move_normal_enemies(float elapsed_ms_since_last_update)
{
	next_enemy_spawn -= elapsed_ms_since_last_update * current_enemy_spawning_speed;
	if (registry.enemies.components.size() < MAX_FIRE_ENEMIES && next_enemy_spawn < 0.f)
	{
		// Reset timer
		next_enemy_spawn = (ENEMY_DELAY_MS / 2) + uniform_dist(rng) * (ENEMY_DELAY_MS / 2);
		srand(time(0));
		float squareFactor = rand() % 2 == 0 ? 0.0005 : -0.0005;
		int leftHeight = ENEMY_SPAWN_HEIGHT_IDLE_RANGE + rand() % (window_height_px - ENEMY_SPAWN_HEIGHT_IDLE_RANGE * 2);
		int rightHeight = ENEMY_SPAWN_HEIGHT_IDLE_RANGE + rand() % (window_height_px - ENEMY_SPAWN_HEIGHT_IDLE_RANGE * 2);
		float curveParameter = (float)(rightHeight - leftHeight - window_width_px * window_width_px * squareFactor) / window_width_px;
		Entity newEnemy = createFireEnemy(renderer, vec2(window_width_px, rightHeight));
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
			motion.dir = 1;
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
			motion.dir = -1;
		}
		float gradient = 2 * testAI.a * motion.position[0] + testAI.b;
		float basicFactor = sqrt(BASIC_SPEED * BASIC_SPEED / (gradient * gradient + 1));
		float direction = testAI.departFromRight ? -1.0 : 1.0;
		motion.velocity = direction * vec2(basicFactor, gradient * basicFactor);
	}
}

void WorldSystem::spawn_move_ghouls(float elapsed_ms_since_last_update)
{
	float GHOUL_SPEED = 100.f;
	float EDGE_DISTANCE = 0.f;
	int BLANK_STATE = 2;

	next_ghoul_spawn -= elapsed_ms_since_last_update * current_ghoul_spawning_speed;
	if (registry.ghouls.components.size() < MAX_GHOULS && next_ghoul_spawn < 0.f)
	{
		// Reset timer
		next_ghoul_spawn = (ENEMY_DELAY_MS / 2) + uniform_dist(rng) * (ENEMY_DELAY_MS / 2);
		float x_pos = uniform_dist(rng) * (window_width_px - 120) + 60;
		float y_pos = uniform_dist(rng) * (window_height_px - 350) + 50;
		Entity newGhoul = createGhoul(renderer, vec2(x_pos, y_pos));
		//printf("Curr state %d\n", registry.animated.get(newEnemy).oneTimeState);
	}
	

	Motion& hero_motion = registry.motions.get(player_hero);
	for (uint i = 0; i < registry.ghouls.entities.size(); i++) {
		Entity enemy = registry.ghouls.entities[i];
		Motion& enemy_motion = registry.motions.get(enemy);
		AnimationInfo& animation = registry.animated.get(enemy);
		Ghoul& enemy_reg = registry.ghouls.get(enemy);
		//printf("Position: %f\n", enemy_motion.position.x);
		if (enemy_motion.velocity.y != 0.f) {
			animation.oneTimeState = BLANK_STATE;
		} 
		else if (enemy_reg.left_x != -1.f && enemy_motion.velocity.x == 0.f && enemy_motion.velocity.y == 0.f && animation.oneTimeState == -1) {
			float direction = max(enemy_motion.position.x - enemy_reg.left_x, enemy_reg.right_x - enemy_motion.position.x);
			direction = direction / abs(direction);
			enemy_motion.velocity.x = direction * GHOUL_SPEED * current_speed;
			enemy_motion.dir = (int)direction;
		}
		// Reverse direction
		else if (enemy_motion.position.x - enemy_reg.left_x <= EDGE_DISTANCE && enemy_motion.velocity.y == 0.f && enemy_motion.velocity.x != 0.f) {
			enemy_motion.velocity.x = GHOUL_SPEED * current_speed;
			enemy_motion.dir = 1;
		}
		else if (enemy_reg.right_x - enemy_motion.position.x <= EDGE_DISTANCE && enemy_motion.velocity.y == 0.f && enemy_motion.velocity.x != 0.f) {
			enemy_motion.velocity.x = -1.f * GHOUL_SPEED * current_speed;
			enemy_motion.dir = -1;
		}
	}
}

// deal with normal eneimies' spawning and moving
void WorldSystem::spawn_move_following_enemies(float elapsed_ms_since_last_update)
{
	const uint PHASE_IN_STATE = 1;
	const uint PHASE_OUT_STATE = 4;

	/*
	next_enemy_spawn -= elapsed_ms_since_last_update * current_enemy_spawning_speed;
	if (registry.followingEnemies.components.size() < MAX_FOLLOWING_ENEMIES && next_enemy_spawn < 0.f)
	{
		// Reset timer
		next_enemy_spawn = (ENEMY_DELAY_MS / 2) + uniform_dist(rng) * (ENEMY_DELAY_MS / 2);
		Entity newEnemy = createFollowingEnemy(renderer, find_index_from_map(vec2(12, 8)));

		std::vector<std::vector<char>> vec = grid_vec;
		bfs_follow_start(vec, registry.motions.get(newEnemy).position, registry.motions.get(player_hero).position, newEnemy);
	}
	*/

	Motion& hero_motion = registry.motions.get(player_hero);
	for (uint i = 0; i < registry.followingEnemies.entities.size(); i++) {
		Entity enemy = registry.followingEnemies.entities[i];
		Motion& enemy_motion = registry.motions.get(enemy);
		AnimationInfo& animation = registry.animated.get(enemy);
		FollowingEnemies& enemy_reg = registry.followingEnemies.get(enemy);
		Enemies enemies = registry.enemies.get(enemy);

		enemy_reg.next_blink_time -= elapsed_ms_since_last_update * current_speed;
		if (enemy_reg.next_blink_time < 0.f && enemy_reg.blinked == false)
		{
			//Time between blinks
			enemy_reg.next_blink_time = 700.f;

			//enemies.hittable = true;
			//enemy_reg.hittable = true;

			if (enemy_reg.path.size() == 0 && find_map_index(enemy_motion.position) != find_map_index(hero_motion.position)) {
				std::vector<std::vector<char>> vec = grid_vec;
				bfs_follow_start(vec, enemy_motion.position, hero_motion.position, enemy);
			}

			//Don't blink when not moving: next pos in path is same pos as current
			if (enemy_reg.path.size() != 0 && find_index_from_map(enemy_reg.path.back()) == enemy_motion.position) {
				enemy_reg.path.pop_back();
			}
			 else if (enemy_reg.path.size() != 0)
			{
				animation.oneTimeState = PHASE_IN_STATE;
				vec2 converted_pos = find_index_from_map(enemy_reg.path.back());
				enemy_motion.dir = (converted_pos.x > enemy_motion.position.x) ? -1 : 1;
				enemy_motion.position = converted_pos;
				enemy_reg.path.pop_back();

				//Don't blink when not moving: next loop will be to re-calc the path
				if (enemy_reg.path.size() != 0) {
					enemy_reg.blinked = true;
				}
			}
		}

		if (enemy_reg.next_blink_time < 0.0f && enemy_reg.blinked == true) {
			enemy_reg.next_blink_time = 100.f;
			animation.oneTimeState = PHASE_OUT_STATE;
			//enemy_reg.hittable = false;
			//enemies.hittable = false;
			enemy_reg.blinked = false;
		}
	}
}

void WorldSystem::spawn_spitter_enemy(float elapsed_ms_since_last_update) {
    const uint SHOOT_STATE = 2;
    const uint SPITTER_FIRE_FRAME = 4;
	const uint WALKING_TIME = 5;
	const float WALKING_SPEED = 1.f;

	next_spitter_spawn -= elapsed_ms_since_last_update * current_spitter_spawning_speed;
	if (registry.spitterEnemies.components.size() < MAX_SPITTERS && next_spitter_spawn < 0.f)
	{
		next_spitter_spawn = (SPITTER_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (SPITTER_SPAWN_DELAY_MS / 2);
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
		AnimationInfo &animation = registry.animated.get(entity);

		//if (!spitterEnemy.canShoot && spitterEnemy.timeUntilNextShotMs >= 2000.f)
		//{
		//	
		//	if (spitterEnemy.left_x == -1.f && spitterEnemy.right_x == -1.f && motion.velocity.y == 0.f) {
		//		vec2 edges = find_edges(motion.position, motion.scale.x);
		//		spitterEnemy.left_x = edges.x;
		//		spitterEnemy.right_x = edges.y;
		//	}

		//	if (motion.velocity.x == 0.f && motion.velocity.y == 0.f && animation.oneTimeState == -1) {
		//		float direction = max(motion.position.x - spitterEnemy.left_x, spitterEnemy.right_x - motion.position.x);
		//		direction = direction / abs(direction);
		//		motion.velocity.x = direction * WALKING_SPEED * current_speed;
		//		motion.dir = (int)direction;
		//	}
		//	// Reverse direction
		//	else if (motion.position.x - spitterEnemy.left_x <= 0 && motion.velocity.y == 0.f && motion.velocity.x != 0.f) {
		//		motion.velocity.x = WALKING_SPEED * current_speed;
		//		motion.dir = 1;
		//	}
		//	else if (spitterEnemy.right_x - motion.position.x <= 0 && motion.velocity.y == 0.f && motion.velocity.x != 0.f) {
		//		motion.velocity.x = -1.f * WALKING_SPEED * current_speed;
		//		motion.dir = -1;
		//	}
		//}

		//if (!spitterEnemy.canShoot && spitterEnemy.timeUntilNextShotMs < 2000.f) {
		//	motion.velocity.x = 0.f;
		//	animation.curState = 0;
		//}

        if ((int)floor(animation.oneTimer * ANIMATION_SPEED_FACTOR) == SPITTER_FIRE_FRAME && spitterEnemy.canShoot) {
            Entity spitterBullet = createSpitterEnemyBullet(renderer, motion.position, motion.angle);
            float absolute_scale_x = abs(registry.motions.get(entity).scale[0]);
            if (registry.motions.get(spitterBullet).velocity[0] < 0.0f)
                registry.motions.get(entity).scale[0] = -absolute_scale_x;
            else
                registry.motions.get(entity).scale[0] = absolute_scale_x;
            spitterEnemy.canShoot = false;
        }
		if (spitterEnemy.bulletsRemaining > 0 && spitterEnemy.timeUntilNextShotMs <= 0.f && registry.enemies.get(entity).hitting == true)
		{
			// attack animation
            animation.oneTimeState = SHOOT_STATE;
            spitterEnemy.canShoot = true;
			// create bullet at same position as enemy

			spitterEnemy.bulletsRemaining--;
			spitterEnemy.timeUntilNextShotMs = SPITTER_PROJECTILE_DELAY_MS;
		}
	}

	// decay spitter bullets
	auto& spitterBullets_container = registry.spitterBullets;
	for (uint i = 0; i < spitterBullets_container.size(); i++)
	{
		SpitterBullet& spitterBullet = spitterBullets_container.components[i];
		Entity entity = spitterBullets_container.entities[i];
        RenderRequest& render = registry.renderRequests.get(entity);
		Motion& motion = registry.motions.get(entity);
		// make bullets smaller over time
		motion.scale = vec2(motion.scale.x / spitterBullet.mass, motion.scale.y / spitterBullet.mass);
		spitterBullet.mass -= elapsed_ms_since_last_update / SPITTER_PROJECTILE_REDUCTION_FACTOR;
		motion.scale = vec2(motion.scale.x * spitterBullet.mass, motion.scale.y * spitterBullet.mass);
        render.scale = motion.scale;
		if (spitterBullet.mass <= SPITTER_PROJECTILE_MIN_SIZE)
		{
			spitterBullet.mass = 0;
			registry.remove_all_components_of(entity);
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
	next_enemy_spawn = 0.f;
	next_ghoul_spawn = 0.f;
	next_spitter_spawn = 0.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());
	// Debugging for memory/component leaks
	registry.list_all_components();
	// add bg
	
	create_parallax_background();
	initiate_weapons();

	// Create a new hero
	player_hero = createHero(renderer, { 100, 200 });
	registry.colors.insert(player_hero, { 1, 0.8f, 0.8f });

	Player& player = registry.players.get(player_hero);
	createSword(renderer, registry.motions.get(player_hero).position);

	int background_pixels_width = 768;
	int background_pixels_height = 432;

	float base_height = 16.0 * window_height_px / background_pixels_height;
	float base_width = 16.0 * window_width_px / background_pixels_width;

	// global variables at this .cpp to reset, don't forget it!
	motionKeyStatus.reset();
	ddl = 0;
	ddf = 0.0f;
	inBossLevel = false;
	player_color = registry.colors.get(player_hero);
	player_hearts_GUI.clear();
	score_GUI.clear();
	db_decorator.clear();

	create_inGame_GUIs();

	// bottom line
	createBlock(renderer, {window_width_px / 2, window_height_px + 100}, {window_width_px, base_height / 2}, grid_vec);
	// left line
	createBlock(renderer, {-base_width, 0}, {base_width * 6, window_height_px * 2}, grid_vec);
	// right line
	createBlock(renderer, {window_width_px + base_width, 0}, {base_width * 6, window_height_px * 2}, grid_vec);
	// top line
	// createBlock(renderer, {window_width_px / 2, -100.f}, {window_width_px, base_height / 2}, grid_vec);

	// left middle platform
	createBlock(renderer, {base_width * 7.5, base_height * 12}, {base_width * 11, base_height * 2}, grid_vec);

	// top middle platform
	createBlock(renderer, {window_width_px / 2, base_height * 6}, {base_width * 26, base_height * 2}, grid_vec);

	// right middle platform
	createBlock(renderer, {window_width_px - base_width * 7.5, base_height * 12}, {base_width * 11, base_height * 2}, grid_vec);

	// bottom middle left platform
	createBlock(renderer, {base_width * 13, base_height * 18}, {base_width * 10, base_height * 2}, grid_vec);

	// bottom middle right platform
	createBlock(renderer, {window_width_px - base_width * 13, base_height * 18}, {base_width * 10, base_height * 2}, grid_vec);

	// bottom left padding platform
	createBlock(renderer, {base_width * 6.5, window_height_px - base_height * 3}, {base_width * 9, base_height * 4}, grid_vec);

	// bottom right padding platform
	createBlock(renderer, {window_width_px - base_width * 6.5, window_height_px - base_height * 3}, {base_width * 9, base_height * 4}, grid_vec);

	// bottom center padding platform
	createBlock(renderer, {window_width_px / 2, window_height_px - base_height * 2}, {base_width * 14, base_height * 2}, grid_vec);
	
	// Adds whatever's needed in the pause screen
	create_pause_screen();

	//testing screen dimensions
	/*for (int i = 10; i < window_width_px; i += ENEMY_BB_WIDTH) {
		for (int j = -25; j < window_height_px; j += ENEMY_BB_HEIGHT) {
			createEnemy(renderer, vec2(i, j), 0.0, vec2(0.0, 0.0), vec2(ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT));
		}
	}*/
}

void WorldSystem::save_game() {
	if (!registry.deathTimers.has(player_hero)) 
	{
		state =
		{
			"ddl", ddl,
			"score", points,
			"hp", registry.players.get(player_hero).hp,
			"player_x", registry.motions.get(player_hero).position.x,
			"player_y", registry.motions.get(player_hero).position.y,
			"weapon", save_weapon(registry.players.get(player_hero).weapon),
			"fire_enemy", json::Array(),
			"spitter", json::Array(),
			"spitter_bullet", json::Array(),
			"ghoul", json::Array(),
		};

		for (Entity fire_enemy : registry.fireEnemies.entities)
		{
			json::JSON sfes = json::Object();   // single_fire_enemy_save
			sfes =
			{
				"hp", registry.enemies.get(fire_enemy).health,
				"x_pos", registry.motions.get(fire_enemy).position.x,
				"y_pos", registry.motions.get(fire_enemy).position.y,
				"a", registry.testAIs.get(fire_enemy).a,
				"b", registry.testAIs.get(fire_enemy).b,
				"c", registry.testAIs.get(fire_enemy).c,
				"from_right", registry.testAIs.get(fire_enemy).departFromRight,
			};
			state["fire_enemy"].append(sfes);
		}

		for (Entity spitter : registry.spitterEnemies.entities)
		{
			json::JSON sss = json::Object();   // single_spitter_save
			sss =
			{
				"hp", registry.enemies.get(spitter).health,
				"x_pos", registry.motions.get(spitter).position.x,
				"y_pos", registry.motions.get(spitter).position.y,
				"bullets", registry.spitterEnemies.get(spitter).bulletsRemaining,
				"timer", registry.spitterEnemies.get(spitter).timeUntilNextShotMs,
				"shootable", registry.spitterEnemies.get(spitter).canShoot,
				"right_x", registry.spitterEnemies.get(spitter).right_x,
				"left_x", registry.spitterEnemies.get(spitter).left_x,
			};
			state["spitter"].append(sss);
		}

		for (Entity spitter_bullet : registry.spitterBullets.entities)
		{
			json::JSON ssbs = json::Object();   // single_spitter_bullet_save
			ssbs =
			{
				"x_pos", registry.motions.get(spitter_bullet).position.x,
				"y_pos", registry.motions.get(spitter_bullet).position.y,
				"x_v", registry.motions.get(spitter_bullet).velocity.x,
				"y_v", registry.motions.get(spitter_bullet).velocity.y,
				"scale_x", registry.motions.get(spitter_bullet).scale.x,
				"scale_y", registry.motions.get(spitter_bullet).scale.y,
				"angle", registry.motions.get(spitter_bullet).angle,
				"mass", registry.spitterBullets.get(spitter_bullet).mass,
			};
			state["spitter_bullet"].append(ssbs);
		}

		for (Entity ghoul : registry.ghouls.entities)
		{
			json::JSON sgs = json::Object();   // single_ghoul_save
			sgs =
			{
				"hp", registry.enemies.get(ghoul).health,
				"x_pos", registry.motions.get(ghoul).position.x,
				"y_pos", registry.motions.get(ghoul).position.y,
				"x_v", registry.motions.get(ghoul).velocity.x,
				"y_v", registry.motions.get(ghoul).velocity.y,
				"dir", registry.motions.get(ghoul).dir,
			};
			state["ghoul"].append(sgs);
		}

		std::ofstream out("game_save.json");
		out << state;
		out.close();
	}

	create_title_screen();
}

int WorldSystem::save_weapon(Entity weapon) {
	if (registry.swords.has(weapon))
		return 0;
	else if (registry.guns.has(weapon))
		return 1;
	else if (registry.rocketLaunchers.has(weapon))
		return 2;
	else if (registry.grenadeLaunchers.has(weapon))
		return 3;
	else if (registry.laserRifles.has(weapon))
		return 4;
	else
		return -1;
}

void WorldSystem::load_game() {
	restart_game();
	std::ifstream in("game_save.json");
	std::stringstream buffer;
	buffer << in.rdbuf();
	std::string jsonString = buffer.str();
	state = json::JSON::Load(jsonString);

	ddf = state["ddl"].ToInt() * 100;
	points = state["score"].ToInt();
	Player &player = registry.players.get(player_hero);
	player.hp = state["hp"].ToInt();
	int weapon = state["weapon"].ToInt();
	registry.motions.get(player_hero).position = { state["player_x"].ToFloat(), state["player_y"].ToFloat() };
	if (weapon == 0)
		collect(createSword(renderer, { 0.f, 0.f }), player_hero);
	else if (weapon == 1)
		collect(createGun(renderer, { 0.f, 0.f }), player_hero);
	else if (weapon == 2)
		collect(createRocketLauncher(renderer, { 0.f, 0.f }), player_hero);
	else if (weapon == 3)
		collect(createGrenadeLauncher(renderer, { 0.f, 0.f }), player_hero);
	else if (weapon == 4)
		collect(createLaserRifle(renderer, { 0.f, 0.f }), player_hero);

	for (int i = 0; i < state["fire_enemy"].size(); i++) 
	{
		json::JSON sfe = state["fire_enemy"][i];
		if (!sfe["hp"].ToInt() <= 0) 
		{
			Entity nfe = createFireEnemy(renderer, { sfe["x_pos"].ToFloat(), sfe["y_pos"].ToFloat() });
			registry.enemies.get(nfe).health = sfe["hp"].ToInt();
			TestAI &nfe_ai = registry.testAIs.get(nfe);
			nfe_ai.a = sfe["a"].ToFloat();
			nfe_ai.b = sfe["b"].ToFloat();
			nfe_ai.c = sfe["c"].ToFloat();
			nfe_ai.departFromRight = sfe["from_right"].ToBool();
		}
	}

	for (int i = 0; i < state["spitter"].size(); i++) 
	{
		json::JSON ss = state["spitter"][i];
		if (!ss["hp"].ToInt() <= 0) 
		{
			Entity ns = createSpitterEnemy(renderer, { ss["x_pos"].ToFloat(), ss["y_pos"].ToFloat() });
			registry.enemies.get(ns).health = ss["hp"].ToInt();
			SpitterEnemy &ns_info = registry.spitterEnemies.get(ns);
			ns_info.bulletsRemaining = ss["bullets"].ToInt();
			ns_info.canShoot = ss["shootable"].ToBool();
			ns_info.timeUntilNextShotMs = ss["timer"].ToFloat();
			ns_info.left_x = ss["left_x"].ToFloat();
			ns_info.right_x = ss["right_x"].ToFloat();
		}
	}

	for (int i = 0; i < state["spitter_bullet"].size(); i++)
	{
		json::JSON ssb = state["spitter_bullet"][i];
		Entity nsb = createSpitterEnemyBullet(renderer, { ssb["x_pos"].ToFloat(), ssb["y_pos"].ToFloat() }, ssb["angle"].ToFloat());
		registry.spitterBullets.get(nsb).mass = ssb["mass"].ToFloat();
		Motion& nsb_mo = registry.motions.get(nsb);
		nsb_mo.scale = { ssb["scale_x"].ToFloat(), ssb["scale_y"].ToFloat() };
		nsb_mo.velocity = { ssb["x_v"].ToFloat(), ssb["y_v"].ToFloat() };
	}

	for (int i = 0; i < state["ghoul"].size(); i++)
	{
		json::JSON sg = state["ghoul"][i];
		if (!sg["hp"].ToInt() <= 0) 
		{
			Entity ng = createGhoul(renderer, { sg["x_pos"].ToFloat(), sg["y_pos"].ToFloat() });
			registry.enemies.get(ng).health = sg["hp"].ToInt();
			Motion& ng_mo = registry.motions.get(ng);
			ng_mo.velocity = { sg["x_v"].ToFloat(), sg["y_v"].ToFloat() };
			ng_mo.dir = { sg["dir"].ToInt() };
		}
	}
	
	registry.players.get(player_hero).invuln_type = INVULN_TYPE::HEAL;
	registry.players.get(player_hero).invulnerable_timer = 3000.f;
}

void WorldSystem::create_pause_screen() {
    createButton(renderer, {18, 18}, TEXTURE_ASSET_ID::MENU, [&](){change_pause();});
    createButton(renderer, {window_width_px / 2, window_height_px / 2}, TEXTURE_ASSET_ID::BACK, [&]() {save_game(); }, false);
    createHelperText(renderer);
}

void WorldSystem::create_parallax_background() {
	parallax_moon = createParallaxItem(renderer, {580, 400}, TEXTURE_ASSET_ID::PARALLAX_MOON);
	parallax_clouds_far_1 = createParallaxItem(renderer, {600, 400}, TEXTURE_ASSET_ID::PARALLAX_CLOUDS_FAR);
	parallax_clouds_far_2 = createParallaxItem(renderer, {-600, 400}, TEXTURE_ASSET_ID::PARALLAX_CLOUDS_FAR);
	parallax_clouds_close_1 = createParallaxItem(renderer, {600, 400}, TEXTURE_ASSET_ID::PARALLAX_CLOUDS_CLOSE);
	parallax_clouds_close_2 = createParallaxItem(renderer, {-600, 400}, TEXTURE_ASSET_ID::PARALLAX_CLOUDS_CLOSE);
	parallax_rain_1 = createParallaxItem(renderer, {0, 1200}, TEXTURE_ASSET_ID::PARALLAX_RAIN);
	parallax_rain_2 = createParallaxItem(renderer, {600, 800}, TEXTURE_ASSET_ID::PARALLAX_RAIN);
	parallax_rain_3 = createParallaxItem(renderer, {400, 400}, TEXTURE_ASSET_ID::PARALLAX_RAIN);
	parallax_rain_4 = createParallaxItem(renderer, {800, 100}, TEXTURE_ASSET_ID::PARALLAX_RAIN);
	parallax_background = createParallaxItem(renderer, {600, 400}, TEXTURE_ASSET_ID::BACKGROUND);
	parallax_lava_1 = createParallaxItem(renderer, {600, 813}, TEXTURE_ASSET_ID::PARALLAX_LAVA);
	parallax_lava_2 = createParallaxItem(renderer, {-600, 813}, TEXTURE_ASSET_ID::PARALLAX_LAVA);
	parallax_lava_3 = createParallaxItem(renderer, {-1200, 813}, TEXTURE_ASSET_ID::PARALLAX_LAVA);
}

void WorldSystem::create_inGame_GUIs() {
	float heartPosition = HEART_START_POS;
	for (int i = 0; i < registry.players.get(player_hero).hp_max; i++) {
		player_hearts_GUI.push_back(createPlayerHeart(renderer, { heartPosition, HEART_Y_CORD }));
		heartPosition += HEART_GAP;
	}
	powerup_GUI = createPowerUpIcon(renderer, POWER_CORD);
	difficulty_bar = createDifficultyBar(renderer, DIFF_BAR_CORD);
	indicator = createDifficultyIndicator(renderer, INDICATOR_START_CORD);
	createScore(renderer, SCORE_CORD);
	float numberPosition = NUMBER_START_POS;
	for (int i = 0; i < 5; i++) {
		score_GUI.push_back(createNumber(renderer, { numberPosition, NUMBER_Y_CORD }));
		numberPosition += NUMBER_GAP;
	}
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
			if (((registry.enemies.has(entity_other) && registry.enemies.get(entity_other).hitting) || (registry.explosions.has(entity_other) && registry.weaponHitBoxes.get(entity_other).isActive)) && registry.players.get(player_hero).invulnerable_timer <= 0.0f && !registry.gravities.get(player_hero).dashing)
			{
				// remove 1 hp
				player.hp -= 1;
				registry.players.get(player_hero).invulnerable_timer = max(3000.f, registry.players.get(player_hero).invulnerable_timer);
				registry.players.get(player_hero).invuln_type = INVULN_TYPE::HIT;
				play_sound(SOUND_EFFECT::HERO_DEAD);
				ddf -= (player.hp_max - player.hp) * DDF_PUNISHMENT;

				if (registry.spitterBullets.has(entity_other))
					registry.remove_all_components_of(entity_other);

				// initiate death unless already dying
				if (player.hp == 0 && !registry.deathTimers.has(entity))
				{
					registry.deathTimers.emplace(entity);
					
					if (player.hasWeapon) {
						if (registry.grenadeLaunchers.has(player.weapon))
							for(Entity line: registry.grenadeLaunchers.get(player.weapon).trajectory)
								registry.remove_all_components_of(line);
						registry.remove_all_components_of(player.weapon);
					}
					player.hasWeapon = false;

					Motion &motion = registry.motions.get(player_hero);
					motion.angle = M_PI / 2;
					motion.velocity = vec2(0, 100);
					registry.colors.get(player_hero) = vec3(1, 0, 0);
				}
			}
			// Checking Player - Collectable collision
			else if (registry.collectables.has(entity_other))
			{
				if (!registry.deathTimers.has(entity) && pickupKeyStatus)
				{
					collect(entity_other, player_hero);
				}
			}
		}
		else if (registry.weaponHitBoxes.has(entity))
		{
			if ((registry.enemies.has(entity_other) && registry.enemies.get(entity_other).hittable) && registry.weaponHitBoxes.get(entity).isActive)
			{
				if (registry.enemies.has(entity_other)) {
					//printf("Health: %d, Damage: %d\n", registry.enemies.get(entity_other).health, registry.weaponHitBoxes.get(entity).damage);
					Enemies& enemy = registry.enemies.get(entity_other);
					enemy.health -= registry.weaponHitBoxes.get(entity).damage;
					if (enemy.health <= 0) {
						points += 10;
						ddf += 5.f;
					}
					enemy.hittable = false;
					enemy.hitting = false;
					if (!registry.fireEnemies.has(entity_other))
						registry.motions.get(entity_other).dir = registry.motions.get(entity).position.x < registry.motions.get(entity_other).position.x ? -1 : 1;
					registry.animated.get(entity_other).oneTimeState = enemy.health <= 0 ? enemy.death_animation : enemy.hit_animation;
				}
				
				if (registry.bullets.has(entity) || registry.rockets.has(entity) || registry.grenades.has(entity)) {
					if (registry.rockets.has(entity) || registry.grenades.has(entity))
						explode(renderer, registry.motions.get(entity).position, entity);
					registry.remove_all_components_of(entity);
				}
			} else if (registry.blocks.has(entity_other) && (registry.bullets.has(entity) || registry.rockets.has(entity))) {
				if (registry.rockets.has(entity))
					explode(renderer, registry.motions.get(entity).position, entity);
				registry.remove_all_components_of(entity);
			} else if (registry.swords.has(entity) && registry.spitterBullets.has(entity_other)) {
				registry.remove_all_components_of(entity_other);
			}
		}
		else if (registry.blocks.has(entity))
		{
			if (registry.solids.has(entity_other)) {
				Motion& block_motion = registry.motions.get(entity);
				Motion& solid_motion = registry.motions.get(entity_other);
				vec2 scale1 = vec2({abs(block_motion.scale.x), abs(block_motion.scale.y)}) / 2.f;
				vec2 scale2 = vec2({abs(solid_motion.scale.x), abs(solid_motion.scale.y)}) / 2.f;
				float vCollisionDepth = scale1.y + scale2.y - abs(block_motion.position.y - solid_motion.position.y);
				float hCollisionDepth = scale1.x + scale2.x - abs(block_motion.position.x - solid_motion.position.x);
				if (vCollisionDepth > 0 && (hCollisionDepth <= 0 || vCollisionDepth < hCollisionDepth)) {
					if (solid_motion.position.y < block_motion.position.y && solid_motion.velocity.y > 0) {
						solid_motion.position.y = block_motion.position.y - scale1.y - scale2.y;
						solid_motion.velocity.y = 0;
					} else if (solid_motion.position.y > block_motion.position.y && solid_motion.velocity.y < 0) {
						solid_motion.position.y = block_motion.position.y + scale1.y + scale2.y;
						solid_motion.velocity.y = 0;
					}
				} else {
					if (solid_motion.position.x < block_motion.position.x) {
						solid_motion.position.x = block_motion.position.x - scale1.x - scale2.x;
					} else {
						solid_motion.position.x = block_motion.position.x + scale1.x + scale2.x;
					}
				}

				if (registry.ghouls.has(entity_other)) {
					int a = 1;
				}
				// if (registry.players.has(entity_other)) {
					if ((solid_motion.position.y <= block_motion.position.y - block_motion.scale.y / 2.f - solid_motion.scale.y / 2.f)) {
						if (registry.players.has(entity_other)) {
							registry.players.get(entity_other).jumps = MAX_JUMPS + (registry.players.get(entity_other).equipment_type == COLLECTABLE_TYPE::WINGED_BOOTS ? 1 : 0);
						} else if (registry.ghouls.has(entity_other) && registry.ghouls.get(entity_other).left_x == -1.f) {
							registry.enemies.get(entity_other).hittable = true;
							registry.enemies.get(entity_other).hitting = true;
							registry.animated.get(entity_other).oneTimeState = 5;
							registry.colors.insert(entity_other, {1, .8f, .8f});
							registry.ghouls.get(entity_other).left_x = block_motion.position.x - scale1.x;
							registry.ghouls.get(entity_other).right_x = block_motion.position.x + scale1.x;
						}
					} else if (solid_motion.position.x <= block_motion.position.x - block_motion.scale.x / 2.f - solid_motion.scale.x / 2.f) {
						if (registry.players.has(entity_other) && motionKeyStatus.test(0) && registry.players.get(entity_other).equipment_type == COLLECTABLE_TYPE::PICKAXE && solid_motion.position.y > 0) {
							use_pickaxe(player_hero, 0, MAX_JUMPS);
						} else if (registry.ghouls.has(entity_other)) {
							registry.motions.get(entity_other).velocity.x = -registry.motions.get(entity_other).velocity.x;
							registry.motions.get(entity_other).dir = -registry.motions.get(entity_other).dir;
						}
					} else if (solid_motion.position.x >= block_motion.position.x + block_motion.scale.x / 2.f + solid_motion.scale.x / 2.f) {
						if (registry.players.has(entity_other) && motionKeyStatus.test(1) && registry.players.get(entity_other).equipment_type == COLLECTABLE_TYPE::PICKAXE && solid_motion.position.y > 0) {
							use_pickaxe(player_hero, 1, MAX_JUMPS);
						} else if (registry.ghouls.has(entity_other)) {
							registry.motions.get(entity_other).velocity.x = -registry.motions.get(entity_other).velocity.x;
							registry.motions.get(entity_other).dir = -registry.motions.get(entity_other).dir;
						}
					}
				// }
			} 
			else if (registry.projectiles.has(entity_other))
			{
				Motion& block_motion = registry.motions.get(entity);
				Motion& projectile_motion = registry.motions.get(entity_other);
				vec2 scale1 = vec2({abs(block_motion.scale.x), abs(block_motion.scale.y)}) / 2.f;
				vec2 scale2 = vec2({abs(projectile_motion.scale.x), abs(projectile_motion.scale.y)}) / 2.f;
				float vCollisionDepth = (scale1.y + scale2.y) - abs(projectile_motion.position.y - block_motion.position.y);
				float hCollisionDepth = (scale1.x + scale2.x) - abs(projectile_motion.position.x - block_motion.position.x);
				if (vCollisionDepth < hCollisionDepth) {
					projectile_motion.velocity.y = -projectile_motion.velocity.y;
					projectile_motion.position.y += vCollisionDepth * (projectile_motion.position.y < block_motion.position.y ? -1 : 1);
				} else {
					projectile_motion.velocity.x = -projectile_motion.velocity.x;
					projectile_motion.position.x += hCollisionDepth * (projectile_motion.position.x < block_motion.position.x ? -1 : 1);
				}
				projectile_motion.velocity *= projectile_motion.friction;
			} 
		} else if (registry.parallaxBackgrounds.has(entity) && registry.renderRequests.get(entity).used_texture == TEXTURE_ASSET_ID::PARALLAX_LAVA) {
			if (registry.bullets.has(entity_other) || registry.rockets.has(entity_other) || registry.grenades.has(entity_other) || registry.spitterBullets.has(entity_other) || registry.collectables.has(entity_other))
				registry.remove_all_components_of(entity_other);
			else if (registry.players.has(entity_other) && !registry.deathTimers.has(entity_other)) {
				// Scream, reset timer, and make the hero fall
				registry.deathTimers.emplace(entity_other);
				Player& player = registry.players.get(entity_other);
			
				if (player.hasWeapon) {
					if (registry.grenadeLaunchers.has(player.weapon))
						for(Entity line: registry.grenadeLaunchers.get(player.weapon).trajectory)
							registry.remove_all_components_of(line);
					registry.remove_all_components_of(player.weapon);
				}
				player.hasWeapon = false;
				player.invulnerable_timer = max(3000.f, registry.players.get(player_hero).invulnerable_timer);
				player.invuln_type = INVULN_TYPE::HIT;

				play_sound(SOUND_EFFECT::HERO_DEAD);
				Motion &motion = registry.motions.get(player_hero);
				motion.angle = M_PI / 2;
				motion.velocity = vec2(0, 100);
				registry.colors.get(player_hero) = vec3(1, 0, 0);
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
			playerMotion.dir = -1;
		else if (playerMotion.velocity.x > 0)
			playerMotion.dir = 1;
	}
}

void WorldSystem::clear_enemies()
{
	std::vector<Entity> justKillThem = { };
	for (uint i = 0; i < registry.enemies.size(); i++)
	{
		Entity enemy = registry.enemies.entities[i];
		if (!registry.followingEnemies.has(enemy)) {
			justKillThem.push_back(registry.enemies.entities[i]);
		}
	}
	for (Entity e : justKillThem) registry.remove_all_components_of(e);
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

		if ((key == GLFW_KEY_W || key == GLFW_KEY_SPACE) && action == GLFW_PRESS && !pause && !registry.gravities.get(player_hero).dashing)
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
		} else if (key == GLFW_KEY_1 && action == GLFW_PRESS && !pause && debug) {
			if (mod == GLFW_MOD_SHIFT) {

			} else {
				createSword(renderer, registry.motions.get(player_hero).position);
			}
		} else if (key == GLFW_KEY_2 && action == GLFW_PRESS && !pause && debug) {
			if (mod == GLFW_MOD_SHIFT) {

			} else {
				createGun(renderer, registry.motions.get(player_hero).position);
			}
		} else if (key == GLFW_KEY_3 && action == GLFW_PRESS && !pause && debug) {
			if (mod == GLFW_MOD_SHIFT) {
				next_enemy_spawn = -1.0;
				spawn_move_ghouls(0);
			} else {
				createGrenadeLauncher(renderer, registry.motions.get(player_hero).position);
			}
		} else if (key == GLFW_KEY_4 && action == GLFW_PRESS && !pause && debug) {
			if (mod == GLFW_MOD_SHIFT) {
				next_spitter_spawn = -1.0;
            	spawn_spitter_enemy(0);
			} else {
				createRocketLauncher(renderer, registry.motions.get(player_hero).position);
			}
		} else if (key == GLFW_KEY_5 && action == GLFW_PRESS && !pause && debug) {
			createLaserRifle(renderer, registry.motions.get(player_hero).position);
		} else if (key == GLFW_KEY_6 && action == GLFW_PRESS && !pause && debug) {
			createHeart(renderer, registry.motions.get(player_hero).position);
		} else if (key == GLFW_KEY_7 && action == GLFW_PRESS && !pause && debug) {
			createWingedBoots(renderer, registry.motions.get(player_hero).position);
		} else if (key == GLFW_KEY_8 && action == GLFW_PRESS && !pause && debug) {
			createPickaxe(renderer, registry.motions.get(player_hero).position);
		} else if (key == GLFW_KEY_9 && action == GLFW_PRESS && !pause && debug) {
			createDashBoots(renderer, registry.motions.get(player_hero).position);
		}

		if (key == GLFW_KEY_I && action == GLFW_PRESS && debug)
		{
			if (registry.players.get(player_hero).invuln_type != INVULN_TYPE::HEAL)
			{
				registry.players.get(player_hero).invulnerable_timer = 12550821.f;
				registry.players.get(player_hero).invuln_type = INVULN_TYPE::HEAL;
			}
			else
			{
				registry.players.get(player_hero).invulnerable_timer = 0.f;
				registry.players.get(player_hero).invuln_type = INVULN_TYPE::NONE;
			}
		}
		if (key == GLFW_KEY_K && action == GLFW_PRESS && debug)
		{
			clear_enemies();
		}

		if (key == GLFW_KEY_X && action == GLFW_PRESS)
		{
			save_game();
		}

		if (key == GLFW_KEY_S && action == GLFW_PRESS && !pause && !registry.gravities.get(player_hero).dashing) {
			pickupKeyStatus = true;
		}

		if (key == GLFW_KEY_S && action == GLFW_RELEASE && !pause && !registry.gravities.get(player_hero).dashing) {
			pickupKeyStatus = false;
		}
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
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
	{
		debug = !debug;
	}
	
	if (key == GLFW_KEY_COMMA && action == GLFW_RELEASE && debug && ddl < 4)
	{
		ddf -= 100;
	}
	if (key == GLFW_KEY_PERIOD && action == GLFW_RELEASE && debug && ddl != 4)
	{
		ddf += 100;
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS && debug && inBossLevel) {
		ddf = 500;
		ddl = 5;
		for (Entity decorator : db_decorator) {
			registry.remove_all_components_of(decorator);
		}
		db_decorator.clear();
		registry.renderRequests.get(difficulty_bar).used_texture = TEXTURE_ASSET_ID::DIFFICULTY_BAR_BROKEN;
		registry.motions.get(difficulty_bar).position[1] = 730.f;
		db_decorator.push_back(createDBSatan(renderer, DB_SATAN_CORD));
	}
	
	if (action == GLFW_RELEASE && key == GLFW_KEY_M) {
		toggle_mute_music();
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    // Calculate the scaling factors for X and Y

    // Scale the mouse position
    float ox = 0, oy = 0;
	int new_w = w, new_h = h;

    float aspect_ratio = window_width_px / (float) window_height_px; // 16:9
    float new_aspect_ratio = w / (float) h;

    if (aspect_ratio < new_aspect_ratio) {
        new_w = h * aspect_ratio;
        ox = (w-new_w)/2.0;
        // w = new_w;
    } else {
        new_h = w / aspect_ratio;
        oy = (h-new_h) / 2.0;
        // h = new_h;
    }

	mouse_pos.x = (mouse_position.x - ox) / new_w * window_width_px;
    mouse_pos.y = (mouse_position.y - oy) / new_h * window_height_px;

	if (!registry.deathTimers.has(player_hero) && !pause)
		for (Entity weapon : registry.weapons.entities)
			update_weapon_angle(renderer, weapon, mouse_pos);
}

void WorldSystem::on_mouse_click(int key, int action, int mods){
    if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		mouse_clicked = true;
		for (Entity entity: registry.buttons.entities) {
			Motion &button = registry.motions.get(entity);
        	GameButton &buttonInfo = registry.buttons.get(entity);
        	RenderRequest &buttonRender = registry.renderRequests.get(entity);
			if (abs(button.position.x - mouse_pos.x) < button.scale.x / 2 && abs(button.position.y - mouse_pos.y) < button.scale.y / 2 && buttonRender.visibility) {
				buttonInfo.clicked = true;
                play_sound(SOUND_EFFECT::BUTTON_CLICK);
				return;
			}
		}
		if (!pause && registry.players.size() > 0 && !registry.gravities.get(player_hero).dashing) {
			for (Entity weapon : registry.weapons.entities)
				do_weapon_action(renderer, weapon, mouse_pos);
		}
	} else if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
		mouse_clicked = false;
		for (Entity entity: registry.buttons.entities) {
        	GameButton &buttonInfo = registry.buttons.get(entity);
			if (buttonInfo.clicked) {
				buttonInfo.clicked = false;
            	buttonInfo.callback();
				return;
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
