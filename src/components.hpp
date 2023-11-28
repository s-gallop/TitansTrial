#pragma once
#include "common.hpp"
#include <vector>
#include <bitset>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include <list>

enum class COLLECTABLE_TYPE
{
	SWORD = 0,
	GUN = SWORD + 1,
	ROCKET_LAUNCHER = GUN + 1,
	GRENADE_LAUNCHER = ROCKET_LAUNCHER + 1,
	HEART = GRENADE_LAUNCHER + 1,
	PICKAXE = HEART + 1,
	WINGED_BOOTS = PICKAXE + 1,
	DASH_BOOTS = WINGED_BOOTS + 1,
	COLLECTABLE_COUNT = DASH_BOOTS + 1
};

enum class INVULN_TYPE
{
	NONE = 0,
	HIT = NONE + 1,
	HEAL = HIT + 1
};

struct Blank
{

};

// Player component
struct Player
{
	// Flag for player having weapon
	//  Only using 0 & 1 right now but other values available for more weapons
	//  hasSword = 1
	uint hasWeapon = 0;
	Entity weapon;
	COLLECTABLE_TYPE equipment_type = COLLECTABLE_TYPE::COLLECTABLE_COUNT;
	float equipment_timer = 0;
	uint jumps = 2; 
	int hp_max = 5;
	int hp = 5;
	float invulnerable_timer = 0;
	INVULN_TYPE invuln_type = INVULN_TYPE::NONE;
};

struct Block
{
};

struct ParallaxBackground
{
	// where to reset entity to once it reaches end of screen
	vec2 resetPosition;
};

struct Enemies
{
	bool hittable = true;
	bool hitting = true;

	float invulnerable_time = 0.f;
	int health = 2;
	int hit_animation = -1;
	
	bool dying = false;
	float death_time = 500.f;
	int death_animation = -1;
};

struct FireEnemy {

};

struct Ghoul
{
	float right_x = -1.f;
	float left_x = -1.f;
};

struct FollowingEnemies
{
	std::list<vec2> path;
	float next_blink_time = 0.f;
	bool blinked = false;
};

struct SpitterEnemy
{
	uint bulletsRemaining;
	float timeUntilNextShotMs;
    bool canShoot;
	float right_x = -1.f;
	float left_x = -1.f;
};

struct SpitterBullet 
{
	float mass; // this eventually decays to 0 and the bullet disappears
};

struct Collectable
{
	COLLECTABLE_TYPE type;
	float despawn_timer = 15000;
};

struct Sword
{
	// Swing State: 0 = not swinging, 1 = wind-up right, 2 = swing right, 3 = wind-up left, 4 = swing left
	int swing = 0;
};

struct Gun{
	float cooldown = 0;
	bool loaded = true;
};

struct Bullet {

};

struct RocketLauncher {
	float cooldown = 0;
	bool loaded = true;
};

struct Rocket {

};

struct GrenadeLauncher {
	float cooldown = 0;
	bool loaded = true;
	std::vector<Entity> trajectory;
};

struct Grenade {
	float explode_timer = 3000;
};

struct Explosion {
	float timer = 1000;
};

// Weapon the player has picked up
struct Weapon
{
	std::vector<Entity> hitBoxes;
	COLLECTABLE_TYPE type;
};

struct WeaponHitBox
{
	bool soundPlayed = false;
	bool isActive = true;
	int damage = 1;
};

// All data relevant to the shape and motion of entities
struct Motion
{
	vec2 position = {0.f, 0.f};
	float angle = 0.f;
	float angleBackup = 0.f;
	float globalAngle = 0.f;
	vec2 velocity = {0.f, 0.f};
	vec2 scale = {10.f, 10.f};
	vec2 positionOffset = {0.f, 0.f};
	bool isSolid = false;
	bool isProjectile = false;
	int dir = 1;
	float friction = 1.f;
};

struct Solid {

};

struct Projectile {
	float friction = 1.f;
};

// just for milestone 1 sudden requirement
struct TestAI
{
	bool departFromRight = true;
	float a;
	float b;
	float c;
};

// Gravity is valid for all entities in this struct
struct Gravity
{
	std::bitset<2> lodged = std::bitset<2>("00");
	uint dashing = 0;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	Collision(Entity &other_entity) { this->other_entity = other_entity; };
};

