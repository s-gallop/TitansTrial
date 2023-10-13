# Team17-DefiantMortals-TitansTrial

Titan's Trial - A 2D Endless survival game

## Implemented features

* Basic level design

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

* Map asset - data/textures/background.png; world_init.cpp : func createBackground
* Collision with platform and Boundary - physics_system.cpp : func step
* Map collision placement - world_init.cpp : function createBlock; world_system.cpp : func restart_game; render_system.cpp : func drawTexturedMesh
* Player movement - world_system.cpp: func on_key, motion_helper
* Player gravity - physics_system.cpp : func step
* Double Jump + Wall Jump - world_system.cpp: func on_key, handle_collisions
* Random Movement enemy - world_init.cpp : func createEnemy; world_system.cpp : func step
* Random sword spawn -  world_system.cpp : func step, handle_collisions
* Sword swing and collision with enemy - world_system.cpp : func on_key, handle_collisions, on_mouse_move, step
* Sprite Animation - render_system.cpp : func drawTexturedMesh; world_init.cpp : func createHero; world_system.cpp: motion_helper; animated.fs.glsl; animated.vs.glsl
*
