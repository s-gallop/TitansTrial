#include <utility>
#include "world_init.hpp"
#include "world_system.hpp"
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
    const float ACTUAL_SCALE_FACTOR = 0.5f;
    const float OFFSET_FACTOR = 4.0f;

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.velocity = velocity;
	motion.scale = {scale.x*ACTUAL_SCALE_FACTOR, scale.y*ACTUAL_SCALE_FACTOR};

	registry.enemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::ENEMY,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
         false,
         true,
         scale,
         {0, -scale.y/OFFSET_FACTOR}});

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
	spitterEnemy.timeUntilNextShotMs = INITIAL_SPITTER_PROJECTILE_DELAY_MS;
	spitterEnemy.bulletsRemaining = SPITTER_PROJECTILE_AMT;

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
	motion.scale = SPITTER_BULLET_BB;
	motion.isProjectile = true;

	SpitterBullet &bullet = registry.spitterBullets.emplace(entity);

	bullet.mass = 1;

	AnimationInfo &animationInfo = registry.animated.emplace(entity, ANIMATION_INFO.at(TEXTURE_ASSET_ID::SPITTER_ENEMY_BULLET));
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
	motion.scale = SWORD_BB;

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
	motion.scale = GUN_BB;

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
    const float GREY_SCALE = .47f;

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BULLET);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = angle;
	motion.velocity = vec2(800.f, 0) * mat2({cos(angle), -sin(angle)}, {sin(angle), cos(angle)});
	motion.scale = mesh.original_size * BULLET_MESH_SCALE;
	
	vec3& colour = registry.colors.emplace(entity);
	colour = {GREY_SCALE, GREY_SCALE, GREY_SCALE};

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
	motion.scale = ROCKET_LAUNCHER_BB;

	// Add to swords, gravity and render requests
	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::ROCKET_LAUNCHER;
	registry.rocketLaunchers.emplace(entity);
	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::ROCKET_LAUNCHER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false,
		 true,
		 motion.scale});

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
	motion.scale = ROCKET_BB;

	registry.rockets.emplace(entity);
	registry.weaponHitBoxes.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ROCKET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			false,
			true,
			motion.scale});

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
	motion.scale = GRENADE_LAUNCHER_BB;

	// Add to swords, gravity and render requests
	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::GRENADE_LAUNCHER;
	registry.grenadeLaunchers.emplace(entity);
	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::GRENADE_LAUNCHER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false,
		 true,
		 motion.scale});

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
	motion.scale = GRENADE_BB;
	motion.isProjectile = true;
	motion.friction = .8f;

	registry.grenades.emplace(entity);
	registry.weaponHitBoxes.emplace(entity);
	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GRENADE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			false,
			true,
			motion.scale});

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
	motion.position = position - size * vec2({8, 0});
	motion.scale = size * ASSET_SIZE.at(TEXTURE_ASSET_ID::EXPLOSION);

	registry.explosions.emplace(entity);
	registry.weaponHitBoxes.emplace(entity);
	
	AnimationInfo& info = registry.animated.emplace(entity, ANIMATION_INFO.at(TEXTURE_ASSET_ID::EXPLOSION));
	info.oneTimeState = 0;
	info.oneTimer = glfwGetTime();

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::EXPLOSION,
		 EFFECT_ASSET_ID::EXPLOSION,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false,
		 true,
		 size * SPRITE_SCALE.at(TEXTURE_ASSET_ID::EXPLOSION),
		 size * SPRITE_OFFSET.at(TEXTURE_ASSET_ID::EXPLOSION)});

	return entity;
}

Entity createHeart(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = HEART_BB;

	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::HEART;

	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::HEART,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false,
		 true,
		 motion.scale});

	return entity;
}

Entity createPickaxe(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = PICKAXE_BB;

	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::PICKAXE;

	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::PICKAXE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false,
		 true,
		 motion.scale});

	return entity;
}

Entity createWingedBoots(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = WINGED_BOOTS_BB;

	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::WINGED_BOOTS;

	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::WINGED_BOOTS,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false,
		 true,
		 motion.scale});

	return entity;
}

Entity createDashBoots(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = DASH_BOOTS_BB;

	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.type = COLLECTABLE_TYPE::DASH_BOOTS;

	registry.gravities.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::DASH_BOOTS,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 false,
		 true,
		 motion.scale});

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

Entity createWeaponHitBox(RenderSystem* renderer, vec2 pos, vec2 size)
{
	auto entity = Entity();
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;
	registry.weaponHitBoxes.emplace(entity);
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

Entity createDifficultyBar(RenderSystem* renderer, vec2 pos) {
	Entity entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = ASSET_SIZE.at(TEXTURE_ASSET_ID::DIFFICULTY_BAR);
	motion.position = pos;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DIFFICULTY_BAR,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 true,
		 true,
		 motion.scale });

	registry.inGameGUIs.emplace(entity);

	return entity;
}

Entity createPlayerHeart(RenderSystem* renderer, vec2 pos) {
	Entity entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = ASSET_SIZE.at(TEXTURE_ASSET_ID::PLAYER_HEART);
	motion.position = pos;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_HEART,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 true,
		 false,
		 motion.scale });

	registry.inGameGUIs.emplace(entity);

	return entity;
}

//float offsetHelper(vec2 hurt_scale, vec2 sprite_scale, vec2 hurt_reduction) {
//
//}