#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::BULLET, mesh_path("bullet.obj"))
		// specify meshes of other assets here
	};

	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> collision_mesh_paths = 
	{
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::SPRITE, mesh_path("sprite_hull.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::BULLET, mesh_path("bullet_hull.obj"))
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("hero.png"),
		textures_path("mock_enemy.png"),
		textures_path("fire_enemy.png"),
		textures_path("ghoul.png"),
		textures_path("following_enemy.png"),
		textures_path("spitter.png"),
		textures_path("spitter_bullet.png"),
		textures_path("sword.png"),
		textures_path("pistol.png"),
		textures_path("rocket_launcher.png"),
		textures_path("rocket.png"),
		textures_path("grenade_launcher.png"),
		textures_path("grenade.png"),
		textures_path("explosion.png"),
		textures_path("heart.png"),
		textures_path("pickaxe.png"),
		/*
		© <a href='https://www.123rf.com/profile_captainvector'>captainvector</a>, <a href='https://www.123rf.com/free-images/'>123RF Free Images</a>*/
		textures_path("winged_boots.png"),
		textures_path("dash_boots.png"),
		textures_path("background.png"),
		textures_path("parallax_rain.png"),
		textures_path("parallax_moon.png"),
		textures_path("parallax_cloud_close.png"),
		textures_path("parallax_cloud_far.png"),
		textures_path("parallax_lava.png"),
		textures_path("buttons/quit.png"),
		textures_path("buttons/quit_pressed.png"),
		textures_path("buttons/menu.png"),
		textures_path("buttons/menu_pressed.png"),
		textures_path("helpers/helper.png"),
		textures_path("helpers/sword_helper.png"),
		textures_path("helpers/gun_helper.png"),
		textures_path("helpers/grenade_helper.png"),
		textures_path("helpers/rocket_helper.png"),
		textures_path("helpers/winged_boots_helper.png"),
		textures_path("helpers/pickaxe_helper.png"),
		textures_path("helpers/dash_boots_helper.png"),
		textures_path("buttons/play.png"),
		textures_path("buttons/play_pressed.png"),
		textures_path("buttons/almanac.png"),
		textures_path("buttons/almanac_pressed.png"),
		textures_path("buttons/back.png"),
		textures_path("buttons/back_pressed.png"),
		textures_path("titans_trial_logo.png"),
        textures_path("hitbox.png"),
        textures_path("black_pixel.png"),
		textures_path("line.png"),
		textures_path("pixel_heart.png"),
		textures_path("pixel_heart_steel.png"),
		textures_path("pixel_heart_heal.png"),
		textures_path("difficulty_bar.png"),
		textures_path("indicator.png"),
		textures_path("scores/score.png"),
		textures_path("scores/0.png"),
		textures_path("scores/1.png"), 
		textures_path("scores/2.png"), 
		textures_path("scores/3.png"), 
		textures_path("scores/4.png"), 
		textures_path("scores/5.png"), 
		textures_path("scores/6.png"), 
		textures_path("scores/7.png"), 
		textures_path("scores/8.png"), 
		textures_path("scores/9.png")};


	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("textured"),
		shader_path("bullet"),
		shader_path("screen"),
		shader_path("animated"),
		shader_path("hero"),
		shader_path("explosion"),
		shader_path("fire_enemy"),
		shader_path("ghoul"),
		shader_path("spitter"),
		shader_path("spitter_bullet"),
		shader_path("following_enemy"),
        shader_path("screen_layer")};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;
	std::array<CollisionMesh, geometry_count> collisionMeshes;

public:
	// Initialize the window
	bool init(GLFWwindow *window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh &getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };
	
	void initializeCollisionMeshes();
	CollisionMesh& getCollisionMesh(GEOMETRY_BUFFER_ID id) {return collisionMeshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw(bool pause, bool debug);

	mat3 createProjectionMatrix();

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3 &projection, bool pause, bool is_debug = false);
	void drawToScreen();
    void drawScreenLayer(const mat3 &projection, bool pause);

	// Window handle
	GLFWwindow *window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string &vs_path, const std::string &fs_path, GLuint &out_program);
