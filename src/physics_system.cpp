// internal
#include <iostream>
#include "physics_system.hpp"
#include "world_init.hpp"

const float GRAVITY_ACCELERATION_FACTOR = 10.0;
const float COLLISION_THRESHOLD = 0.0f;
// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion &motion)
{
    // abs is to avoid negative scale due to the facing direction.
    return {abs(motion.scale.x), abs(motion.scale.y)};
}

// assumes at least one collider is box shaped

bool collides(const Entity &entity1, const Entity &entity2)
{
    Motion& motion1 = registry.motions.get(entity1);
    Motion& motion2 = registry.motions.get(entity2);
    vec2 scale1 = get_bounding_box(motion1) / 2.0f;
    vec2 scale2 = get_bounding_box(motion2) / 2.0f;
    Mesh* mesh1 = registry.meshPtrs.get(entity1);
    Mesh* mesh2 = registry.meshPtrs.get(entity2);
    if (!mesh1->vertices.empty()) {
        for (ColoredVertex v: registry.meshPtrs.get(entity1)->vertices) {
            if (abs(motion1.position.x + motion1.scale.x * v.position.x - motion2.position.x) < scale2.x && 
                abs(motion1.position.y + motion1.scale.y * v.position.y - motion2.position.y) < scale2.y)
            {
                return true;
            }
        }
    } else if (!mesh2->vertices.empty()) {
        for (ColoredVertex v: registry.meshPtrs.get(entity2)->vertices) {
            if (abs(motion2.position.x + motion2.scale.x * v.position.x - motion1.position.x) < scale1.x && 
                abs(motion2.position.y + motion2.scale.y * v.position.y - motion1.position.y) < scale1.y)
            {
                return true;
            }
        }
    } else {
        if (abs(motion1.position.x - motion2.position.x) < (scale1.x + scale2.x) &&
            abs(motion1.position.y - motion2.position.y) < (scale1.y + scale2.y))
        {
            return true;
        }
    }
    return false;
}

// vec2 getVecToOther(float angle1To2, vec2 scale1i, vec2 scale1j) {
// 	float angleToCorner = atan(length(scale1j) / length(scale1i));
// 	if (abs(angle1To2) <= angleToCorner || abs(angle1To2) >= M_PI - angleToCorner) {
// 		return (abs(angle1To2) < M_PI/2 ? -1.f : 1.f) * scale1i + sin(angle1To2) * scale1j;
// 	} else {
// 		return cos(angle1To2) * scale1i + (angle1To2 < 0 ? -1.f : 1.f) * scale1j;
// 	}
// }

// vec2 getClosestCorner(float angle, vec2 scaleI, vec2 scaleJ) {
// 	return (abs(angle) < M_PI/2 ? -1.f : 1.f) * scaleI + (angle < 0 ? -1.f : 1.f) * scaleJ;
// }

// bool collides (const Motion& motion1, const Motion& motion2) {
// 	float angle1 = motion1.angle;
// 	float angle2 = motion2.angle;
// 	mat2 rotMat1 = {cos(angle1), -sin(angle1), sin(angle1), cos(angle1)};
// 	mat2 rotMat2 = {cos(angle2), -sin(angle2), sin(angle2), cos(angle2)};
// 	vec2 scale1i = vec2(get_bounding_box(motion1).x, 0) * rotMat1 / 2.f;
// 	vec2 scale1j = vec2(0, get_bounding_box(motion1).y) * rotMat1 / 2.f;
//  	vec2 scale2i = vec2(get_bounding_box(motion2).x, 0) * rotMat2 / 2.f;
// 	vec2 scale2j = vec2(0, get_bounding_box(motion2).y) * rotMat2 / 2.f;
// 	vec2 pos1 = motion1.position + motion1.positionOffset * rotMat1;
// 	vec2 pos2 = motion2.position + motion2.positionOffset * rotMat2;
// 	vec2 vec1To2 = pos2 - pos1;
// 	vec2 vec2To1 = pos1 - pos2;
// 	float angle1To2 = atan(dot(vec1To2, vec2(-scale1i.y, scale1i.x)) / dot(vec1To2, scale1i));
// 	float angle2To1 = atan(dot(vec2To1, vec2(-scale2i.y, scale2i.x)) / dot(vec2To1, scale2i));
// 	vec2 box1To2 = getVecToOther(angle1To2, scale1i, scale1j);
// 	vec2 box2To1 = getVecToOther(angle2To1, scale2i, scale2j);
// 	if (length(vec1To2) < length(box1To2) + length(box2To1))
// 		return true;

