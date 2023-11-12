#include <map>
#include <utility>
#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"


Entity createHero(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = ASSET_SIZE.at(TEXTURE_ASSET_ID::HERO);
	motion.isSolid = true;

	registry.players.emplace(entity);
	registry.animated.emplace(entity, ANIMATION_INFO.at(TEXTURE_ASSET_ID::HERO));
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::HERO,
		 EFFECT_ASSET_ID::HERO,
		 GEOMETRY_BUFFER_ID::SPRITE,
         true,
         true,
         SPRITE_SCALE.at(TEXTURE_ASSET_ID::HERO),
         SPRITE_OFFSET.at(TEXTURE_ASSET_ID::HERO)});
    registry.debugRenderRequests.emplace(entity);

    registry.gravities.emplace(entity);
	return entity;
}

Entity createEnemy(RenderSystem *renderer, vec2 position, float angle, vec2 velocity, vec2 scale)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.velocity = velocity;
	motion.scale = {scale.x/2, scale.y/2};

	registry.enemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::ENEMY,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         true,
         scale,
         {0, -scale.y/4}});

    registry.debugRenderRequests.emplace(entity);
	registry.testAIs.emplace(entity);

	return entity;
}

Entity createSpitterEnemy(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = ASSET_SIZE.at(TEXTURE_ASSET_ID::SPITTER_ENEMY);

	SpitterEnemy &spitterEnemy = registry.spitterEnemies.emplace(entity);
	// wait 1s for first shot
	spitterEnemy.timeUntilNextShotMs = 1000.f;
	spitterEnemy.bulletsRemaining = 10;

	registry.animated.emplace(entity, ANIMATION_INFO.at(TEXTURE_ASSET_ID::SPITTER_ENEMY));
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::SPITTER_ENEMY,
		 EFFECT_ASSET_ID::SPITTER_ENEMY,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         true,
         SPRITE_SCALE.at(TEXTURE_ASSET_ID::SPITTER_ENEMY),
         SPRITE_OFFSET.at(TEXTURE_ASSET_ID::SPITTER_ENEMY)});

    registry.debugRenderRequests.emplace(entity);
	registry.gravities.emplace(entity);

	return entity;
}

Entity createSpitterEnemyBullet(RenderSystem *renderer, vec2 pos, float angle)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = angle;
	auto dir = []() -> int
	{ return rand() % 2 == 0 ? 1 : -1; };
	motion.velocity = {dir() * 300, dir() * (rand() % 300)};
	motion.scale = {-SPITTER_BULLET_BB_WIDTH, SPITTER_BULLET_BB_HEIGHT};
	motion.isProjectile = true;

	SpitterBullet &bullet = registry.spitterBullets.emplace(entity);

	bullet.mass = 1;

	AnimationInfo &animationInfo = registry.animated.emplace(entity);
	animationInfo.states = 1;
	animationInfo.curState = 0;
	animationInfo.stateFrameLength = {4};
	animationInfo.stateCycleLength = 4;
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::SPITTER_ENEMY_BULLET,
		 EFFECT_ASSET_ID::SPITTER_ENEMY_BULLET,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         true,
         motion.scale});

	return entity;
}

Entity createBackground(RenderSystem* renderer)
{
	Entity entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = {window_width_px / 2, window_height_px / 2};
	motion.scale = {window_width_px, window_height_px};

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::BACKGROUND,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         true,
         motion.scale});
	return entity;
}

Entity createHelperText(RenderSystem* renderer)
{
    Entity entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

    auto &motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = {0.f, 0.f};
    motion.scale = ASSET_SIZE.at(TEXTURE_ASSET_ID::HELPER);
    motion.position = {window_width_px - motion.scale.x/2, window_height_px - motion.scale.y/2};

    // Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
    registry.renderRequests.insert(
            entity,
            {TEXTURE_ASSET_ID::HELPER,
             EFFECT_ASSET_ID::TEXTURED,
             GEOMETRY_BUFFER_ID::SPRITE,
             true,
             false,
             motion.scale});
    registry.showWhenPaused.emplace(entity);
    return entity;
}

Entity createSword(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	motion.scale = vec2({SWORD_BB_WIDTH, SWORD_BB_HEIGHT});

	// Add to swords, gravity and render requests
	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::SWORD;
	registry.swords.emplace(entity);
	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::SWORD,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         true,
         motion.scale});

	return entity;
}

Entity createGun(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	motion.scale = vec2({GUN_BB_WIDTH, GUN_BB_HEIGHT});

	// Add to swords, gravity and render requests
	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::GUN;
	registry.guns.emplace(entity);
	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::GUN,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         true,
         motion.scale});

	return entity;
}

Entity createBullet(RenderSystem* renderer, vec2 position, float angle) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BULLET);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = angle;
	motion.velocity = vec2(800.f, 0) * mat2({cos(angle), -sin(angle)}, {sin(angle), cos(angle)});
	motion.scale = mesh.original_size * 4.f;
	
	vec3& colour = registry.colors.emplace(entity);
	colour = {.47, .47, .47};

	registry.bullets.emplace(entity);
	registry.weaponHitBoxes.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::BULLET,
			GEOMETRY_BUFFER_ID::BULLET,
          false,
          true,
          motion.scale});

	return entity;
}

