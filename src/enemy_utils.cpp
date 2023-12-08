//
// Created by justi on 2023-12-07.
//


#include "enemy_utils.hpp"

// deal with normal eneimies' spawning and moving
void move_firelings(RenderSystem* renderer)
{
    auto &testAI_container = registry.testAIs;
    for (uint i = 0; i < testAI_container.size(); i++)
    {
        TestAI &testAI = testAI_container.components[i];
        Entity entity = testAI_container.entities[i];
        Motion &motion = registry.motions.get(entity);
        if (testAI.departFromRight && motion.position[0] < 0)
        {
            float squareFactor = rand() % 2 == 0 ? 0.0005 : -0.0005;
            int rightHeight = ENEMY_SPAWN_HEIGHT_IDLE_RANGE + rand() % (window_height_px - ENEMY_SPAWN_HEIGHT_IDLE_RANGE * 2);
            motion.position = vec2(0.0, testAI.c);
            float curveParameter = (float)(rightHeight - testAI.c - window_width_px * window_width_px * squareFactor) / window_width_px;
            testAI.departFromRight = false;
            testAI.a = (float)squareFactor;
            testAI.b = curveParameter;
            motion.dir = 1;
        }
        else if (!testAI.departFromRight && motion.position[0] > window_width_px)
        {
            float squareFactor = rand() % 2 == 0 ? 0.0005 : -0.0005;
            int rightHeight = testAI.a * window_width_px * window_width_px + testAI.b * window_width_px + testAI.c;
            int leftHeight = ENEMY_SPAWN_HEIGHT_IDLE_RANGE + rand() % (window_height_px - ENEMY_SPAWN_HEIGHT_IDLE_RANGE * 2);
            motion.position = vec2(window_width_px, rightHeight);
            float curveParameter = (float)(rightHeight - leftHeight - window_width_px * window_width_px * squareFactor) / window_width_px;
            testAI.departFromRight = true;
            testAI.a = (float)squareFactor;
            testAI.b = curveParameter;
            testAI.c = (float)leftHeight;
            motion.dir = -1;
        }
        float gradient = 2 * testAI.a * motion.position[0] + testAI.b;
        float basicFactor = sqrt(BASIC_SPEED * BASIC_SPEED / (gradient * gradient + 1));
        float direction = testAI.departFromRight ? -1.0 : 1.0;
        motion.velocity = direction * vec2(basicFactor, gradient * basicFactor);
    }
}

void move_boulder(RenderSystem* renderer) {
    for (Entity boulder: registry.boulders.entities) {
        Motion& motion = registry.motions.get(boulder);
        if (motion.velocity.x > 0) {
            motion.angle += M_PI / 64;
        } else if (motion.velocity.x < 0) {
            motion.angle -= M_PI / 64;
        }
    }
}

void move_ghouls(RenderSystem* renderer, Entity player_hero)
{
    float GHOUL_SPEED = 100.f;
    float EDGE_DISTANCE = 0.f;

    registry.motions.get(player_hero);
    for (uint i = 0; i < registry.ghouls.entities.size(); i++) {
        Entity enemy = registry.ghouls.entities[i];
        Motion& enemy_motion = registry.motions.get(enemy);
        AnimationInfo& animation = registry.animated.get(enemy);
        Ghoul& enemy_reg = registry.ghouls.get(enemy);
        //printf("Position: %f\n", enemy_motion.position.x);
        if (enemy_reg.left_x != -1.f && enemy_motion.velocity.x == 0.f && enemy_motion.velocity.y == 0.f && animation.oneTimeState == -1) {
            float direction = max(enemy_motion.position.x - enemy_reg.left_x, enemy_reg.right_x - enemy_motion.position.x);
            direction = direction / abs(direction);
            enemy_motion.velocity.x = direction * GHOUL_SPEED;
            enemy_motion.dir = (int)direction;
        }
            // Reverse direction
        else if (enemy_motion.position.x - enemy_reg.left_x <= EDGE_DISTANCE && enemy_motion.velocity.y == 0.f && enemy_motion.velocity.x != 0.f) {
            enemy_motion.velocity.x = GHOUL_SPEED;
            enemy_motion.dir = 1;
        }
        else if (enemy_reg.right_x - enemy_motion.position.x <= EDGE_DISTANCE && enemy_motion.velocity.y == 0.f && enemy_motion.velocity.x != 0.f) {
            enemy_motion.velocity.x = -1.f * GHOUL_SPEED;
            enemy_motion.dir = -1;
        }
    }
}

