// internal
#include <iostream>
#include "physics_system.hpp"
#include "world_init.hpp"

const float GRAVITY_ACCELERATION_FACTOR = 10.0;
const float COLLISION_THRESHOLD = 0.0f;
// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// assumes that the colliders are box shaped

bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 scale1 = get_bounding_box(motion1) / 2.0f;
	vec2 scale2 = get_bounding_box(motion2) / 2.0f;
	if (abs(motion1.position.x - motion2.position.x) < (scale1.x + scale2.x) && 
		abs(motion1.position.y - motion2.position.y) < (scale1.y + scale2.y)) {
		return true;
	}
	return false;
}



void PhysicsSystem::step(float elapsed_ms)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_container = registry.motions;
	for(uint i = 0; i < motion_container.size(); i++)
	{
		Motion& motion = motion_container.components[i];
		Entity entity = motion_container.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
        if (registry.gravities.has(entity)) {
            motion.velocity[1] += GRAVITY_ACCELERATION_FACTOR;
        }
		motion.position += motion.velocity * step_seconds;
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE PEBBLE UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Check for collisions between all moving entities
	for(uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				if (motion_i.isSolid && motion_j.isSolid) {
					
					vec2 scale1 = get_bounding_box(motion_i) / 2.0f;
					vec2 scale2 = get_bounding_box(motion_j) / 2.0f;
					if (abs(motion_i.position.y - motion_j.position.y) < (scale1.y + scale2.y) + COLLISION_THRESHOLD) {
						
						if (motion_i.velocity.x > motion_j.velocity.x && motion_i.position.x < motion_j.position.x - scale2.x) {

							motion_i.position.x = motion_j.position.x - scale2.x - scale1.x;
							motion_j.position.x = motion_i.position.x + scale2.x + scale1.x;
							
						}
						else if (motion_i.velocity.x < motion_j.velocity.x && motion_i.position.x > motion_j.position.x + scale2.x) {
							motion_i.position.x = motion_j.position.x + scale2.x + scale1.x;
							motion_j.position.x = motion_i.position.x - scale2.x - scale1.x;
						}
						
						
					}
					if (abs(motion_i.position.x - motion_j.position.x) < (scale1.x + scale2.x) + COLLISION_THRESHOLD) {

						if (motion_i.velocity.y > motion_j.velocity.y && motion_i.position.y < motion_j.position.y - scale2.y) {

							motion_i.position.y = motion_j.position.y - scale2.y - scale1.y;
							motion_j.position.y = motion_i.position.y + scale2.y + scale1.y;
							motion_i.velocity.y = 0;
							motion_j.velocity.y = 0;
						}
						else if (motion_i.velocity.y < motion_j.velocity.y && motion_i.position.y > motion_j.position.y + scale2.y) {
							motion_i.position.y = motion_j.position.y + scale2.y + scale1.y;
							motion_j.position.y = motion_i.position.y - scale2.y - scale1.y;
							motion_i.velocity.y = 0;
							motion_j.velocity.y = 0;
						}
						

					}
					
					
					
				}
				
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE PEBBLE collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}