Entity createRocketLauncher(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	motion.scale = vec2({ROCKET_LAUNCHER_BB_WIDTH, ROCKET_LAUNCHER_BB_HEIGHT});

	// Add to swords, gravity and render requests
	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::ROCKET_LAUNCHER;
	registry.rocketLaunchers.emplace(entity);
	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::ROCKET_LAUNCHER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createRocket(RenderSystem* renderer, vec2 position, float angle) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = angle;
	motion.velocity = vec2(500.f, 0) * mat2({cos(angle), -sin(angle)}, {sin(angle), cos(angle)});
	motion.scale = vec2({ROCKET_BB_WIDTH, ROCKET_BB_HEIGHT});;

	registry.rockets.emplace(entity);
	registry.weaponHitBoxes.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ROCKET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createGrenadeLauncher(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	motion.scale = vec2({GRENADE_LAUNCHER_BB_WIDTH, GRENADE_LAUNCHER_BB_HEIGHT});

	// Add to swords, gravity and render requests
	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::GRENADE_LAUNCHER;
	registry.grenadeLaunchers.emplace(entity);
	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::GRENADE_LAUNCHER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createGrenade(RenderSystem* renderer, vec2 position, float angle) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.velocity = vec2(500.f, 0) * mat2({cos(angle), -sin(angle)}, {sin(angle), cos(angle)});
	motion.scale = vec2({GRENADE_BB_WIDTH, GRENADE_BB_HEIGHT});
	motion.isProjectile = true;
	motion.friction = .8f;

	registry.grenades.emplace(entity);
	registry.weaponHitBoxes.emplace(entity);
	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GRENADE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createExplosion(RenderSystem *renderer, vec2 position, float size)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = position - vec2({8, 0});
	motion.scale = size * vec2({EXPLOSION_BB_WIDTH, EXPLOSION_BB_HEIGHT});

	Explosion& explosion = registry.explosions.emplace(entity);
	explosion.hit_box = createWeaponHitBox(renderer, motion.position, motion.scale / 2.f, true);

	AnimationInfo &animationInfo = registry.animated.emplace(entity);
	animationInfo.states = 1;
	animationInfo.curState = 0;
	animationInfo.stateFrameLength = {6};
	animationInfo.stateCycleLength = 6;
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::EXPLOSION,
		 EFFECT_ASSET_ID::EXPLOSION,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createHeart(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = {HEART_BB_WIDTH, HEART_BB_HEIGHT};

	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::HEART;

	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::HEART,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createPickaxe(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = {PICKAXE_BB_WIDTH, PICKAXE_BB_HEIGHT};

	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::PICKAXE;

	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::PICKAXE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createWingedBoots(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = {WINGED_BOOTS_BB_WIDTH, WINGED_BOOTS_BB_HEIGHT};

	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::WINGED_BOOTS;

	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::WINGED_BOOTS,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createDashBoots(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = {DASH_BOOTS_BB_WIDTH, DASH_BOOTS_BB_HEIGHT};

	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::DASH_BOOTS;

	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::DASH_BOOTS,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createBlock(RenderSystem* renderer, vec2 pos, vec2 size)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.scale = size;
	motion.isSolid = true;
	registry.blocks.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         false,
         motion.scale});
    registry.debugRenderRequests.emplace(entity);
	return entity;
}

Entity createWeaponHitBox(RenderSystem* renderer, vec2 pos, vec2 size, bool hits_player)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;
	WeaponHitBox& hit_box = registry.weaponHitBoxes.emplace(entity);
	hit_box.hits_player = hits_player;
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
		 EFFECT_ASSET_ID::COLOURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         true,
         motion.scale});

	return entity;
}

Entity createButton(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID type, std::function<void ()> callback, bool visibility) {
    auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

    Motion &motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.angle = 0.f;
    motion.velocity = {0.f, 0.f};
    motion.scale = ASSET_SIZE.at(type);
    GameButton &button = registry.buttons.emplace(entity);
    button.clicked = false;
    button.callback = std::move(callback);
    registry.renderRequests.insert(
            entity,
            {type,
             EFFECT_ASSET_ID::TEXTURED,
             GEOMETRY_BUFFER_ID::SPRITE,
             true,
             visibility,
            motion.scale});
    if (!visibility) {
        registry.showWhenPaused.emplace(entity);
    }

    return entity;
}

Entity createTitleText(RenderSystem* renderer, vec2 pos) {
	Entity entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = ASSET_SIZE.at(TEXTURE_ASSET_ID::TITLE_TEXT);
	motion.position = pos;

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TITLE_TEXT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         true,
         motion.scale});
	registry.showWhenPaused.emplace(entity);
	return entity;
}

//float offsetHelper(vec2 hurt_scale, vec2 sprite_scale, vec2 hurt_reduction) {
//
//}