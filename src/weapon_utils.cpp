// internal
#include "weapon_utils.hpp"
#include "world_init.hpp"
#include "sound_utils.hpp"

float next_collectable_spawn = 1000.f;
const size_t COLLECTABLE_DELAY_MS = 8000 * 3;
const size_t MAX_COLLECTABLES = 3;
std::default_random_engine rng = std::default_random_engine(std::random_device()());
std::uniform_real_distribution<float> uniform_dist;

void rotate_weapon(Entity weapon, vec2 mouse_pos) {
	if (!registry.swords.has(weapon) || registry.swords.get(weapon).swing == 0) {
		Motion &motion = registry.motions.get(weapon);
		motion.angle = atan2(mouse_pos.y - motion.position.y, mouse_pos.x - motion.position.x) + M_PI / 2;
		motion.angleBackup = motion.angle;
	}
}

void swing_sword(Entity weapon) {
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
			registry.weapons.get(weapon).hitBoxes.push_back(createWeaponHitBox(hitBoxPos, {hbScale, hbScale}));
			if (!registry.weaponHitBoxes.get(registry.weapons.get(weapon).hitBoxes.front()).soundPlayed) {
				registry.weaponHitBoxes.get(registry.weapons.get(weapon).hitBoxes.front()).soundPlayed = true;
				play_sound(SOUND_EFFECT::SWORD_SWING);
			}
		}
	}
}

void update_weapon_pos(Entity weapon, Entity hero) {
	Motion &weaponMot = registry.motions.get(weapon);
	weaponMot.position = registry.motions.get(hero).position;
	if (registry.swords.has(weapon))
		swing_sword(weapon);
}

float spawn_weapon(RenderSystem* renderer) {
	float sword_x = uniform_dist(rng) * (window_width_px - 120) + 60;
	float sword_y = uniform_dist(rng) * (window_height_px - 350) + 50;

	createSword(renderer, {sword_x, sword_y});

	return (COLLECTABLE_DELAY_MS / 2) + uniform_dist(rng) * (COLLECTABLE_DELAY_MS / 2);
}

void update_weapon_timer(float elapsed_ms, RenderSystem* renderer) {
	next_collectable_spawn -= elapsed_ms;
	if (registry.collectables.components.size() <= MAX_COLLECTABLES && next_collectable_spawn < 0.f)
		next_collectable_spawn = spawn_weapon(renderer);
}

void do_weapon_action(Entity weapon) {
	if (registry.swords.has(weapon)) {
		int &swingState = registry.swords.get(weapon).swing;
		if (swingState == 0) {
			float weaponAngle = registry.motions.get(weapon).angle;
			swingState = (weaponAngle < 0 || weaponAngle > M_PI) ? 3 : 1;
		}
	}
}