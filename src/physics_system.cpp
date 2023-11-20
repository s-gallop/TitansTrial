// internal
#include <iostream>
#include "physics_system.hpp"
#include "world_init.hpp"

const float COLLISION_THRESHOLD = 0.0f;

void PhysicsSystem::init(RenderSystem* renderer_arg) {
    this->renderer = renderer_arg;
}

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion &motion)
{
    // abs is to avoid negative scale due to the facing direction.
    return {abs(motion.scale.x), abs(motion.scale.y)};
}

bool check_collision_conditions(Entity entity_i, Entity entity_j) {
    if (registry.players.has(entity_i)) {
        if ((registry.enemies.has(entity_j) || 
            registry.spitterBullets.has(entity_j) || 
            registry.spitterEnemies.has(entity_j) || 
            registry.explosions.has(entity_j) ||
            registry.collectables.has(entity_j)))
        {
            return true;
        }
    } else if (registry.weaponHitBoxes.has(entity_i)) {
        if (registry.enemies.has(entity_j) || registry.spitterEnemies.has(entity_j) || registry.blocks.has(entity_j))
        {
            return true;
        }
    } else if (registry.blocks.has(entity_i)) {
        if (registry.solids.has(entity_j) || registry.projectiles.has(entity_j)) {
            return true;
        } 
    } else if (registry.parallaxBackgrounds.has(entity_i)) {
        if (registry.bullets.has(entity_j) ||
            registry.rockets.has(entity_j) ||
            registry.grenades.has(entity_j) ||
            registry.spitterBullets.has(entity_j) ||
            registry.collectables.has(entity_j) ||
            registry.players.has(entity_j)) 
        {
            return true;
        }
    }
    return false;
}

vec2 get_parametrics(vec2 p1, vec2 c1, vec2 p2, vec2 c2) {
    vec2 t;
    if (c1.x == 0) {
        t.y = (p1.x - p2.x + c1.x * p2.y / c1.y - c1.x * p1.y / c1.y) / (c2.x - c1.x * c2.y / c1.y);
        t.x = (p2.y - p1.y + t.y*c2.y) / c1.y;
    } else {
        t.y = (p1.y - p2.y + c1.y * p2.x / c1.x - c1.y * p1.x / c1.x) / (c2.y - c1.y * c2.x / c1.x);
        t.x = (p2.x - p1.x + t.y*c2.x) / c1.x;
    }
    return t;
}

bool check_intersection(vec2 p1, vec2 q1, vec2 p2, vec2 q2) {
    vec2 c1 = q1 - p1;
    vec2 c2 = q2 - p2;
    if (c1.x * c2.y == c1.y * c2.x)
        return false;

    vec2 t = get_parametrics(p1, c1, p2, c2);

    if (t.x < 0 || t.x > 1 || t.y < 0 || t.y > 1)
        return false;
    return true;
}

bool check_inside(vec2 p1, vec2 q1, vec2 p2, vec2 q2) {
    vec2 c1 = q1 - p1;
    vec2 c2 = q2 - p2;
    if (c1.x * c2.y == c1.y * c2.x)
        return false;

    vec2 t = get_parametrics(p1, c1, p2, c2);

    if (t.x >= 0 && t.x <= 1 && t.y > 1)
        return true;
    return false;
}

bool precise_collision(const Entity& entity1, const Entity& entity2) {
    Motion& motion1 = registry.motions.get(entity1);
    Motion& motion2 = registry.motions.get(entity2);
    CollisionMesh* mesh1 = registry.collisionMeshPtrs.get(entity1);
    CollisionMesh* mesh2 = registry.collisionMeshPtrs.get(entity2);
    
    std::vector<ColoredVertex> vertices1 = mesh1->vertices;
    mat2 rotation_matrix1 = mat2({cos(motion1.angle), -sin(motion1.angle)}, {sin(motion1.angle), cos(motion1.angle)});
    for (ColoredVertex vertex: vertices1) {
        vertex.position = vec3(motion1.position + (motion1.positionOffset + vec2(vertex.position.x * motion1.scale.x, vertex.position.y * motion1.scale.y)) * rotation_matrix1, 0);
    }
    
    std::vector<ColoredVertex> vertices2 = mesh2->vertices;
    mat2 rotation_matrix2 = mat2({cos(motion2.angle), -sin(motion2.angle)}, {sin(motion2.angle), cos(motion2.angle)});
    for (ColoredVertex vertex: vertices2) {
        vertex.position = vec3(motion2.position + (motion2.positionOffset + vec2(vertex.position.x * motion2.scale.x, vertex.position.y * motion2.scale.y)) * rotation_matrix2, 0);
    }

    for (std::pair<int, int> edge1: mesh1->edges) {
        for (std::pair<int, int> edge2: mesh2->edges) {
            if (check_intersection(vertices1[edge1.first - 1].position, vertices1[edge1.second - 1].position, vertices2[edge2.first - 1].position, vertices2[edge2.second - 1].position))
                return true;
        }
        if (check_inside(vertices1[edge1.first - 1].position, vertices1[edge1.second - 1].position, motion1.position, motion2.position))
            return true;
    }

    for (std::pair<int, int> edge2: mesh2->edges) {
        if (check_inside(vertices2[edge2.first - 1].position, vertices2[edge2.second - 1].position, motion2.position, motion1.position))
            return true;
    }

    return false;
}

