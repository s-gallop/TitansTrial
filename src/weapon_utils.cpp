// internal
#include "weapon_utils.hpp"
#include "world_init.hpp"
#include "sound_utils.hpp"

float next_collectable_spawn = 1000.f;

const size_t COLLECTABLE_DELAY_MS = 12000;
const size_t MAX_COLLECTABLES = 3;
const size_t GUN_COOLDOWN = 800;

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
		case COLLECTABLE_TYPE::PICKAXE: {
			registry.players.get(hero).equipment_type = COLLECTABLE_TYPE::PICKAXE;
			registry.remove_all_components_of(collectable);
		}
		break;
		case COLLECTABLE_TYPE::WINGED_BOOTS: {
			registry.players.get(hero).equipment_type = COLLECTABLE_TYPE::WINGED_BOOTS;
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

void update_weapon(RenderSystem* renderer, float elapsed_ms, Entity weapon, Entity hero) {
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

float spawn_collectable(RenderSystem* renderer, int ddl) {
	float x_pos = uniform_dist(rng) * (window_width_px - 120) + 60;
	float y_pos = uniform_dist(rng) * (window_height_px - 350) + 50;

	float rand = uniform_dist(rng);
	createWingedBoots(renderer, {x_pos, y_pos});
	// if (ddl == 0)
	// 	createSword(renderer, { x_pos, y_pos });
	// else if (ddl == 1)
	// 	if (rand > 0.8)
	// 		createGun(renderer, { x_pos, y_pos });
	// 	else
	// 		createSword(renderer, { x_pos, y_pos });
	// else
	// 	if (rand > 0.5)
	// 		createGun(renderer, { x_pos, y_pos });
	// 	else
	// 		createSword(renderer, { x_pos, y_pos });

	return (COLLECTABLE_DELAY_MS / 2) + uniform_dist(rng) * (COLLECTABLE_DELAY_MS / 2);
}

void update_collectable_timer(float elapsed_ms, RenderSystem* renderer, int ddl) {
	next_collectable_spawn -= elapsed_ms;
	if (registry.collectables.components.size() < MAX_COLLECTABLES && next_collectable_spawn < 0.f)
		next_collectable_spawn = spawn_collectable(renderer, ddl);
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
	registry.motions.get(hero).velocity = {0, 0};
	registry.motions.get(hero).scale.x = (direction ? 1 : -1) * abs(registry.motions.get(hero).scale.x);
	registry.players.get(hero).jumps = max_jumps;
	registry.gravities.get(hero).lodged.set(direction);
	play_sound(SOUND_EFFECT::PICKAXE);
}