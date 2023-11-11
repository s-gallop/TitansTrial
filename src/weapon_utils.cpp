// internal
#include "weapon_utils.hpp"
#include "world_init.hpp"
#include "sound_utils.hpp"

float next_collectable_spawn = 0.f;
float pickaxe_disable = 0.f;
float dash_window = 0.f;
float dash_time = 0.f;
uint dash_direction = 0;

const size_t COLLECTABLE_DELAY_MS = 12000;
const size_t MAX_COLLECTABLES = 3;
const size_t GUN_COOLDOWN = 800;
const size_t EQUIPMENT_DURATION = 10000;
const size_t DASH_WINDOW = 250;
const size_t DASH_TIME = 2250;

std::default_random_engine rng = std::default_random_engine(std::random_device()());
std::uniform_real_distribution<float> uniform_dist;

void collect_weapon(Entity weapon, Entity hero) {
	if (!registry.deathTimers.has(hero)) {
		if (registry.players.get(hero).hasWeapon && registry.weapons.get(registry.players.get(hero).weapon).type == registry.collectables.get(weapon).type) {
			registry.remove_all_components_of(weapon);
		} else {
			if (registry.players.get(hero).hasWeapon)
				registry.remove_all_components_of(registry.players.get(hero).weapon);
			registry.collectables.remove(weapon);
			registry.gravities.remove(weapon);
			registry.players.get(hero).hasWeapon = 1;
			registry.players.get(hero).weapon = weapon;
			
			Weapon& weapon_comp = registry.weapons.emplace(weapon);
			Motion& motion = registry.motions.get(weapon);
			motion.position = registry.motions.get(hero).position;
			motion.scale *= 1.3;
			motion.velocity = vec2(0, 0);
			if (registry.swords.has(weapon)) {
				motion.positionOffset.y = -50.f;
				weapon_comp.type = COLLECTABLE_TYPE::SWORD;
			} else if (registry.guns.has(weapon)) {
				motion.positionOffset.x = 30.f;
				weapon_comp.type = COLLECTABLE_TYPE::GUN;
			}
		}
	}
}

void collect(Entity collectable, Entity hero) {
	COLLECTABLE_TYPE type = registry.collectables.get(collectable).type;
	switch (type) {
		case COLLECTABLE_TYPE::GUN:
		case COLLECTABLE_TYPE::SWORD:
			collect_weapon(collectable, hero);
			break;
		case COLLECTABLE_TYPE::HEART: {
			Player& player = registry.players.get(hero);
			if (player.hp < player.hp_max) {
				play_sound(SOUND_EFFECT::HEAL);
				player.hp++;
			}
			registry.remove_all_components_of(collectable);
		}
		break;
		case COLLECTABLE_TYPE::PICKAXE:
		case COLLECTABLE_TYPE::WINGED_BOOTS:
		case COLLECTABLE_TYPE::DASH_BOOTS: 
		{
			registry.players.get(hero).equipment_type = type;
			registry.players.get(hero).equipment_timer = EQUIPMENT_DURATION;
			registry.remove_all_components_of(collectable);
		}
		break;
	}
}

void rotate_weapon(Entity weapon, vec2 mouse_pos) {
	Motion &motion = registry.motions.get(weapon);
	if (!registry.swords.has(weapon) || registry.swords.get(weapon).swing == 0) {
		motion.angle = atan2(mouse_pos.y - motion.position.y, mouse_pos.x - motion.position.x);
		
		if (registry.swords.has(weapon))
			motion.angle += M_PI/2;
		motion.angleBackup = motion.angle;
		
		if (registry.guns.has(weapon)) {
			if (motion.angle < -M_PI/2 || motion.angle > M_PI/2) {
				motion.scale.y = -1*abs(motion.scale.x);
			} else {
				motion.scale.y = abs(motion.scale.x);
			}
		}
	}
}