void move_tracer(float elapsed_ms_since_last_update, Entity player_hero)
{
    const uint PHASE_IN_STATE = 1;
    const uint PHASE_OUT_STATE = 4;

    Motion& hero_motion = registry.motions.get(player_hero);
    for (uint i = 0; i < registry.followingEnemies.entities.size(); i++) {
        Entity enemy = registry.followingEnemies.entities[i];
        Motion& enemy_motion = registry.motions.get(enemy);
        AnimationInfo& animation = registry.animated.get(enemy);
        FollowingEnemies& enemy_reg = registry.followingEnemies.get(enemy);

        enemy_reg.next_blink_time -= elapsed_ms_since_last_update;
        if (enemy_reg.next_blink_time < 0.f && enemy_reg.blinked == false)
        {
            //Time between blinks
            enemy_reg.next_blink_time = 700.f;

            //enemies.hittable = true;
            //enemy_reg.hittable = true;

            if (enemy_reg.path.size() == 0 && find_map_index(enemy_motion.position) != find_map_index(hero_motion.position)) {
                std::vector<std::vector<char>> vec = grid_vec;
                bfs_follow_start(vec, enemy_motion.position, hero_motion.position, enemy);
            }

            //Don't blink when not moving: next pos in path is same pos as current
            if (enemy_reg.path.size() != 0 && find_index_from_map(enemy_reg.path.back()) == enemy_motion.position) {
                enemy_reg.path.pop_back();
            }
            else if (enemy_reg.path.size() != 0)
            {
                animation.oneTimeState = PHASE_IN_STATE;
                animation.oneTimer = 0;
                vec2 converted_pos = find_index_from_map(enemy_reg.path.back());
                enemy_motion.dir = (converted_pos.x > enemy_motion.position.x) ? -1 : 1;
                enemy_motion.position = converted_pos;
                enemy_reg.path.pop_back();

                //Don't blink when not moving: next loop will be to re-calc the path
                if (enemy_reg.path.size() != 0) {
                    enemy_reg.blinked = true;
                }
            }
        }

        if (enemy_reg.next_blink_time < 0.0f && enemy_reg.blinked == true) {
            enemy_reg.next_blink_time = 100.f;
            animation.oneTimeState = PHASE_OUT_STATE;
            animation.oneTimer = 0;
            //enemy_reg.hittable = false;
            //enemies.hittable = false;
            enemy_reg.blinked = false;
        }
    }
}

