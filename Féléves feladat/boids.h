#ifndef BOIDS_H
#define BOIDS_H 
#include <stdbool.h>
#include <stdio.h>
#include "include/glad/glad.h"
#include "include/KHR/khrplatform.h"
#include <math.h>
#include <SDL2/SDL.h>

#ifndef CALLBACK
#define CALLBACK
#endif
#include <GL/glu.h>

#define NUM_BOIDS 1000
#define SPACE_WIDTH 400
#define SPACE_HEIGHT 250
#define SPACE_DEPTH 400
#define MAX_SPEED 0.5
#define MIN_SPEED 0.05
#define VIEW_RADIUS 15.0
#define SEPARATION_RADIUS 8.0f
#define COHESION_WEIGHT 0.3f
#define ALIGNMENT_WEIGHT 0.04f
#define SEPARATION_WEIGHT 0.8f
#define TURNING_RATE 0.007f
#define SPEED_SMOOTHING 0.002
#define M_PI 3.14159265358979323846
#define MAX_VERTICES 10000
#define MAX_FACES 10000
#define MAX_PARTICLES 500
#define NUM_DIRECTIONS 8

/**
 * Data of Boid
 */
typedef struct {
    float x, y, z;
    float vx, vy ,vz;
    bool alive;
    float targetSpeed;
    float speeds;
    float targetspeeds;
    float r, g, b;
    float frame_timer;
    int frame_index;
    float avoiding_timer;
    bool collision;
} Boid;

/**
 * Data of Vertex
 */
typedef struct {
    float x, y, z;
} Vertex;

/**
 * Data of Face
 */
typedef struct {
    int v1, v2, v3;
} Face;

/**
 * Data of Torus
 */
typedef struct{
    int x, y, z;
    float R, r;
}Torus;

/**
 * Data of Vector
 */
typedef struct {
    float x, y, z;
}Vec3;

/**
 * Data of Particles
 */
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float lifetime;
    float size;
    float fade;
    float speed;
} Particle;

/**
 * Data of Settings
 */
typedef struct{
    bool fog;
    bool debug;
    bool apply_water;
}Settings;



/**
 * Move the boid to the given position
 */
void init_boids();

/**
 * Updates the boid's behavior
 */
void update_boids();


/**
 * If an object passes the screen length it will come out of the other side
 */
void wrap_boid(Boid* b);

/**
 * This function renders the boids
 */
void render_boids();

void draw_plane(float x, float y, float z);

/**
 * Draws the bounds if the waterbox
 */
void draw_bounds();

/**
 * Prints the speed and target of the first boid for debug
 */
void whatspeed();

/**
 * Loads in an .obj file and saves the Vertexes and Faces in the arrays
 */
void load_obj(const char* filename, int index);

/**
 * Renders an object form the Vortexes and Faces
 */
void render_model_instance(float x, float y, float z, float yaw, float pitch, int index);

/**
 * Renders a donut shape with set properties
 */
void create_donut_shape(int index);

/**
 * Helper function returns the subtrection of 2 vectors
 */
Vec3 vec3_sub(Vec3 a, Vec3 b);

/**
 * Helper function returns the addition of 2 vectors
 */
Vec3 vec3_add(Vec3 a, Vec3 b);

/**
 * Helper function returns the multiplication of 2 vectors
 */
Vec3 vec3_mul(Vec3 v, float s);

/**
 * Helper function returns the scarlet multiplication of 2 vectors
 */
float vec3_dot(Vec3 a, Vec3 b);

/**
 * Helper function the vectors length
 */
float vec3_length(Vec3 v);

/**
 * Helper function returns the normalised version of a vector
 */
Vec3 vec3_normalize(Vec3 v);

/**
 * Helper function the distance between two points
 */
float vec3_distance(Vec3 a, Vec3 b);

/**
 * Helper function that scales a vector
 */
Vec3 vec3_scale(Vec3 v, float s);

/**
 * Returns the center points of a torus as a vector 
 */
Vec3 get_center(int index);

/**
 * Retrns the boid's velocity as a vector
 */
Vec3 get_boid_vel(int index);

/**
 * This function moves the torus to it's position
 */
void init_torus();

/**
 * Saves the image into a variable
 */
void init_textures();

/**
 * This function loads an image 
 */
GLuint load_texture(const char* filename);

/**
 * Returns the sky variable
 */
GLuint get_skytexture();

/**
 * Renders the moving sky
 */
void draw_sky_plane(float dt);

/**
 * Renders the particles
 */
void render_particles();

/**
 * Handles the particle's properties
 */
void update_particles(float deltaTime);

/**
 * Sets the starting values of a particle
 */
void init_particles();

/**
 * Fades the boid's color into blue depending on the players distance to it
 */
void apply_water_color(Vec3 pos, int index);

/**
 * Renders blue walls for given with, height and depth
 */
void draw_water_cube();

/**
 * Checks if a boid is going to collide with a torus
 */
bool check_ray_torus_collision(Vec3 ray_origin, Vec3 ray_dir, Torus t, float max_distance);

/**
 * Sets deltatime
 */
void set_timer(Uint32 time);

/**
 * Returns deltatime
 */
float get_timer();

/**
 * Finds a direction where there aren't any objects (doesn't work yet)
 */
Vec3 find_free_direction(Vec3 origin, Vec3 forward, Torus* toruses, int torusCount, float max_distance);

/**
 * Draws the collision ray for the boid for debug
 */
void draw_ray(Vec3 origin, Vec3 direction, float length, Vec3 color);

/**
 * Applies fog 
 */
void render_fog();

/**
 * Turns fog on and off
 */
void fog_setter();

/**
 * Turns water colored boids on and off
 */
void apply_water_setter();

/**
 * Turns the boid's collision ray on and off if it's heading for collision
 */
void debug_setter();

#endif