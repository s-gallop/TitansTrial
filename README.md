# Team17-DefiantMortals-TitansTrial

Titan's Trial - A 2D Endless survival game

## Write-up Milestone 1

There were few changes that we had to implement to our previous proposal, all changes can be seen highlighted in green in the M1_Proposal_V2 document. The changes include:

* Clarification on enemy - player collision
* Early implementations
  * Mouse + space for weapon use forgoing arrow keys
  * Some improved movement options (double jump + wall jump)
    * To allow better traversal of the level
  * Rudementary scoring system
* Added creative elements
  * Player movement animations
  * Audio Feedback

## Features

* Map asset
  * data/textures/background.png
  * world_init.cpp :
    * func createBackground
* Collision with platform and Boundary
  * physics_system.cpp :
    * func collides
* Map collision placement
  * world_init.cpp :
    * function createBlock
  * world_system.cpp :
    * func restart_game - lines 410-440
  * render_system.cpp :
    * func drawTexturedMesh - lines 78-88
* Player movement
  * world_system.cpp :
    * func on_key - lines 552-589
    * func motion_helper
* Player gravity
  * physics_system.cpp :
    * func step - lines 86-89
* Double Jump + Wall Jump
  * world_system.cpp:
    * func on_key - lines 581-589
    * func handle_collisions - lines 490-498
* Random Movement enemy
  * world_init.cpp :
    * func createEnemy
  * world_system.cpp :
    * func step - lines 283-333
* Random sword spawn
  * world_system.cpp :
    * func step - lines 335-345
    * func handle_collisions - lines 500-517
* Sword and collision with enemy
  * world_system.cpp :
    * func on_key - lines 591-602
    * func handle_collisions - lines 518-528
    * func on_mouse_move
    * func step - lines 200-263
* Sprite Animation
  * render_system.cpp :
    * func drawTexturedMesh - line 59-66
  * world_init.cpp :
    * func createHero - lines 21-24
  * world_system.cpp:
    * func step - lines 265-281
  * animated.fs.glsl
  * animated.vs.glsl

M2:
* Button System
  * world_init.cpp:
    * func createButton
  * render_system.cpp:
    * func drawTextureMesh
  * world_system.cpp:
    * func on_mouse_click
  * button.fs.glsl
  * button.vs.glsl
* Pause System
  * world_init.cpp:
    * func createHelperText
  * world_system.cpp:
    * func change_pause
  * main.cpp:
    * func main
* screen.fs.glsl
* screen.vs.glsl
* Spitter Enemy
  * world_system.cpp:
    * func small_spitter_enemy
  * world_init.cpp:
    * func createSpitterEnemy
    * func createSpitterEnemyBullet
  * physics_system.cpp:
    * func step *(at the bottom)
* Main Menu
  * world_init.cpp:
    * func createTitleText
  * world_system.cpp:
    * func create_help_screen
    * func create_title_screen
* Projectile Weapon:
  * weapon_utils.cpp
    * everything
  * physics_system.cpp
    * func collides
* Dynamic Difficulty System / Health Bar + Invulnerability
  * world_system.cpp
    * func step
    * func handle_collision
