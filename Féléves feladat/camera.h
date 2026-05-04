#ifndef CAMERA_H
#define CAMERA_H

#include "include/glad/glad.h"
#include "include/KHR/khrplatform.h"
#include <SDL2/SDL.h>

/**
 * Data of the player
 */
typedef struct {
    float x, y, z;
    float pitch, yaw;
    float speed;
    int lastMouseX, lastMouseY;
    int firstMouse;
} Camera;

/**
 * Moves the camera to it's starting position
 */
void init_camera();

/**
 * Handels the properties of the player
 */
void update_camera( const Uint8* keystates, int mouseX, int mouseY);

/**
 * Sets point of view
 */
void set_camera_view();

/**
 * Returns the cameras x position
 */
float get_camera_x();

/**
 * Returns the cameras y position
 */
float get_camera_y();

/**
 * Returns the cameras z position
 */
float get_camera_z();

#endif
