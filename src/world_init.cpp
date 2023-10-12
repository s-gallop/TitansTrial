#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

Entity createHero(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
    motion.scale = {HERO_BB_WIDTH, HERO_BB_HEIGHT};
	motion.isSolid = true;

	// Create and (empty) Salmon component to be able to refer to all turtles
	registry.players.emplace(entity);
    AnimationInfo& animationInfo = registry.animated.emplace(entity);
    animationInfo.states = 4;
    animationInfo.curState = 0;
    animationInfo.stateFrameLength = {9, 8, 4, 4};
    registry.renderRequests.insert(
            entity,
            { TEXTURE_ASSET_ID::HERO,
              EFFECT_ASSET_ID::ANIMATED,
              GEOMETRY_BUFFER_ID::SPRITE });

	registry.gravities.emplace(entity);

	return entity;
}

//Entity createFish(RenderSystem* renderer, vec2 position)
//{
//	 Reserve en entity
//	auto entity = Entity();
//
//	 Store a reference to the potentially re-used mesh object
//	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//	registry.meshPtrs.emplace(entity, &mesh);
//
//	 Initialize the position, scale, and physics components
//	auto& motion = registry.motions.emplace(entity);
//	motion.angle = 0.f;
//	motion.velocity = { -50.f, 0.f };
//	motion.position = position;
//
//	 Setting initial values, scale is negative to make it face the opposite way
//	motion.scale = vec2({ -FISH_BB_WIDTH, FISH_BB_HEIGHT });
//
//	 Create an (empty) Fish component to be able to refer to all fish
//	registry.softShells.emplace(entity);
//	registry.renderRequests.insert(
//		entity,
//		{ TEXTURE_ASSET_ID::FISH,
//			EFFECT_ASSET_ID::TEXTURED,
//			GEOMETRY_BUFFER_ID::SPRITE });
//
//	return entity;
//}

Entity createEnemy(RenderSystem* renderer, vec2 position, float angle, vec2 velocity, vec2 scale)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.velocity = velocity;
	motion.scale = scale;
	
	// Create and (empty) Turtle component to be able to refer to all turtles
	registry.enemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	// registry.gravities.emplace(entity);
	registry.testAIs.emplace(entity);

	return entity;
}

Entity createBackground()
{
    Entity entity = Entity();
    auto& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = { 0.f, 0.f };
    motion.position = {window_width_px/2,window_height_px/2};
    motion.scale = {window_width_px, window_height_px};

    // Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
    registry.renderRequests.insert(
            entity,
            { TEXTURE_ASSET_ID::BACKGROUND,
              EFFECT_ASSET_ID::TEXTURED,
              GEOMETRY_BUFFER_ID::SPRITE });
    return entity;
}

Entity createSword(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({SWORD_BB_WIDTH, SWORD_BB_HEIGHT });

	// Add to swords, gravity and render requests
	registry.swords.emplace(entity);
	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SWORD,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createWeaponSword(RenderSystem* renderer) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = { 0.f, 0.f };
	motion.scale = {1.3 * SWORD_BB_WIDTH, 1.3 * SWORD_BB_HEIGHT };
	motion.positionOffset = {0.f, -50.f};

	// Add to weapons and renderRequests
	registry.weapons.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SWORD,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::PEBBLE,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createPebble(vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	// Create and (empty) Salmon component to be able to refer to all turtles
	registry.enemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::PEBBLE,
			GEOMETRY_BUFFER_ID::PEBBLE });

	return entity;
}

Entity createBlock(vec2 pos, vec2 size)
{
	auto entity = Entity();

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;
	motion.isSolid = true;
	registry.blocks.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createWeaponHitBox(vec2 pos, vec2 size)
{
	auto entity = Entity();

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;
	registry.weaponHitBoxes.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}