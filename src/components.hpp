#pragma once
#include "common.hpp"
#include <vector>
#include <bitset>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

enum class COLLECTABLE_TYPE
{
	SWORD = 0,
	GUN = SWORD + 1,
	HEART = GUN + 1,
	PICKAXE = HEART + 1,
	WINGED_BOOTS = PICKAXE + 1,
	DASH_BOOTS = WINGED_BOOTS + 1
};

// Player component
struct Player
{
	// Flag for player having weapon
	//  Only using 0 & 1 right now but other values available for more weapons
	//  hasSword = 1
	uint hasWeapon = 0;
	Entity weapon;
	COLLECTABLE_TYPE equipment_type = COLLECTABLE_TYPE::SWORD;
	uint jumps = 2; 
	int hp_max = 5;
	int hp = 5;
	float invulnerable_timer = 0;
};

struct Block
{
};

struct Enemies
{
	bool follows = false;
	std::list<vec2> path;
	vec2 cur_dest = vec2(0.f,0.f);
};

struct SpitterEnemy
{
	uint bulletsRemaining;
	float timeUntilNextShotMs;
};

struct SpitterBullet 
{
	float mass; // this eventually decays to 0 and the bullet disappears
};

struct Collectable
{
	COLLECTABLE_TYPE type;
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

// Weapon the player has picked up
struct Weapon
{
	std::vector<Entity> hitBoxes;
	COLLECTABLE_TYPE type;
};

struct WeaponHitBox
{
	bool soundPlayed = false;
};

// All data relevant to the shape and motion of entities
struct Motion
{
	vec2 position = {0.f, 0.f};
	float angle = 0.f;
	float angleBackup = 0.f;
	vec2 velocity = {0.f, 0.f};
	vec2 scale = {10.f, 10.f};
	vec2 positionOffset = {0.f, 0.f};
	bool isSolid = false;
	bool isProjectile = false;
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
};

struct ShowWhenPaused {
};

struct GameButton {
    int clicked;
    std::function<void ()> callback;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
	vec2 original_size = {1, 1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
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
	SPITTER_ENEMY = ENEMY + 1,
	SPITTER_ENEMY_BULLET = SPITTER_ENEMY + 1,
	SWORD = SPITTER_ENEMY_BULLET + 1,
	GUN = SWORD + 1,
	HEART = GUN + 1,
	PICKAXE = HEART + 1,
	WINGED_BOOTS = PICKAXE + 1,
	DASH_BOOTS = WINGED_BOOTS + 1,
	BACKGROUND = DASH_BOOTS + 1,
	QUIT = BACKGROUND + 1,
	QUIT_PRESSED = QUIT + 1,
	MENU = QUIT_PRESSED + 1,
	MENU_PRESSED = MENU + 1,
	HELPER = MENU_PRESSED + 1,
	PLAY = HELPER + 1,
	PLAY_PRESSED = PLAY + 1,
	TITLE_TEXT = PLAY_PRESSED + 1,
	TEXTURE_COUNT = TITLE_TEXT + 1,
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
	SPITTER_ENEMY = HERO + 1,
	SPITTER_ENEMY_BULLET = SPITTER_ENEMY + 1,
	EFFECT_COUNT = SPITTER_ENEMY_BULLET + 1
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
};