void swing_sword(RenderSystem* renderer, Entity weapon) {
	Motion &weaponMot = registry.motions.get(weapon);
	int &swingState = registry.swords.get(weapon).swing;
	if (swingState != 0 && swingState % 2 == 0) {
		weaponMot.angle += -1*(swingState - 3) * M_PI / 16;
		if (weaponMot.angle <= weaponMot.angleBackup - M_PI / 4 || weaponMot.angle >= weaponMot.angleBackup + M_PI / 4) {
			swingState = 0;
			weaponMot.angleBackup = weaponMot.angle;
			for (Entity hitBox : registry.weapons.get(weapon).hitBoxes)
				registry.remove_all_components_of(hitBox);
			registry.weapons.get(weapon).hitBoxes.clear();
		}
	} else if (swingState % 2 == 1) {
		weaponMot.angle -= -1*(swingState - 2) * M_PI / 32;
		if (weaponMot.angle <= weaponMot.angleBackup - M_PI / 4 || weaponMot.angle >= weaponMot.angleBackup + M_PI / 4) {
			swingState++;
			float angleBackup = weaponMot.angleBackup;
			vec2 hitBoxPos = weaponMot.position + weaponMot.positionOffset * mat2({cos(angleBackup), -sin(angleBackup)}, {sin(angleBackup), cos(angleBackup)});
			float hbScale = .9 * max(weaponMot.scale.x, weaponMot.scale.y);
			registry.weapons.get(weapon).hitBoxes.push_back(createWeaponHitBox(renderer, hitBoxPos, {hbScale, hbScale}));
			if (!registry.weaponHitBoxes.get(registry.weapons.get(weapon).hitBoxes.front()).soundPlayed) {
				registry.weaponHitBoxes.get(registry.weapons.get(weapon).hitBoxes.front()).soundPlayed = true;
				play_sound(SOUND_EFFECT::SWORD_SWING);
			}
		}
	}
}

void update_weapon(RenderSystem* renderer, float elapsed_ms, Entity hero) {
	if (registry.players.get(hero).hasWeapon) {
		Entity weapon = registry.players.get(hero).weapon;
		Motion &weaponMot = registry.motions.get(weapon);
		weaponMot.position = registry.motions.get(hero).position;
		if (registry.swords.has(weapon))
			swing_sword(renderer, weapon);
		else if (registry.guns.has(weapon)) {
			Gun& gun = registry.guns.get(weapon);
			if (!gun.loaded) {
				gun.cooldown -= elapsed_ms;
				if (gun.cooldown <= 0) {
					play_sound(SOUND_EFFECT::GUN_LEVER);
					gun.loaded = true;
				}
			}
		}
	}
}

void update_equipment(float elapsed_ms, Entity hero) {
	if (registry.players.get(hero).equipment_timer > 0) {
		Player& player = registry.players.get(hero);
		player.equipment_timer -= elapsed_ms;
		if (player.equipment_timer <= 0) {
			if (player.equipment_type == COLLECTABLE_TYPE::PICKAXE)
				registry.gravities.get(hero).lodged = false;
			player.equipment_type = COLLECTABLE_TYPE::COLLECTABLE_COUNT;
			play_sound(SOUND_EFFECT::EQUIPMENT_DROP);
		}
	}
}

void spawn_weapon(RenderSystem* renderer, vec2 pos, int ddl) {
	float rand = uniform_dist(rng);
	if (ddl == -1)
		createSword(renderer, pos);
	else if (ddl == 0)
		createSword(renderer, pos);
	else if (ddl == 1)
		if (rand > 0.8)
			createGun(renderer, pos);
		else
			createSword(renderer, pos);
	else
		if (rand > 0.5)
			createGun(renderer, pos);
		else
			createSword(renderer, pos);
}

void spawn_powerup(RenderSystem* renderer, vec2 pos, int ddl) {
	float rand = uniform_dist(rng);
	if (ddl == 0)
		if (rand < 0.6)
			createWingedBoots(renderer, pos);
		else if (rand < 0.9)
			createPickaxe(renderer, pos);
		else
			createDashBoots(renderer, pos);
	else if (ddl == 1)
		if (rand < 0.3)
			createWingedBoots(renderer, pos);
		else if (rand < 0.7)
			createPickaxe(renderer, pos);
		else
			createDashBoots(renderer, pos);
	else
		if (rand < 0.2)
			createWingedBoots(renderer, pos);
		else if (rand < 0.5)
			createPickaxe(renderer, pos);
		else
			createDashBoots(renderer, pos);
}