// 	vec2 closestCorner1 = pos1 + getClosestCorner(angle1To2, scale1i, scale1j);
// 	vec2 closestCorner2 = pos2 + getClosestCorner(angle2To1, scale2i, scale2j);
// 	vec2 pos1ToCorner2 = closestCorner2 - pos1;
// 	vec2 pos2ToCorner1 = closestCorner1 - pos2;
// 	float angle1ToCorner2 = atan(dot(pos1ToCorner2, vec2(-scale1i.y, scale1i.x)) / dot(pos1ToCorner2, scale1i));
// 	float angle2ToCorner1 = atan(dot(pos2ToCorner1, vec2(-scale2i.y, scale2i.x)) / dot(pos2ToCorner1, scale2i));
// 	vec2 box1ToCorner2 = getVecToOther(angle1ToCorner2, scale1i, scale1j);
// 	vec2 box2ToCorner1 = getVecToOther(angle2ToCorner1, scale2i, scale2j);
// 	if (length(pos1ToCorner2) < length(box1ToCorner2) || length(pos2ToCorner1) < length(box2ToCorner1))
// 		return true;

// 	return false;
// }

void PhysicsSystem::step(float elapsed_ms)
{
    // Move fish based on how much time has passed, this is to (partially) avoid
    // having entities move at different speed based on the machine.
    auto &motion_container = registry.motions;
    for (uint i = 0; i < motion_container.size(); i++)
    {
        Motion &motion = motion_container.components[i];
        Entity entity = motion_container.entities[i];
        float step_seconds = elapsed_ms / 1000.f;
        if (registry.gravities.has(entity))
        {
            Gravity& gravity = registry.gravities.get(entity);
            if (!gravity.lodged.test(0) && !gravity.lodged.test(1) && !gravity.dashing)
                motion.velocity[1] += GRAVITY_ACCELERATION_FACTOR * elapsed_ms / 17.5;
        }
        motion.position += motion.velocity * step_seconds;
    }

    // Check for collisions between all moving entities
    for (uint i = 0; i < motion_container.components.size(); i++)
    {
        Motion &motion_i = motion_container.components[i];
        Entity entity_i = motion_container.entities[i];

        // note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
        for (uint j = i + 1; j < motion_container.components.size(); j++)
        {
            Motion &motion_j = motion_container.components[j];
            Entity entity_j = motion_container.entities[j];

            if (collides(entity_i, entity_j))
            {
                if (motion_i.isSolid && motion_j.isSolid)
                {

                    vec2 scale1 = get_bounding_box(motion_i) / 2.0f;
                    vec2 scale2 = get_bounding_box(motion_j) / 2.0f;
                    float vCollisionDepth = (scale1.y + scale2.y) - abs(motion_i.position.y - motion_j.position.y) + COLLISION_THRESHOLD;
                    float hCollisionDepth = (scale1.x + scale2.x) - abs(motion_i.position.x - motion_j.position.x) + COLLISION_THRESHOLD;
                    if (vCollisionDepth > 0 && hCollisionDepth > 0)
                    {
                        if (vCollisionDepth < hCollisionDepth)
                        {
                            if (motion_i.velocity.y > motion_j.velocity.y && motion_i.position.y < motion_j.position.y - scale2.y)
                            {

                                motion_i.position.y = motion_j.position.y - scale2.y - scale1.y;
                                motion_j.position.y = motion_i.position.y + scale2.y + scale1.y;
                                motion_i.velocity.y = 0;
                                motion_j.velocity.y = 0;
                            }
                            else if (motion_i.velocity.y < motion_j.velocity.y && motion_i.position.y > motion_j.position.y + scale2.y)
                            {
                                motion_i.position.y = motion_j.position.y + scale2.y + scale1.y;
                                motion_j.position.y = motion_i.position.y - scale2.y - scale1.y;
                                motion_i.velocity.y = 0;
                                motion_j.velocity.y = 0;
                            }
                        }
                        else
                        {
                            if (motion_i.velocity.x > motion_j.velocity.x && motion_i.position.x < motion_j.position.x - scale2.x)
                            {

                                motion_i.position.x = motion_j.position.x - scale2.x - scale1.x;
                                motion_j.position.x = motion_i.position.x + scale2.x + scale1.x;
                            }
                            else if (motion_i.velocity.x < motion_j.velocity.x && motion_i.position.x > motion_j.position.x + scale2.x)
                            {
                                motion_i.position.x = motion_j.position.x + scale2.x + scale1.x;
                                motion_j.position.x = motion_i.position.x - scale2.x - scale1.x;
                            }
                        }
                    }
                    else if (vCollisionDepth > 0)
                    {

                        if (motion_i.velocity.y > motion_j.velocity.y && motion_i.position.y < motion_j.position.y - scale2.y)
                        {

                            motion_i.position.y = motion_j.position.y - scale2.y - scale1.y;
                            motion_j.position.y = motion_i.position.y + scale2.y + scale1.y;
                            motion_i.velocity.y = 0;
                            motion_j.velocity.y = 0;
                        }
                        else if (motion_i.velocity.y < motion_j.velocity.y && motion_i.position.y > motion_j.position.y + scale2.y)
                        {
                            motion_i.position.y = motion_j.position.y + scale2.y + scale1.y;
                            motion_j.position.y = motion_i.position.y - scale2.y - scale1.y;
                            motion_i.velocity.y = 0;
                            motion_j.velocity.y = 0;
                        }
                    }
                    else if (hCollisionDepth > 0)
                    {

                        if (motion_i.velocity.x > motion_j.velocity.x && motion_i.position.x < motion_j.position.x - scale2.x)
                        {

                            motion_i.position.x = motion_j.position.x - scale2.x - scale1.x;
                            motion_j.position.x = motion_i.position.x + scale2.x + scale1.x;
                        }
                        else if (motion_i.velocity.x < motion_j.velocity.x && motion_i.position.x > motion_j.position.x + scale2.x)
                        {
                            motion_i.position.x = motion_j.position.x + scale2.x + scale1.x;
                            motion_j.position.x = motion_i.position.x - scale2.x - scale1.x;
                        }
                    }
                }

                else if ((motion_i.isProjectile && motion_j.isSolid) || (motion_j.isProjectile && motion_i.isSolid))
                {
                    vec2 scale1 = get_bounding_box(motion_i) / 2.0f;
                    vec2 scale2 = get_bounding_box(motion_j) / 2.0f;

                    float vCollisionDepth = (scale1.y + scale2.y) - abs(motion_i.position.y - motion_j.position.y) + COLLISION_THRESHOLD;
                    float hCollisionDepth = (scale1.x + scale2.x) - abs(motion_i.position.x - motion_j.position.x) + COLLISION_THRESHOLD;

                    if (vCollisionDepth > 0 && hCollisionDepth > 0)
                    {
                        if (vCollisionDepth < hCollisionDepth)
                        {
                            // Vertical collision
                            if (motion_i.isProjectile)
                            {
                                motion_i.velocity.y = -motion_i.velocity.y;
                                motion_i.position.y += vCollisionDepth * (motion_i.position.y < motion_j.position.y ? 1 : -1);
                            }
                            else
                            {
                                motion_j.velocity.y = -motion_j.velocity.y;
                                motion_j.position.y += vCollisionDepth * (motion_i.position.y < motion_j.position.y ? 1 : -1);
                            }
                        }
                        else
                        {
                            // Horizontal collision
                            if (motion_i.isProjectile)
                            {
                                motion_i.velocity.x = -motion_i.velocity.x;
                                motion_i.position.x += hCollisionDepth * (motion_i.position.x < motion_j.position.x ? 1 : -1);
                            }
                            else
                            {
                                motion_j.velocity.x = -motion_j.velocity.x;
                                motion_j.position.x += hCollisionDepth * (motion_i.position.x < motion_j.position.x ? 1 : -1);
                            }
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
}