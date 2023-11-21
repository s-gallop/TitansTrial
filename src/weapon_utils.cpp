// internal
#include "weapon_utils.hpp"
#include "world_init.hpp"
#include "sound_utils.hpp"
#include "physics_system.hpp"

float next_collectable_spawn = 600.f;
vec2 mouse_click_pos = {-1.f, -1.f};
vec2 mouse_cur_pos = {-1.f, -1.f};
float pickaxe_disable = 0.f;
float dash_window = 0.f;
float dash_time = 0.f;
uint dash_direction = 0;

const size_t COLLECTABLE_DELAY_MS = 3000;
const size_t MAX_COLLECTABLES = 6;
const size_t GUN_COOLDOWN = 800;
const size_t EQUIPMENT_DURATION = 20000;
const size_t ROCKET_COOLDOWN = 3000;
const float ROCKET_EXPLOSION_FACTOR = 3.5f;
const size_t GRENADE_COOLDOWN = 1750;
const float GRENADE_SPEED_FACTOR = 1.f;
const size_t GRENADE_TRAJECTORY_WIDTH = 3;
const size_t GRENADE_TRAJECTORY_SEGMENT_TIME = 50;
const float GRENADE_EXPLOSION_FACTOR = 2.5f;
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
			registry.gravities.remove(weapon);
			registry.players.get(hero).hasWeapon = 1;
			registry.players.get(hero).weapon = weapon;
			
			Weapon& weapon_comp = registry.weapons.emplace(weapon);
			weapon_comp.type = registry.collectables.get(weapon).type;
			registry.collectables.remove(weapon);
			registry.collisionMeshPtrs.remove(weapon);
			
			Motion& motion = registry.motions.get(weapon);
			motion.position = registry.motions.get(hero).position;
			motion.scale *= 1.3;
			registry.renderRequests.get(weapon).scale *= 1.3;
			motion.velocity = vec2(0, 0);
			
			if (registry.swords.has(weapon)) {
				motion.positionOffset.y = -50.f;
			} else if (registry.guns.has(weapon)) {
				motion.positionOffset.x = 30.f;
			} else if (registry.rocketLaunchers.has(weapon)) {
				motion.positionOffset.x = 20.f;
			} else if (registry.grenadeLaunchers.has(weapon)) {
				motion.positionOffset.x = 25.f;
			}
		}
	}
}