float spawn_collectable(RenderSystem* renderer, int ddl) {
	float x_pos = uniform_dist(rng) * (window_width_px - 120) + 60;
	float y_pos = uniform_dist(rng) * (window_height_px - 350) + 50;

	float rand = uniform_dist(rng);
	
	if (ddl == -1)
		spawn_weapon(renderer, {x_pos, y_pos}, ddl);
	else
		if (rand < 0.1)
			createHeart(renderer, {x_pos, y_pos});
		else if (rand < 0.5)
			spawn_powerup(renderer, {x_pos, y_pos}, ddl);
		else
			spawn_weapon(renderer, {x_pos, y_pos}, ddl);

	return (COLLECTABLE_DELAY_MS / 2) + uniform_dist(rng) * (COLLECTABLE_DELAY_MS / 2);
}

void update_collectable_timer(float elapsed_ms, RenderSystem* renderer, int ddl) {
	next_collectable_spawn -= elapsed_ms;
	if (registry.collectables.components.size() < MAX_COLLECTABLES && next_collectable_spawn <= 0.f)
		next_collectable_spawn = spawn_collectable(renderer, ddl);
	for (Entity entity: registry.collectables.entities) {
		Collectable& collectable = registry.collectables.get(entity);
		if (collectable.despawn_timer > 0) {
			collectable.despawn_timer -= elapsed_ms;
			if (collectable.despawn_timer <= 0) {
				registry.remove_all_components_of(entity);
			}
		}
	}
}

void do_weapon_action(RenderSystem* renderer, Entity weapon) {
	if (registry.swords.has(weapon)) {
		int &swingState = registry.swords.get(weapon).swing;
		if (swingState == 0) {
			float weaponAngle = registry.motions.get(weapon).angle;
			swingState = (weaponAngle < 0 || weaponAngle > M_PI) ? 3 : 1;
			printf("");
		}
	} else if (registry.guns.has(weapon)) {
		Gun& gun = registry.guns.get(weapon);
		if (gun.loaded) {
			createBullet(renderer, registry.motions.get(weapon).position, registry.motions.get(weapon).angle);
			play_sound(SOUND_EFFECT::BULLET_SHOOT);
			gun.cooldown = GUN_COOLDOWN;
			gun.loaded = false;
		}
	}
}

void use_pickaxe(Entity hero, uint direction, size_t max_jumps) {
	if (pickaxe_disable <= 0) {
		registry.motions.get(hero).velocity = {0, 0};
		registry.motions.get(hero).scale.x = (direction ? 1 : -1) * abs(registry.motions.get(hero).scale.x);
		registry.players.get(hero).jumps = max_jumps;
		registry.gravities.get(hero).lodged.set(direction);
		play_sound(SOUND_EFFECT::PICKAXE);
	}
}

void disable_pickaxe(Entity hero, uint direction, float disable_time) {
	registry.gravities.get(hero).lodged.reset(direction);
	pickaxe_disable = disable_time;
}

void update_pickaxe(float elapsed_ms) {
	pickaxe_disable -= elapsed_ms;
}

void check_dash_boots(Entity hero, uint direction) {
	if (!registry.gravities.get(hero).dashing) {
		if (direction == dash_direction && dash_window > 0 && dash_time <= 0) {
			registry.gravities.get(hero).dashing = 1;
			registry.motions.get(hero).velocity = {(direction ? -1 : 1) * 750.f, 0.f};
			dash_time = DASH_TIME;
			play_sound(SOUND_EFFECT::DASH);
		} else {
			dash_direction = direction;
			dash_window = DASH_WINDOW;
		}
	}
}

void update_dash_boots(float elapsed_ms, Entity hero, std::bitset<2> motionKeyStatus, float speed) {
	if (dash_time > 0) {
		dash_time -= elapsed_ms;
		if (dash_time < 2000) {
			registry.gravities.get(hero).dashing = 0;
			float rightFactor = motionKeyStatus.test(0) ? 1 : 0;
			float leftFactor = motionKeyStatus.test(1) ? -1 : 0;
			Motion& playerMotion = registry.motions.get(hero);
			playerMotion.velocity[0] = speed * (rightFactor + leftFactor);
			if (playerMotion.velocity.x < 0)
				playerMotion.scale.x = -1 * abs(playerMotion.scale.x);
			else if (playerMotion.velocity.x > 0)
				playerMotion.scale.x = abs(playerMotion.scale.x);
		}
	} else if (dash_window > 0) {
		dash_window -= elapsed_ms;
	}
}