// Data structure for toggling debug mode
struct Debug
{
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float screen_darken_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying player
struct DeathTimer
{
	float timer_ms = 3000.f;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

struct AnimationInfo
{
	int states;
	std::vector<int> stateFrameLength;
	int curState;
	int stateCycleLength;
    int oneTimeState = -1;
    double oneTimer;
};

struct ShowWhenPaused {
};

struct GameButton {
    int clicked;
    std::function<void ()> callback;
};

struct InGameGUI {

};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
	vec2 original_size = {1, 1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct CollisionMesh {
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<std::pair<int, int>>& out_edges, vec2 &out_size);
	vec2 original_size = {1, 1};
	std::vector<ColoredVertex> vertices;
	std::vector<std::pair<int, int>> edges;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID
{
	HERO = 0,
	ENEMY = HERO + 1,
	FIRE_ENEMY = ENEMY + 1,
	GHOUL_ENEMY = FIRE_ENEMY + 1,
	FOLLOWING_ENEMY = GHOUL_ENEMY + 1,
	SPITTER_ENEMY = FOLLOWING_ENEMY + 1,
	SPITTER_ENEMY_BULLET = SPITTER_ENEMY + 1,
	SWORD = SPITTER_ENEMY_BULLET + 1,
	GUN = SWORD + 1,
	ROCKET_LAUNCHER = GUN + 1,
	ROCKET = ROCKET_LAUNCHER + 1,
	GRENADE_LAUNCHER = ROCKET + 1,
	GRENADE = GRENADE_LAUNCHER + 1,
	EXPLOSION = GRENADE + 1,
	HEART = EXPLOSION + 1,
	PICKAXE = HEART + 1,
	WINGED_BOOTS = PICKAXE + 1,
	DASH_BOOTS = WINGED_BOOTS + 1,
	BACKGROUND = DASH_BOOTS + 1,
	PARALLAX_RAIN = BACKGROUND + 1,
	PARALLAX_MOON = PARALLAX_RAIN + 1,
	PARALLAX_CLOUDS_CLOSE = PARALLAX_MOON + 1,
	PARALLAX_CLOUDS_FAR = PARALLAX_CLOUDS_CLOSE + 1,
	PARALLAX_LAVA = PARALLAX_CLOUDS_FAR + 1,
	QUIT = PARALLAX_LAVA + 1,
	QUIT_PRESSED = QUIT + 1,
	MENU = QUIT_PRESSED + 1,
	MENU_PRESSED = MENU + 1,
	HELPER = MENU_PRESSED + 1,
	SWORD_HELPER = HELPER + 1,
	GUN_HELPER = SWORD_HELPER + 1,
	GRENADE_HELPER = GUN_HELPER + 1,
	ROCKET_HELPER = GRENADE_HELPER + 1,
	WINGED_BOOTS_HELPER = ROCKET_HELPER + 1,
	PICKAXE_HELPER = WINGED_BOOTS_HELPER + 1,
	DASH_BOOTS_HELPER = PICKAXE_HELPER + 1,
	PLAY = DASH_BOOTS_HELPER + 1,
	PLAY_PRESSED = PLAY + 1,
	ALMANAC = PLAY_PRESSED + 1,
	ALMANAC_PRESSED = ALMANAC + 1,
	BACK = ALMANAC_PRESSED + 1,
	BACK_PRESSED = BACK + 1,
	TITLE_TEXT = BACK_PRESSED + 1,
	HITBOX = TITLE_TEXT + 1,
    BLACK_LAYER = HITBOX + 1,
	LINE = BLACK_LAYER + 1,
	PLAYER_HEART = LINE + 1,
	PLAYER_HEART_STEEL = PLAYER_HEART + 1,
	PLAYER_HEART_HEAL = PLAYER_HEART_STEEL + 1,
	DIFFICULTY_BAR = PLAYER_HEART_HEAL + 1,
	INDICATOR = DIFFICULTY_BAR + 1,
	SCORE = INDICATOR + 1,
	ZERO = SCORE + 1,
	ONE = ZERO + 1,
	TWO = ONE + 1,
	THREE = TWO + 1,
	FOUR = THREE + 1,
	FIVE = FOUR + 1,
	SIX = FIVE + 1,
	SEVEN = SIX + 1,
	EIGHT = SEVEN + 1,
	NINE = EIGHT + 1,
	TEXTURE_COUNT = NINE + 1
};

const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID
{
	COLOURED = 0,
	TEXTURED = COLOURED + 1,
	BULLET = TEXTURED + 1,
	SCREEN = BULLET + 1,
	ANIMATED = SCREEN + 1,
	HERO = ANIMATED + 1,
	EXPLOSION = HERO + 1,
	ENEMY = EXPLOSION + 1,
	SPITTER_ENEMY = ENEMY + 1,
	SPITTER_ENEMY_BULLET = SPITTER_ENEMY + 1,
    SCREEN_LAYER = SPITTER_ENEMY_BULLET + 1,
	EFFECT_COUNT = SCREEN_LAYER + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID
{
	SPRITE = 0,
	BULLET = SPRITE + 1,
	DEBUG_LINE = BULLET + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest
{
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
    bool on_top_screen = false;
    bool visibility = true;
    vec2 scale = {1,1};
    vec2 offset = {0,0};
};