void move_spitters(float elapsed_ms_since_last_update, RenderSystem* renderer) {
    const uint SHOOT_STATE = 2;
    const uint SPITTER_FIRE_FRAME = 4;
    const uint WALKING_TIME = 5;
    const float WALKING_SPEED = 100.f;
    float EDGE_DISTANCE = 10.f;
    const float STOP_WALK_TIME = 300.f;

    auto &spitterEnemy_container = registry.spitterEnemies;
    for (uint i = 0; i < spitterEnemy_container.size(); i++)
    {
        SpitterEnemy &spitterEnemy = spitterEnemy_container.components[i];
        spitterEnemy.timeUntilNextShotMs -= elapsed_ms_since_last_update;
        Entity entity = spitterEnemy_container.entities[i];
        Motion &motion = registry.motions.get(entity);
        AnimationInfo &animation = registry.animated.get(entity);

        if (!spitterEnemy.canShoot && spitterEnemy.timeUntilNextShotMs > STOP_WALK_TIME && motion.velocity.y == 0.f && animation.oneTimeState != 2) {
            if (spitterEnemy.left_x != -1.f && motion.velocity.x == 0.f) {
                float direction;
                if (motion.position.x <= spitterEnemy.left_x || motion.position.x >= spitterEnemy.right_x) {
                    direction = max(motion.position.x - spitterEnemy.left_x, spitterEnemy.right_x - motion.position.x);
                }
                else {
                    direction = (rand() % 2) - 0.5f;
                }

                direction = direction / abs(direction);
                motion.velocity.x = direction * WALKING_SPEED;
                motion.dir = (int)direction;
            }
                // Reverse direction
            else if (motion.position.x - spitterEnemy.left_x <= EDGE_DISTANCE && motion.velocity.x != 0.f) {
                motion.velocity.x = WALKING_SPEED;
                motion.dir = 1;
            }
            else if (spitterEnemy.right_x - motion.position.x <= EDGE_DISTANCE && motion.velocity.x != 0.f) {
                motion.velocity.x = -1.f * WALKING_SPEED;
                motion.dir = -1;
            }
        }
        else if (spitterEnemy.canShoot || spitterEnemy.timeUntilNextShotMs <= STOP_WALK_TIME) {
            motion.velocity.x = 0;
        }

        animation.curState = (motion.velocity.x != 0)? 1: 0;

        if (animation.oneTimeState == SHOOT_STATE && (int)floor(animation.oneTimer * ANIMATION_SPEED_FACTOR) == SPITTER_FIRE_FRAME && spitterEnemy.canShoot) {
            Entity spitterBullet = createSpitterEnemyBullet(renderer, motion.position, motion.angle);
            float absolute_scale_x = abs(registry.motions.get(entity).scale[0]);
            if (registry.motions.get(spitterBullet).velocity[0] < 0.0f)
                registry.motions.get(entity).scale[0] = -absolute_scale_x;
            else
                registry.motions.get(entity).scale[0] = absolute_scale_x;
            spitterEnemy.canShoot = false;
        }
        if (spitterEnemy.timeUntilNextShotMs <= 0.f && registry.enemies.get(entity).hitting == true)
        {
            // attack animation
            animation.oneTimeState = SHOOT_STATE;
            animation.oneTimer = 0;
            spitterEnemy.canShoot = true;
            // create bullet at same position as enemy
            spitterEnemy.timeUntilNextShotMs = spitter_projectile_delay_ms;
        }
    }

    // decay spitter bullets
    auto& spitterBullets_container = registry.spitterBullets;
    for (uint i = 0; i < spitterBullets_container.size(); i++)
    {
        SpitterBullet& spitterBullet = spitterBullets_container.components[i];
        Entity entity = spitterBullets_container.entities[i];
        RenderRequest& render = registry.renderRequests.get(entity);
        Motion& motion = registry.motions.get(entity);
        // make bullets smaller over time
        motion.scale = vec2(motion.scale.x / spitterBullet.mass, motion.scale.y / spitterBullet.mass);
        spitterBullet.mass -= elapsed_ms_since_last_update / SPITTER_PROJECTILE_REDUCTION_FACTOR;
        motion.scale = vec2(motion.scale.x * spitterBullet.mass, motion.scale.y * spitterBullet.mass);
        render.scale = motion.scale;
        if (spitterBullet.mass <= SPITTER_PROJECTILE_MIN_SIZE)
        {
            spitterBullet.mass = 0;
            registry.remove_all_components_of(entity);
        }
    }
}

void boss_action_decision(Entity boss, RenderSystem* renderer){
    Boss& boss_state = registry.boss.get(boss);
    AnimationInfo& info = registry.animated.get(boss);
    // 11 and 12 are hurt and death animation
    if (info.oneTimeState > 10) {
        boss_state.phase = 0;
        boss_state.state = BOSS_STATE::SIZE;
        for (auto hurt_box : boss_state.hurt_boxes) {
            registry.weaponHitBoxes.get(hurt_box).isActive = false;
        }
        return;
    }
    switch (boss_state.state) {
        case BOSS_STATE::TELEPORT:
            boss_action_teleport(boss);
            break;
        case BOSS_STATE::SWIPE:
            boss_action_swipe(boss);
            break;
        case BOSS_STATE::SUMMON:
            boss_action_summon(boss, renderer);
            break;
        case BOSS_STATE::SIZE:
            boss_state.state = static_cast<BOSS_STATE>(rand() % ((int)BOSS_STATE::SIZE));
            break;
    }
}

