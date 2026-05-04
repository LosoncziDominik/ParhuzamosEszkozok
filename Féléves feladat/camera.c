#include "camera.h"
#include "boids.h"
#include <windows.h>
#include <gl/glu.h>

#define SENSITIVITY 0.1f

Camera cam;

void init_camera() {
    cam.x = 0.0f;
    cam.y = 0.0f;
    cam.z = 5.0f;
    cam.pitch = 0.0f;
    cam.yaw = -90.0f;
    cam.speed = 0.7f;
    cam.firstMouse = 1;
}

void update_camera(const Uint8* keystates, int mouseX, int mouseY) {
    if (cam.firstMouse) {
        cam.lastMouseX = mouseX;
        cam.lastMouseY = mouseY;
        cam.firstMouse = 0;
    }

    cam.lastMouseX = mouseX;
    cam.lastMouseY = mouseY;

    cam.yaw   += mouseX * SENSITIVITY;
    cam.pitch += -mouseY * SENSITIVITY;



    if (cam.pitch > 89.0f) cam.pitch = 89.0f;
    if (cam.pitch < -89.0f) cam.pitch = -89.0f;



    float frontX = cosf(cam.yaw * M_PI / 180.0f) * cosf(cam.pitch * M_PI / 180.0f);
    float frontY = sinf(cam.pitch * M_PI / 180.0f);
    float frontZ = sinf(cam.yaw * M_PI / 180.0f) * cosf(cam.pitch * M_PI / 180.0f);

    float length = sqrtf(frontX * frontX + frontY * frontY + frontZ * frontZ);
    frontX /= length;
    frontY /= length;
    frontZ /= length;

    float rightX = -sinf(cam.yaw * M_PI / 180.0f);
    float rightZ = cosf(cam.yaw * M_PI / 180.0f);


    float rlen = sqrtf(rightX * rightX + rightZ * rightZ);
    rightX /= rlen;
    rightZ /= rlen;


    if (keystates[SDL_SCANCODE_W]) {
        cam.x += frontX * cam.speed;
        cam.y += frontY * cam.speed;
        cam.z += frontZ * cam.speed;
    }
    if (keystates[SDL_SCANCODE_S]) {
        cam.x -= frontX * cam.speed;
        cam.y -= frontY * cam.speed;
        cam.z -= frontZ * cam.speed;
    }
    if (keystates[SDL_SCANCODE_A]) {
        cam.x -= rightX * cam.speed;
        cam.z -= rightZ * cam.speed;
    }
    if (keystates[SDL_SCANCODE_D]) {
        cam.x += rightX * cam.speed;
        cam.z += rightZ * cam.speed;
    }
    if (keystates[SDL_SCANCODE_LSHIFT]){
        cam.y -= 1.0f * cam.speed;
    }
    if(keystates[SDL_SCANCODE_SPACE]){
        cam.y += 1.0f * cam.speed;
    }
    if(keystates[SDL_SCANCODE_LCTRL]){
        cam.speed = 1.5f;
    }
    else{
        cam.speed = 0.7f;
    }



}

void set_camera_view() {
    float frontX = cosf(cam.yaw * M_PI / 180.0f) * cosf(cam.pitch * M_PI / 180.0f);
    float frontY = sinf(cam.pitch * M_PI / 180.0f);
    float frontZ = sinf(cam.yaw * M_PI / 180.0f) * cosf(cam.pitch * M_PI / 180.0f);

    gluLookAt(
        cam.x, cam.y, cam.z,                             
        cam.x + frontX, cam.y + frontY, cam.z + frontZ,
        0.0f, 1.0f, 0.0f                                    
    );
}

float get_camera_x(){
    return cam.x;
}

float get_camera_y(){
    return cam.y;
}

float get_camera_z(){
    return cam.z;
}