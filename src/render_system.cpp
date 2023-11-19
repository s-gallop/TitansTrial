// internal
#include "render_system.hpp"
#include "world_system.hpp"
#include <SDL.h>

#include "tiny_ecs_registry.hpp"

void RenderSystem::drawTexturedMesh(Entity entity, const mat3 &projection, bool pause, bool is_debug)
{
    assert(registry.renderRequests.has(entity));
    const RenderRequest &render_request = registry.renderRequests.get(entity);

	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
    vec2 flip = {motion.dir, 1};
	if (!is_debug) {
        transform.translate(motion.positionOffset + render_request.offset * flip);
    } else {
		transform.translate(motion.positionOffset);
	}
	transform.rotate(motion.globalAngle);
	transform.scale((is_debug ? motion.scale : render_request.scale) * flip);


	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED || (uint) render_request.used_effect > (uint) EFFECT_ASSET_ID::ANIMATED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

        // does animation if texture has animation and is not DEAD
		if (registry.animated.has(entity) && !registry.deathTimers.has(entity) && !pause && !is_debug)
		{
			AnimationInfo &info = registry.animated.get(entity);
			GLint frame_loc = glGetUniformLocation(program, "frame");
            if (info.oneTimeState != -1) {
                int count = (int)floor((glfwGetTime() - info.oneTimer) * ANIMATION_SPEED_FACTOR);
                if (count < info.stateFrameLength[info.oneTimeState]) {
                    glUniform2f(frame_loc, count % info.stateFrameLength[info.oneTimeState], info.oneTimeState);
                } else {
                    info.oneTimeState = -1;
                }
            } else {
                glUniform2f(frame_loc, ((int)floor(glfwGetTime() * ANIMATION_SPEED_FACTOR) % info.stateFrameLength[info.curState]), info.curState);
            }
			GLint scale_loc = glGetUniformLocation(program, "scale");
			glUniform2f(scale_loc, info.stateCycleLength, info.states);
		}

		if (registry.players.has(entity) && !registry.deathTimers.has(entity)) {
			GLint invulnerable_time_loc = glGetUniformLocation(program, "invulnerable_timer");
			glUniform1f(invulnerable_time_loc, registry.players.get(entity).invulnerable_timer);
			GLint pi_loc = glGetUniformLocation(program, "M_PI");
			glUniform1f(pi_loc, M_PI);
		}

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		GLuint texture_id = is_debug? texture_gl_handles[(GLuint) TEXTURE_ASSET_ID::HITBOX] :texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

        assert(registry.renderRequests.has(entity));
        if (registry.buttons.has(entity))
        {
            GameButton &button = registry.buttons.get(entity);
            if (button.clicked) {
                // pressed texture must be +1 of the unpressed texture
                texture_id = texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture+1];
            }
        }
		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::COLOURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::BULLET)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen(bool pause)
{
	// Setting shaders
	// get the water texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::SCREEN]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint water_program = effects[(GLuint)EFFECT_ASSET_ID::SCREEN];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(water_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(water_program, "screen_darken_factor");
    GLuint pause_uloc = glGetUniformLocation(water_program, "pause");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.screen_darken_factor);
    glUniform1i(pause_uloc, pause);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(bool pause, bool debug)
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0.01f, 0.02f, 0.08f, 1.0f);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
    std::vector<Entity> beyonders;
    // separates what needs the screen effects and what doesn't need screen effect, Not the most efficient, could look into it later
	for (Entity entity : registry.renderRequests.entities)
	{
        RenderRequest &render_request = registry.renderRequests.get(entity);
		if (!registry.motions.has(entity) || !render_request.visibility)
			continue;
		if (render_request.on_top_screen) {
            beyonders.push_back(entity);
        } else {
            drawTexturedMesh(entity, projection_2D, pause);
        }
	}
	// Truely render to the screen
	drawToScreen(pause);
    //draws whatever is filtered out as on top of the screen effects.
    for (Entity e : beyonders) {
        drawTexturedMesh(e, projection_2D, pause);
    }
    if (debug) {
        for (Entity entity : registry.debugRenderRequests.entities)
        {
            drawTexturedMesh(entity, projection_2D, pause, true);
        }
    }

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float)window_width_px;
	float bottom = (float)window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}