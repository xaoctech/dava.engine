#pragma once

#include <Base/FastName.h>
#include <Math/Vector.h>
#include <Math/MathConstants.h>

// Input
static const DAVA::FastName SHOOTER_ACTION_MOVE_FORWARD("SHOOTER_MOVE_FORWARD");
static const DAVA::FastName SHOOTER_ACTION_MOVE_BACKWARD("SHOOTER_MOVE_BACKWARD");
static const DAVA::FastName SHOOTER_ACTION_MOVE_LEFT("SHOOTER_MOVE_LEFT");
static const DAVA::FastName SHOOTER_ACTION_MOVE_RIGHT("SHOOTER_MOVE_RIGHT");
static const DAVA::FastName SHOOTER_ACTION_ACCELERATE("SHOOTER_ACCELERATE");
static const DAVA::FastName SHOOTER_ACTION_ANALOG_MOVE("LMOVE");
static const DAVA::FastName SHOOTER_ACTION_ANALOG_ROTATE("RMOVE");
static const DAVA::FastName SHOOTER_ACTION_ATTACK_BULLET("FIRST_SHOOT");
static const DAVA::FastName SHOOTER_ACTION_ATTACK_ROCKET("SHOOTER_ATTACK_ROCKET");
static const DAVA::FastName SHOOTER_ACTION_INTERACT("SECOND_SHOOT");

// Character traits
static const DAVA::float32 SHOOTER_CHARACTER_CAPSULE_HEIGHT = 1.0f;
static const DAVA::float32 SHOOTER_CHARACTER_CAPSULE_RADIUS = 0.55f;
static const DAVA::Vector3 SHOOTER_CHARACTER_FORWARD = DAVA::Vector3(0.0f, -1.0f, 0.0f);
static const DAVA::Vector3 SHOOTER_CHARACTER_RIGHT = DAVA::Vector3(-1.0f, 0.0f, 0.0f);
static const DAVA::Vector3 SHOOTER_CHARACTER_LOOK_FROM = DAVA::Vector3(-0.17f, -0.37f, 1.8f); // Used for raycasting to camera position to avoid looking through obstacles
static const DAVA::uint32 SHOOTER_CHARACTER_MAX_HEALTH = 10;
static const DAVA::float32 SHOOTER_CHARACTER_ROTATION_SPEED = 0.20f;
static const DAVA::float32 SHOOTER_MOVEMENT_SPEED = 0.05f;

// Shooting traits
static const DAVA::float32 SHOOTER_MAX_SHOOTING_DISTANCE = 80.0f;
static const DAVA::Vector3 SHOOTER_AIM_OFFSET = DAVA::Vector3(-0.2f, 2.0f, 2.1f); // Offset of aim (and camera) relative to the character
static const DAVA::uint32 SHOOTER_SHOOT_COOLDOWN_FRAMES = 10; // How often a user can fire (cooldown in frames)

// Physics constants
static const DAVA::uint32 SHOOTER_STATIC_COLLISION_TYPE = 1 << 3;
static const DAVA::uint32 SHOOTER_CHARACTER_COLLISION_TYPE = 1 << 4;
static const DAVA::uint32 SHOOTER_PROJECTILE_COLLISION_TYPE = 1 << 5;
static const DAVA::uint32 SHOOTER_GRENADE_COLLISION_TYPE = 1 << 6;

// Scene & models constants
static const DAVA::FastName SHOOTER_GUN_BARREL_ENTITY_NAME = DAVA::FastName("shot_auto"); // Name of child entity in weapon model which is used to retrieve position of a gun barrel for raycasting etc.
static const DAVA::Vector3 SHOOTER_CAR_PASSENGER_NODES_POSITIONS[4] = { DAVA::Vector3(0.0f, -1.7f, 0.0f), DAVA::Vector3(0.0f, 1.7f, 0.0f), DAVA::Vector3(1.0f, -1.7f, 0.0f), DAVA::Vector3(1.0f, 1.7f, 0.0f) };
static const DAVA::Vector3 SHOOTER_CAR_CAMERA_OFFSET = DAVA::Vector3(0.0f, 10.0f, 4.0f);

// Cars specific constants
static const DAVA::uint32 SHOOTER_NUM_CARS = 5;
static const DAVA::uint32 SHOOTER_MAX_NUM_PASSENGERS = 4;
static const DAVA::float32 SHOOTER_CAR_IMPULSE_MAGNITUDE_PER_DAMAGE = 5000.0f;