// assumes at least one collider is box shaped

bool PhysicsSystem::collides(const Entity &entity1, const Entity &entity2)
{
    Motion& motion1 = registry.motions.get(entity1);
    Motion& motion2 = registry.motions.get(entity2);
    vec2 scale1 = get_bounding_box(motion1) / 2.0f;
    vec2 scale2 = get_bounding_box(motion2) / 2.0f;
    if (abs(motion1.position.x - motion2.position.x) < (scale1.x + scale2.x) &&
        abs(motion1.position.y - motion2.position.y) < (scale1.y + scale2.y))
    {
        if (registry.renderRequests.get(entity1).used_geometry != GEOMETRY_BUFFER_ID::SPRITE || registry.renderRequests.get(entity2).used_geometry != GEOMETRY_BUFFER_ID::SPRITE)
            return precise_collision(entity1, entity2);
        else
            return true;
    }
    return false;

    // Motion& motion1 = registry.motions.get(entity1);
    // Motion& motion2 = registry.motions.get(entity2);
    // vec2 scale1 = get_bounding_box(motion1) / 2.0f;
    // vec2 scale2 = get_bounding_box(motion2) / 2.0f;
    // CollisionMesh* mesh1 = registry.collisionMeshPtrs.get(entity1);
    // CollisionMesh* mesh2 = registry.collisionMeshPtrs.get(entity2);
    // if (registry.renderRequests.get(entity1).used_geometry != GEOMETRY_BUFFER_ID::SPRITE) {
    //     for (ColoredVertex v: mesh1->vertices) {
    //         if (abs(motion1.position.x + motion1.scale.x * v.position.x - motion2.position.x) < scale2.x && 
    //             abs(motion1.position.y + motion1.scale.y * v.position.y - motion2.position.y) < scale2.y)
    //         {
    //             return true;
    //         }
    //     }
    // } else if (registry.renderRequests.get(entity2).used_geometry != GEOMETRY_BUFFER_ID::SPRITE) {
    //     for (ColoredVertex v: mesh2->vertices) {
    //         if (abs(motion2.position.x + motion2.scale.x * v.position.x - motion1.position.x) < scale1.x && 
    //             abs(motion2.position.y + motion2.scale.y * v.position.y - motion1.position.y) < scale1.y)
    //         {
    //             return true;
    //         }
    //     }
    // } else {
    //     if (abs(motion1.position.x - motion2.position.x) < (scale1.x + scale2.x) &&
    //         abs(motion1.position.y - motion2.position.y) < (scale1.y + scale2.y))
    //     {
    //         return true;
    //     }
    // }
    // return false;
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
                motion.velocity[1] += GRAVITY_ACCELERATION_FACTOR * elapsed_ms;
        }
        motion.position += motion.velocity * step_seconds;
    }

    // Check for collisions between all entities with meshes
    for (uint i = 0; i < registry.collisionMeshPtrs.size(); i++) {
        for (uint j = i + 1; j < registry.collisionMeshPtrs.size(); j++) {
            Entity entity_i = registry.collisionMeshPtrs.entities[i];
            Entity entity_j = registry.collisionMeshPtrs.entities[j];            
            if ((check_collision_conditions(entity_i, entity_j) || check_collision_conditions(entity_j, entity_i)) && PhysicsSystem::collides(entity_i, entity_j)) {
                registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                registry.collisions.emplace_with_duplicates(entity_j, entity_i);
            }
        }
    }
}