void boss_action_teleport(Entity boss){
    const int PHASE_OUT = 8;
    const int PHASE_IN = 9;
    const std::vector<int> boss_platforms{0,1,2,7};
    AnimationInfo& info = registry.animated.get(boss);
    Boss& boss_state = registry.boss.get(boss);
    Enemies& enemy_info = registry.enemies.get(boss);
    if (boss_state.phase == 0) {
        enemy_info.hitting = false;
        enemy_info.hittable = false;
        info.oneTimeState = PHASE_OUT;
        boss_state.phase++;
    } else if(boss_state.phase == 1 && info.oneTimeState == -1) {
        Motion& motion = registry.motions.get(boss);
        motion.position = getRandomWalkablePos(motion.scale, boss_platforms[rand() % boss_platforms.size()], false);
        boss_state.phase++;
    } else if(boss_state.phase == 2) {
        enemy_info.hittable = true;
        info.oneTimeState = PHASE_IN;
        boss_state.phase++;
    } else if(boss_state.phase == 3 && info.oneTimeState == -1) {
        enemy_info.hitting = true;
        boss_state.phase = 0;
        boss_state.state = BOSS_STATE::SIZE;
    }
}

void boss_action_swipe(Entity boss){
    const int SWIPE = 1;
    const int STAND_UP = 10;
    Boss& boss_state = registry.boss.get(boss);
    AnimationInfo& info = registry.animated.get(boss);
    if (boss_state.phase == 0) {
        info.oneTimeState = SWIPE;
        registry.motions.get(boss_state.hurt_boxes[0]).position = registry.motions.get(boss).position + vec2(0,55);
        registry.motions.get(boss_state.hurt_boxes[1]).position = registry.motions.get(boss).position + vec2(0,15);
        boss_state.phase++;
    } else if (boss_state.phase == 1 && info.oneTimeState != -1) {
        Motion& motion = registry.motions.get(boss);
        int frame = (int)floor(info.oneTimer * ANIMATION_SPEED_FACTOR);
        if (frame == 1) {
            registry.weaponHitBoxes.get(boss_state.hurt_boxes[0]).isActive = true;
        } else if (frame == 3) {
            registry.weaponHitBoxes.get(boss_state.hurt_boxes[1]).isActive = true;
        } else {
            for (auto hurt_box : boss_state.hurt_boxes) {
                registry.weaponHitBoxes.get(hurt_box).isActive = false;
            }
        }
    } else if (boss_state.phase == 1 && info.oneTimeState == -1) {
        info.oneTimeState = STAND_UP;
        boss_state.phase++;
    } else if (boss_state.phase == 2 && info.oneTimeState == -1) {
        boss_state.phase = 0;
        boss_state.state = BOSS_STATE::SIZE;
    }
}

void boss_action_summon(Entity boss, RenderSystem* renderer){
    const int SUMMON = 6;
    const int STAND_UP = 10;
    AnimationInfo& info = registry.animated.get(boss);
    Boss& boss_state = registry.boss.get(boss);
    if (boss_state.phase == 0) {
        info.oneTimeState = SUMMON;
        boss_state.phase++;
    } else if(boss_state.phase == 1 && info.oneTimeState == -1) {
        info.oneTimeState = STAND_UP;
        switch (rand() % 2) {
            case 0:
                for(int i = 0; i < 3 + rand() % 4; i++) {
                    createGhoul(renderer, getRandomWalkablePos(ASSET_SIZE.at(TEXTURE_ASSET_ID::GHOUL_ENEMY)));
                }
                for(int i = 0; i < 1 + rand() % 3; i++) {
                    createSpitterEnemy(renderer, getRandomWalkablePos(ASSET_SIZE.at(TEXTURE_ASSET_ID::SPITTER_ENEMY)));
                }
                break;
            case 1:
                for(int i = 0; i < 10 + rand() % 6; i++) {
                    Motion& motion = registry.motions.get(boss);
                    createSpitterEnemyBullet(renderer, motion.position, motion.angle);
                }
                break;
        }
        boss_state.phase++;
    } else if(boss_state.phase == 2 && info.oneTimeState == -1) {
        boss_state.phase = 0;
        boss_state.state = BOSS_STATE::SIZE;
    }

}