void collect(Entity collectable, Entity hero) {
	COLLECTABLE_TYPE type = registry.collectables.get(collectable).type;
	switch (type) {
		case COLLECTABLE_TYPE::GUN:
		case COLLECTABLE_TYPE::SWORD:
		case COLLECTABLE_TYPE::ROCKET_LAUNCHER:
		case COLLECTABLE_TYPE::GRENADE_LAUNCHER:
			collect_weapon(collectable, hero);
			break;
		case COLLECTABLE_TYPE::HEART: {
			Player& player = registry.players.get(hero);
			if (player.hp < player.hp_max) {
				play_sound(SOUND_EFFECT::HEAL);
				player.hp++;
				player.invulnerable_timer = max(3000.f, player.invulnerable_timer);
				player.invuln_type = INVULN_TYPE::HEAL;
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
	RenderRequest& render = registry.renderRequests.get(weapon);
	if (!registry.swords.has(weapon) || registry.swords.get(weapon).swing == 0) {
		motion.angle = atan2(mouse_pos.y - motion.position.y, mouse_pos.x - motion.position.x);
		
		if (registry.swords.has(weapon))
			motion.angle += M_PI/2;
		motion.angleBackup = motion.angle;
		
		if (registry.guns.has(weapon) || registry.rocketLaunchers.has(weapon) || registry.grenadeLaunchers.has(weapon)) {
			if (motion.angle < -M_PI/2 || motion.angle > M_PI/2) {
				render.scale.y = -1*abs(motion.scale.y);
			} else {
				render.scale.y = abs(motion.scale.y);
			}
		}
	}
}

std::vector<Entity> create_grenade_trajectory(RenderSystem* renderer, vec2 start, vec2 velocity) {
	std::vector<Entity> lines;
	vec2 start_point = start;
	vec2 end_point = start;
	float velocity_change = GRAVITY_ACCELERATION_FACTOR * GRENADE_TRAJECTORY_SEGMENT_TIME;
	float segment_seconds = GRENADE_TRAJECTORY_SEGMENT_TIME / 1000.f;
	while(end_point.y - start.y < 1.5 * window_height_px) {
		start_point = end_point;
		velocity.y += velocity_change;
		end_point = start_point + velocity * segment_seconds;
		vec2 line_position = start;
		vec2 line_offset = (start_point + end_point) / 2.f - start;
		vec2 line_scale = {sqrt(dot(end_point - start_point, end_point - start_point)), GRENADE_TRAJECTORY_WIDTH};
		float line_angle = atan2(end_point.y - start_point.y, end_point.x - start_point.x);
		lines.push_back(createLine(renderer, line_position, line_offset, line_scale, line_angle));		
	}
	return lines;
}

void update_weapon_angle(RenderSystem* renderer, Entity weapon, vec2 mouse_pos) {
	if (mouse_click_pos == vec2({-1.f, -1.f}))
		rotate_weapon(weapon, mouse_pos);
	else {
		mouse_cur_pos = mouse_pos;
		rotate_weapon(weapon, registry.motions.get(weapon).position + mouse_click_pos - mouse_cur_pos);
		for (Entity line: registry.grenadeLaunchers.get(weapon).trajectory) {
			registry.remove_all_components_of(line);
		}
		Motion& motion = registry.motions.get(weapon);
		float angle = motion.angle;
		mat2 rot_mat = {{cos(angle), -sin(angle)}, {sin(angle), cos(angle)}};
		registry.grenadeLaunchers.get(weapon).trajectory = create_grenade_trajectory(renderer, motion.position + vec2(motion.positionOffset.x + motion.scale.x / 2.f, 0) * rot_mat, (mouse_click_pos - mouse_cur_pos) * GRENADE_SPEED_FACTOR);
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

void update_grenades(RenderSystem* renderer, float elapsed_ms) {
	for (Entity grenade: registry.grenades.entities) {
		float& explode_timer = registry.grenades.get(grenade).explode_timer;
		explode_timer -= elapsed_ms;
		if (explode_timer <= 0) {
			explode(renderer, registry.motions.get(grenade).position, grenade);
			registry.remove_all_components_of(grenade);
		}
	}
}

void explode(RenderSystem* renderer, vec2 position, Entity explodable) {
	play_sound(SOUND_EFFECT::EXPLOSION);
	if (registry.rockets.has(explodable))
		createExplosion(renderer, position, ROCKET_EXPLOSION_FACTOR);
	else if (registry.grenades.has(explodable))
		createExplosion(renderer, position, GRENADE_EXPLOSION_FACTOR);
}

void update_explosions(float elapsed_ms) {
	for (Entity explosion: registry.explosions.entities) {
		int frame = (int)floor((glfwGetTime() - registry.animated.get(explosion).oneTimer) * 10.0);
		if (frame == 6)
			registry.remove_all_components_of(explosion);
		else if (frame == 2)
			registry.weaponHitBoxes.get(explosion).isActive = false;
	}
}

void update_weapon(RenderSystem* renderer, float elapsed_ms, Entity hero, bool mouse_clicked) {
	Entity weapon = registry.players.get(hero).weapon;
	Motion &weaponMot = registry.motions.get(weapon);
	weaponMot.position = registry.motions.get(hero).position;
	if (registry.swords.has(weapon))
		swing_sword(renderer, weapon);
	else if (registry.guns.has(weapon)) {
		Gun& gun = registry.guns.get(weapon);
		if (gun.cooldown > 0) {
			gun.cooldown -= elapsed_ms;
			if (gun.cooldown <= 640 && !gun.loaded) {
				play_sound(SOUND_EFFECT::GUN_LEVER);
				gun.loaded = true;
			}
		}
	} else if (registry.rocketLaunchers.has(weapon)) {
		RocketLauncher& launcher = registry.rocketLaunchers.get(weapon);
		if (launcher.cooldown > 0) {
			launcher.cooldown -= elapsed_ms;
			if (launcher.cooldown <= 2000 && !launcher.loaded) {
				play_sound(SOUND_EFFECT::ROCKET_LAUNCHER_RELOAD);
				launcher.loaded = true;
			}
		}
	} else if (registry.grenadeLaunchers.has(weapon)) {
		GrenadeLauncher& launcher = registry.grenadeLaunchers.get(weapon);
		if (launcher.cooldown > 0) {
			launcher.cooldown -= elapsed_ms;
			if (!launcher.loaded) {
				play_sound(SOUND_EFFECT::GRENADE_LAUNCHER_RELOAD);
				launcher.loaded = true;
			}
		} else if (mouse_click_pos != vec2({-1.f, -1.f}) && !mouse_clicked) {
			Motion launcher_motion = registry.motions.get(weapon);
			mat2 rot_mat = mat2({cos(launcher_motion.angle), -sin(launcher_motion.angle)}, {sin(launcher_motion.angle), cos(launcher_motion.angle)});
			vec2 position = launcher_motion.position + launcher_motion.positionOffset * rot_mat;
			vec2 velocity = (mouse_click_pos - mouse_cur_pos) * GRENADE_SPEED_FACTOR;
			createGrenade(renderer, position, velocity);
			play_sound(SOUND_EFFECT::GRENADE_LAUNCHER_FIRE);
			for (Entity line: launcher.trajectory)
				registry.remove_all_components_of(line);
			launcher.trajectory.clear();
			launcher.cooldown = GRENADE_COOLDOWN;
			launcher.loaded = false;
			mouse_click_pos = {-1.f, -1.f};
		} else if (mouse_click_pos != vec2({-1.f, -1.f}) && launcher.trajectory.size() > 0) {
			for (Entity line: launcher.trajectory) {
				float angle = weaponMot.angle;
				registry.motions.get(line).position = weaponMot.position + vec2(weaponMot.positionOffset.x + weaponMot.scale.x / 2.f, 0) * mat2({cos(angle), -sin(angle)}, {sin(angle), cos(angle)});
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
	if (ddl == 0)
		if (rand < 0.9)
			createSword(renderer, pos);
		else
			createGun(renderer, pos);
	else if (ddl == 1)
		if (rand < 0.2)
			createSword(renderer, pos);
		else if (rand < 0.7)
			createGun(renderer, pos);
		else
			createGrenadeLauncher(renderer, pos);
	else
		if (rand < 0.1)
			createSword(renderer, pos);
		else if (rand < 0.4)
			createGun(renderer, pos);
		else if (rand < 0.7)
			createGrenadeLauncher(renderer, pos);
		else
			createRocketLauncher(renderer, pos);
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

void do_weapon_action(RenderSystem* renderer, Entity weapon, vec2 mouse_pos) {
	if (registry.swords.has(weapon)) {
		int &swingState = registry.swords.get(weapon).swing;
		if (swingState == 0) {
			float weaponAngle = registry.motions.get(weapon).angle;
			swingState = (weaponAngle < 0 || weaponAngle > M_PI) ? 3 : 1;
			printf("");
		}
	} else if (registry.guns.has(weapon)) {
		Gun& gun = registry.guns.get(weapon);
		if (gun.cooldown <= 0) {
			createBullet(renderer, registry.motions.get(weapon).position, registry.motions.get(weapon).angle);
			play_sound(SOUND_EFFECT::BULLET_SHOOT);
			gun.cooldown = GUN_COOLDOWN;
			gun.loaded = false;
		}
	} else if (registry.rocketLaunchers.has(weapon)) {
		RocketLauncher& launcher = registry.rocketLaunchers.get(weapon);
		if (launcher.cooldown <= 0) {
			createRocket(renderer, registry.motions.get(weapon).position, registry.motions.get(weapon).angle);
			play_sound(SOUND_EFFECT::ROCKET_LAUNCHER_FIRE);
			launcher.cooldown = ROCKET_COOLDOWN;
			launcher.loaded = false;
		}
	} else if (registry.grenadeLaunchers.has(weapon)) {
		GrenadeLauncher& launcher = registry.grenadeLaunchers.get(weapon);
		if (launcher.cooldown <= 0)
			mouse_click_pos = mouse_pos;
	}
}

void use_pickaxe(Entity hero, uint direction, size_t max_jumps) {
	if (pickaxe_disable <= 0) {
		registry.motions.get(hero).velocity = {0, 0};
		registry.motions.get(hero).dir = direction ? 1 : -1;
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
	if (pickaxe_disable > 0)
		pickaxe_disable -= elapsed_ms;
}

void check_dash_boots(Entity hero, uint direction) {
	if (!registry.gravities.get(hero).dashing) {
		if (direction == dash_direction && dash_window > 0 && dash_time <= 0) {
			registry.gravities.get(hero).dashing = 1;
			registry.motions.get(hero).velocity = {(direction ? -1 : 1) * 750.f, 0.f};
			dash_time = DASH_TIME;
			play_sound(SOUND_EFFECT::DASH);
            AnimationInfo& info = registry.animated.get(hero);
            info.oneTimeState = 5;
            info.oneTimer = glfwGetTime();
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