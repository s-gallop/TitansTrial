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
const size_t ENEMY_DELAY_MS = 2000 * 3;
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
	: points(0), next_enemy_spawn(0.f)
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


void WorldSystem::create_help_screen() {
	glfwSetWindowTitle(window, "Help");
	
	ScreenState& screen = registry.screenStates.components[0];
	screen.screen_darken_factor = 0;
	isTitleScreen = true;
	
	pause = false;
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());
	createSword(renderer, { 500,500 });
	createHero(renderer, { 400,400 });
	createButton(renderer, { window_width_px / 2, window_height_px * 0.825 }, TEXTURE_ASSET_ID::QUIT, [&]() {create_title_screen(); });
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
	
	createTitleText(renderer, { window_width_px / 2, window_height_px * 0.15 });
	createButton(renderer, { window_width_px / 2, window_height_px * 0.425 }, TEXTURE_ASSET_ID::BATTLE_BUTTON, [&]() {restart_game(); });
	createButton(renderer, { window_width_px / 2, window_height_px * 0.575 }, TEXTURE_ASSET_ID::HELP_BUTTON, [&]() {create_help_screen(); });
	createButton(renderer, { window_width_px / 2, window_height_px * 0.725 }, TEXTURE_ASSET_ID::QUIT_BUTTON, [&]() {exit(0); });
	
}



// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	// internal data update section
	if (invlunerable_timer > 0.0f)
	{
		float expectedTimer = invlunerable_timer - elapsed_ms_since_last_update;
		if (expectedTimer <= 0.0f) {
			invlunerable_timer = 0.0f;
			registry.colors.get(player_hero) = player_color;
		}
		else
		{
			invlunerable_timer = expectedTimer;
		}
	}
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

	if (ddl == 0) current_enemy_spawning_speed = 1.f;
	else if (ddl == 1) current_enemy_spawning_speed = 5.f;
	else current_enemy_spawning_speed = 10.f;

	spawn_move_normal_enemies(elapsed_ms_since_last_update);

	update_collectable_timer(elapsed_ms_since_last_update * current_speed, renderer);

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
  isTitleScreen = false;
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
	
	createBackground(renderer);
	// Create a new hero
	player_hero = createHero(renderer, {100, 200});
	registry.colors.insert(player_hero, {1, 0.8f, 0.8f});
	int background_pixels_width = 768;
	int background_pixels_height = 432;

	int base_height = ceil(16 * window_height_px / background_pixels_height);
	int base_width = ceil(16 * window_width_px / background_pixels_width);


	// global variables at this .cpp to reset, don't forget it!
	motionKeyStatus.reset();
	ddl = 0;
	ddf = 0.0f;
	invlunerable_timer = 0.0f;
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
			if (registry.enemies.has(entity_other) && invlunerable_timer <= 0.0f)
			{
				// remove 1 hp
				player.hp -= 1;
				invlunerable_timer = 3000.0f;
				registry.colors.get(player_hero) = vec3(1, 0, 0);
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
					collect_weapon(entity_other, player_hero);
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
		else if (registry.collectables.has(entity))
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
					if (registry.bullets.has(entity))
						registry.remove_all_components_of(entity);
					++points;
					ddf += 10.0f;
				}
			} else if (registry.blocks.has(entity_other) && registry.bullets.has(entity)) {
				registry.remove_all_components_of(entity);
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
	float rightFactor = motionKeyStatus.test(0) ? 1 : 0;
	float leftFactor = motionKeyStatus.test(1) ? -1 : 0;
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
			motionKeyStatus.set(0);
		}
		else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			motionKeyStatus.reset(0);
		}
		else if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			motionKeyStatus.set(1);
		}
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			motionKeyStatus.reset(1);
		}

		motion_helper(playerMotion);

		if (key == GLFW_KEY_W && action == GLFW_PRESS && !pause)
		{
			if (registry.players.get(player_hero).jumps > 0)
			{
				playerMotion.velocity[1] = -JUMP_INITIAL_SPEED;
				registry.players.get(player_hero).jumps--;
				play_sound(SOUND_EFFECT::HERO_JUMP);
			}
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !pause)
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
