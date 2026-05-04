#include <math.h>
#include <stdlib.h>
#include "boids.h"
#include "camera.h"
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



Boid boids[NUM_BOIDS];
Vertex vertices[8][1000];
Face faces[8][1000];
int vertex_counts[8] = {0};
int face_counts[8] = {0};
Torus torus[3];
Vec3 boid_pos[NUM_BOIDS];
Vec3 boid_vel;
GLuint skyTextureID;
float skyScroll = 0.0f;
Particle particles[MAX_PARTICLES];
GLuint smokeTextureID;
Uint32 timetime;
Settings setting;


void init_boids() {
    for (int i = 0; i < NUM_BOIDS; i++) {
        boids[i].x = (rand() % SPACE_WIDTH) - SPACE_WIDTH / 2;
        boids[i].y = (rand() % SPACE_HEIGHT) - SPACE_HEIGHT / 2;
        boids[i].z = (rand() % SPACE_DEPTH) - SPACE_DEPTH / 2;
        boids[i].vx = ((float)rand() / RAND_MAX - 0.5f);
        boids[i].vy = ((float)rand() / RAND_MAX - 0.5f);
        boids[i].vz = ((float)rand() / RAND_MAX - 0.5f);
        boids[i].alive = true;
        boids[i].targetSpeed = MAX_SPEED * (0.7f + (rand() % 5) * 0.01f);
        boids[i].r = (rand() % 9) / 10.0f;
        boids[i].g = (rand() % 9) / 10.0f;
        boids[i].b = (rand() % 9) / 10.0f;
        boids[i].frame_index = rand() % 8;
        boids[i].frame_timer = ((float)rand() / RAND_MAX) * 0.5f;
        boids[i].avoiding_timer = 0.0f;
        boids[i].collision = false;
    }

    load_obj("hal.obj", 0);
    load_obj("hal3.obj", 1);
    load_obj("hal4.obj", 2);
    load_obj("hal3.obj", 3);
    load_obj("hal.obj", 4);
    load_obj("hal1.obj", 5);
    load_obj("hal2.obj", 6);
    load_obj("hal1.obj", 7);
}

void init_torus(){
    torus[0].x = 80;
    torus[0].y = 30;
    torus[0].z = -40;

    torus[1].x = -80;
    torus[1].y = 30;
    torus[1].z = -40;

    torus[2].x = 0;
    torus[2].y = -50;
    torus[2].z = 40;
    
    for(int i = 0 ; i < 3; i++){
        torus[i].R = 30.0f;
        torus[i].r = 10.0f; 
    }
}

void init_settings(){
    setting.fog = false;
    setting.apply_water = false;
    setting.debug = false;
}


void update_boids() {

    for (int i = 0; i < NUM_BOIDS; i++) {
        float avgX = 0, avgY = 0, avgZ = 0;
        float avgVX = 0, avgVY = 0, avgVZ = 0;
        float sepX = 0, sepY = 0, sepZ = 0;
        int count = 0;

        for (int j = 0; j < NUM_BOIDS; j++) {
            if (i == j) continue;

            float dx = boids[j].x - boids[i].x;
            float dy = boids[j].y - boids[i].y;
            float dz = boids[j].z - boids[i].z;
            float dist = sqrt(dx * dx + dy * dy + dz * dz);

            if (dist < VIEW_RADIUS) {
                avgX += boids[j].x;
                avgY += boids[j].y;
                avgZ += boids[j].z;
                avgVX += boids[j].vx;
                avgVY += boids[j].vy;
                avgVZ += boids[j].vz;
                count++;
            }

            if (dist < SEPARATION_RADIUS) {
                sepX -= (dx * SPEED_SMOOTHING);
                sepY -= (dy * SPEED_SMOOTHING);
                sepZ -= (dz * SPEED_SMOOTHING);
            }
        }

        float alignWeight = ALIGNMENT_WEIGHT;
        float cohesionWeight = COHESION_WEIGHT;


        if (count > 0) {

            if (boids[i].avoiding_timer > 0.0f)
            {
                alignWeight *= 0.2f;
                cohesionWeight *= 0.2f;
            }
            else {
                alignWeight = ALIGNMENT_WEIGHT / (1.0f + 0.05f * count);
                cohesionWeight = COHESION_WEIGHT / (1.0f + 0.05f * count);
            }
            
            avgX /= count;
            avgY /= count;
            avgZ /= count;
            avgVX /= count;
            avgVY /= count;
            avgVZ /= count;

            float targetVX = (avgX - boids[i].x) * cohesionWeight;
            float targetVY = (avgY - boids[i].y) * cohesionWeight;
            float targetVZ = (avgZ - boids[i].z) * cohesionWeight;

            boids[i].vx += (targetVX - boids[i].vx) * TURNING_RATE ;
            boids[i].vy += (targetVY - boids[i].vy) * TURNING_RATE ;
            boids[i].vz += (targetVZ - boids[i].vz) * TURNING_RATE ;

            boids[i].vx += (avgVX - boids[i].vx) * alignWeight;
            boids[i].vy += (avgVY - boids[i].vy) * alignWeight;
            boids[i].vz += (avgVZ - boids[i].vz) * alignWeight;
        }

        boids[i].vx += sepX * SEPARATION_WEIGHT;
        boids[i].vy += sepY * SEPARATION_WEIGHT;
        boids[i].vz += sepZ * SEPARATION_WEIGHT;

        const float margin = 10.0f;
        const float avoid = 0.01f;
        if (boids[i].x >  SPACE_WIDTH /2 - margin) boids[i].vx -= avoid;
        if (boids[i].x < -SPACE_WIDTH /2 + margin) boids[i].vx += avoid;
        if (boids[i].y >  SPACE_HEIGHT/2 - margin) boids[i].vy -= avoid;
        if (boids[i].y < -SPACE_HEIGHT/2 + margin) boids[i].vy += avoid;
        if (boids[i].z >  SPACE_DEPTH /2 - margin) boids[i].vz -= avoid;
        if (boids[i].z < -SPACE_DEPTH /2 + margin) boids[i].vz += avoid;

        float speed = sqrt(boids[i].vx * boids[i].vx + boids[i].vy * boids[i].vy + boids[i].vz * boids[i].vz);
        boids[i].speeds = speed;

        float targetSpeed = MAX_SPEED * 0.7f + (rand() % 5) * 0.1f;
        boids[i].targetspeeds = targetSpeed;

        if (speed > MAX_SPEED) {
            float scale = MAX_SPEED / speed;
            boids[i].vx *= scale;
            boids[i].vy *= scale;
            boids[i].vz *= scale;
        }

        if (speed < targetSpeed) {
            boids[i].vx *= (1 + SPEED_SMOOTHING);
            boids[i].vy *= (1 + SPEED_SMOOTHING);
            boids[i].vz *= (1 + SPEED_SMOOTHING);
        } else if (speed > targetSpeed){
            boids[i].vx *= (1 - SPEED_SMOOTHING);
            boids[i].vy *= (1 - SPEED_SMOOTHING);
            boids[i].vz *= (1 - SPEED_SMOOTHING);
        }

        //Vec3 origin = { boids[i].x, boids[i].y, boids[i].z };
        //Vec3 dir = vec3_normalize((Vec3){ boids[i].vx, boids[i].vy, boids[i].vz });

        boids[i].collision = false;

        /*for (int h = 0; h < 3; h++) {
            if (check_ray_torus_collision(origin, dir, torus[h], 30.0f)) {
                boids[i].collision = true;
                break;
            }
        }*/

        if (boids[i].collision && boids[i].avoiding_timer <= 0.0f) {
            Vec3 origin = { boids[i].x, boids[i].y, boids[i].z };
            Vec3 forward = vec3_normalize((Vec3){ boids[i].vx, boids[i].vy, boids[i].vz });
        
            Vec3 safe_dir = find_free_direction(origin, forward, torus, 3, 30.0f);
        
            boids[i].vx = safe_dir.x * MAX_SPEED + 5;
            boids[i].vy = safe_dir.y * MAX_SPEED + 5;
            boids[i].vz = safe_dir.z * MAX_SPEED + 5;
        
            boids[i].avoiding_timer = 0.8f;
        }
        
        boids[i].avoiding_timer -= get_timer();
        if (boids[i].avoiding_timer < 0.0f) boids[i].avoiding_timer = 0.0f;

        boids[i].x += boids[i].vx;
        boids[i].y += boids[i].vy;
        boids[i].z += boids[i].vz;

        boids[i].frame_timer -= 0.064f;
        if (boids[i].frame_timer <= 0) {
            boids[i].frame_index = (boids[i].frame_index + 1) % 8;
            boids[i].frame_timer = 0.2f + ((float)rand() / RAND_MAX) * 0.3f;
        }

    }
}

void init_particles() {

    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].x = ((rand() % SPACE_WIDTH) - SPACE_WIDTH / 2.0f);
        particles[i].y = ((rand() % SPACE_HEIGHT) - SPACE_HEIGHT / 2.0f);
        particles[i].z = ((rand() % SPACE_DEPTH) - SPACE_DEPTH / 2.0f);

        particles[i].vx = ((rand() % 100) - 50) / 5000.0f;
        particles[i].vy = ((rand() % 50) / 1000.0f) + 0.01f;
        particles[i].vz = ((rand() % 100) - 50) / 5000.0f;

        particles[i].lifetime = (rand() % 1000) / 100.0f + 1.0f;
        particles[i].size = (rand() % 10) / 10.0f + 0.5f;
    }
}

void update_particles(float deltaTime) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].x += particles[i].vx * deltaTime * 100;
        particles[i].y += particles[i].vy * deltaTime * 100;
        particles[i].z += particles[i].vz * deltaTime * 100;

        particles[i].lifetime -= deltaTime;

        bool outOfBounds =
            fabsf(particles[i].x) > SPACE_WIDTH / 2 ||
            fabsf(particles[i].y) > SPACE_HEIGHT / 2 ||
            fabsf(particles[i].z) > SPACE_DEPTH / 2;

        if (particles[i].lifetime <= 0.0f || outOfBounds) {
            particles[i].x = ((rand() % SPACE_WIDTH) - SPACE_WIDTH / 2.0f);
            particles[i].y = ((rand() % SPACE_HEIGHT) - SPACE_HEIGHT / 2.0f);
            particles[i].z = ((rand() % SPACE_DEPTH) - SPACE_DEPTH / 2.0f);

            particles[i].vx = ((rand() % 100) - 50) / 10000.0f;
            particles[i].vy = (rand() % 50) / 1000.0f + 0.05f;
            particles[i].vz = ((rand() % 100) - 50) / 10000.0f;
            

            particles[i].lifetime = (rand() % 100) / 100.0f + 1.0f;
            particles[i].size = (rand() % 10) / 100.0f + 0.5f;
        }
    }
}


void render_particles() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, smokeTextureID);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        float alpha = particles[i].lifetime / 1.5f;

        glColor4f(1.0f, 1.0f, 1.0f, alpha);

        float size = particles[i].size / 2.0f;

        glPushMatrix();
        glTranslatef(particles[i].x, particles[i].y, particles[i].z);

        glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex3f(-size, -size, 0);
            glTexCoord2f(1, 0); glVertex3f( size, -size, 0);
            glTexCoord2f(1, 1); glVertex3f( size,  size, 0);
            glTexCoord2f(0, 1); glVertex3f(-size,  size, 0);
        glEnd();

        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void wrap_boid(Boid* b) {
    if (b->x < -SPACE_WIDTH / 2)  b->x += SPACE_WIDTH * 2;
    if (b->x >  SPACE_WIDTH / 2)  b->x -= SPACE_WIDTH * 2;

    if (b->y < -SPACE_HEIGHT / 2) b->y += SPACE_HEIGHT * 2;
    if (b->y >  SPACE_HEIGHT / 2) b->y -= SPACE_HEIGHT * 2;

    if (b->z < -SPACE_DEPTH / 2)  b->z += SPACE_DEPTH * 2;
    if (b->z >  SPACE_DEPTH / 2)  b->z -= SPACE_DEPTH * 2;
}


void render_boids() {
    for (int i = 0; i < NUM_BOIDS; i++) {
        if (!boids[i].alive) continue;
        
        Vec3 vel = { boids[i].vx, boids[i].vy, boids[i].vz };
        Vec3 dir = vec3_normalize(vel);

        float yaw = atan2f(dir.x, dir.z) * 180.0f / M_PI;
        float pitch = -asinf(dir.y) * 180.0f / M_PI;

        Vec3 boid_pos = { boids[i].x, boids[i].y, boids[i].z };
        apply_water_color(boid_pos, i);

        Vec3 color = {1.0f, 0.0f, 0.0f};
        if (boids[i].collision)
        {
            draw_ray(boid_pos, dir, 30.0f, color);
        }
        
        render_model_instance(boids[i].x, boids[i].y, boids[i].z, yaw, pitch, i);
    }
}

void draw_plane(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glBegin(GL_QUADS);
        glColor3f(1, 1, 0);
        glVertex3f(-0.5f, 0, -0.5f);
        glVertex3f( 0.5f, 0, -0.5f);
        glVertex3f( 0.5f, 0,  0.5f);
        glVertex3f(-0.5f, 0,  0.5f);
    glEnd();
    glPopMatrix();
}


void draw_bounds() {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES);
    for (int i = -1; i <= 1; i += 2) {
        for (int j = -1; j <= 1; j += 2) {
            for (int k = -1; k <= 1; k += 2) {
                glVertex3f(i * SPACE_WIDTH/2, j * SPACE_HEIGHT/2, -SPACE_DEPTH/2);
                glVertex3f(i * SPACE_WIDTH/2, j * SPACE_HEIGHT/2,  SPACE_DEPTH/2);
            }
        }
    }
    glEnd();
}

void whatspeed(){
    printf("\n%f\n%f\n", boids[0].speeds, boids[0].targetspeeds);
    printf("\nx:%f\ny:%f\nz:%f\n", boids[0].x, boids[0].y, boids[0].z);
}

void load_obj(const char* filename, int index) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Could not open OBJ file\n");
        exit(1);
    }

    vertex_counts[index] = 0;
    face_counts[index] = 0;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                vertices[index][vertex_counts[index]++] = (Vertex){x, y, z};
            }
        } else if (strncmp(line, "f ", 2) == 0) {
            int v[4];
            int matches = sscanf(line, "f %d %d %d %d", &v[0], &v[1], &v[2], &v[3]);

            if (matches == 3) {
                faces[index][face_counts[index]++] = (Face){v[0], v[1], v[2]};
            } else if (matches == 4) {
                faces[index][face_counts[index]++] = (Face){v[0], v[1], v[2]};
                faces[index][face_counts[index]++] = (Face){v[0], v[2], v[3]};
            } else {
                int vtx[4], vt[4], vn[4];
                int count = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                    &vtx[0], &vt[0], &vn[0], &vtx[1], &vt[1], &vn[1], &vtx[2], &vt[2], &vn[2], &vtx[3], &vt[3], &vn[3]);

                if (count >= 9) {
                    faces[index][face_counts[index]++] = (Face){vtx[0], vtx[1], vtx[2]};
                    if (count == 12) {
                        faces[index][face_counts[index]++] = (Face){vtx[0], vtx[2], vtx[3]};
                    }
                } else {
                    count = sscanf(line, "f %d//%d %d//%d %d//%d %d//%d",
                        &vtx[0], &vn[0], &vtx[1], &vn[1], &vtx[2], &vn[2], &vtx[3], &vn[3]);

                    if (count >= 6) {
                        faces[index][face_counts[index]++] = (Face){vtx[0], vtx[1], vtx[2]};
                        if (count == 8) {
                            faces[index][face_counts[index]++] = (Face){vtx[0], vtx[2], vtx[3]};
                        }
                    }
                }
            }
        }
    }

    fclose(file);
}

void render_model_instance(float x, float y, float z, float yaw, float pitch, int index) {
    int frame = boids[index].frame_index + boids[index].frame_timer;


    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(yaw, 0, 1, 0);
    glRotatef(pitch, 1, 0, 0);
    glColor3f(boids[index].r, boids[index].g, boids[index].b);
    Vec3 boid_pos = { boids[index].x, boids[index].y, boids[index].z };
    apply_water_color(boid_pos, index);


    glBegin(GL_TRIANGLES);
    for (int i = 0; i < face_counts[frame]; i++) {
        int vi1 = faces[frame][i].v1 - 1;
        int vi2 = faces[frame][i].v2 - 1;
        int vi3 = faces[frame][i].v3 - 1;

        glVertex3f(vertices[frame][vi1].x, vertices[frame][vi1].y, vertices[frame][vi1].z);
        glVertex3f(vertices[frame][vi2].x, vertices[frame][vi2].y, vertices[frame][vi2].z);
        glVertex3f(vertices[frame][vi3].x, vertices[frame][vi3].y, vertices[frame][vi3].z);
    }
    glEnd();

    glPopMatrix();
}

void create_donut_shape(int index) {
    int numSides = 32;
    int numRings = 32;
    float R = torus[index].R;
    float r = torus[index].r;

    for (int j = 0; j < numRings; ++j) {
        float theta = 2.0f * M_PI * j / numRings;
        float nextTheta = 2.0f * M_PI * (j + 1) / numRings;

        for (int k = 0; k < numSides; ++k) {
            float phi = 2.0f * M_PI * k / numSides;
            float nextPhi = 2.0f * M_PI * (k + 1) / numSides;

            float x0 = (R + r * cos(phi)) * cos(theta);
            float y0 = (R + r * cos(phi)) * sin(theta);
            float z0 = r * sin(phi);

            float x1 = (R + r * cos(nextPhi)) * cos(theta);
            float y1 = (R + r * cos(nextPhi)) * sin(theta);
            float z1 = r * sin(nextPhi);

            float x2 = (R + r * cos(nextPhi)) * cos(nextTheta);
            float y2 = (R + r * cos(nextPhi)) * sin(nextTheta);
            float z2 = r * sin(nextPhi);

            float x3 = (R + r * cos(phi)) * cos(nextTheta);
            float y3 = (R + r * cos(phi)) * sin(nextTheta);
            float z3 = r * sin(phi);

            glBegin(GL_TRIANGLES);
            glVertex3f(x0, y0, z0);
            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);

            glVertex3f(x0, y0, z0);
            glVertex3f(x2, y2, z2);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }   
}

Vec3 get_center(int index)
{
    Vec3 torus_center;
    torus_center.x = torus[index].x;
    torus_center.y = torus[index].y;
    torus_center.z = torus[index].z;

    return torus_center;
}

Vec3 get_boid_vel(int index){
    Vec3 boid_vel = (Vec3){boids[index].vx, boids[index].vy, boids[index].vz,};

    return boid_vel;
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vec3_mul(Vec3 v, float s) {
    return (Vec3){v.x * s, v.y * s, v.z * s};
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_length(Vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 vec3_normalize(Vec3 v) {
    float len = vec3_length(v);
    if (len > 0.0001f)
        return vec3_mul(v, 1.0f / len);
    return (Vec3){0, 0, 0};
}

float vec3_distance(Vec3 a, Vec3 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

Vec3 vec3_scale(Vec3 v, float s) {
    return (Vec3){v.x * s, v.y * s, v.z * s};
}

void apply_water_color(Vec3 pos, int index) {
    if (setting.apply_water)
    {
        float dx = pos.x - get_camera_x();
        float dy = pos.y - get_camera_y();
        float dz = pos.z - get_camera_z();
    
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        if(dist > 40){
    
        float maxDist = 200.0f;
    
        float t = fminf((dist - 40) / maxDist, 1.0f);
    
        float r = (boids[index].r - t) * boids[index].r + t * 0.2f;
        float g = (boids[index].g - t) * boids[index].g + t * 0.4f;
        float b = (boids[index].b - t) * boids[index].b + t * 1.0f;
    
        glColor3f(r, g, b);
        }
    }
    
    
    
}


GLuint load_texture(const char* filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        printf("Failed to load texture: %s\n", filename);
        exit(1);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return textureID;
}

void init_textures() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    skyTextureID = load_texture("sky.png");
}

GLuint get_skytexture(){
    return skyTextureID;
}

void draw_sky_plane(float dt) {
    skyScroll += dt * 0.02f;
    if (skyScroll > 1.0f) skyScroll -= 1.0f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyTextureID);

    glColor3f(1, 1, 1);

    glPushMatrix();
    glTranslatef(0.0f, 500.0f, 0.0f);

    float size = 10000.0f;
    float repeat = 30.0f;

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f + skyScroll, 0.0f); glVertex3f(-size, 0, -size);
        glTexCoord2f(repeat + skyScroll, 0.0f); glVertex3f( size, 0, -size);
        glTexCoord2f(repeat + skyScroll, repeat); glVertex3f( size, 0,  size);
        glTexCoord2f(0.0f + skyScroll, repeat); glVertex3f(-size, 0,  size);
    glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}


void draw_water_cube() {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.2f, 0.5f, 1.0f, 0.7f);

    float w = SPACE_WIDTH / 2.0f;
    float h = SPACE_HEIGHT / 2.0f;
    float d = SPACE_DEPTH / 2.0f;

    glBegin(GL_QUADS);

    glVertex3f(-w, -h,  d); glVertex3f(w, -h,  d); glVertex3f(w, h,  d); glVertex3f(-w, h,  d);

    glVertex3f(-w, -h, -d); glVertex3f(-w, h, -d); glVertex3f(w, h, -d); glVertex3f(w, -h, -d);

    glVertex3f(-w, -h, -d); glVertex3f(-w, -h,  d); glVertex3f(-w, h,  d); glVertex3f(-w, h, -d);

    glVertex3f(w, -h, -d); glVertex3f(w, h, -d); glVertex3f(w, h,  d); glVertex3f(w, -h,  d);

    glVertex3f(-w, -h, -d); glVertex3f(w, -h, -d); glVertex3f(w, -h,  d); glVertex3f(-w, -h,  d);

    glVertex3f(-w, h, -d); glVertex3f(-w, h,  d); glVertex3f(w, h,  d); glVertex3f(w, h, -d);
    glEnd();

    glDisable(GL_BLEND);
}
/*
bool check_ray_torus_collision(Vec3 ray_origin, Vec3 ray_dir, Torus t, float max_distance) {
    int steps = 10;
    float step_size = max_distance / (float)steps;

    for (int i = 1; i <= steps; ++i) {
        float t_step = i * step_size;
        Vec3 point = vec3_add(ray_origin, vec3_scale(ray_dir, t_step));

        float dx = point.x - t.x;
        float dz = point.z - t.z;
        float dist_xz = sqrtf(dx * dx + dz * dz);

        float circle_diff = fabsf(dist_xz - t.R+2);

        float dy = fabsf(point.y - t.y);

        if (circle_diff < t.r && dy < t.r) {
            return true;
        }
    }

    return false;
}*/

void set_timer(Uint32 time){
    timetime = time;
}

float get_timer(){
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - timetime) / 1000.0f;
    timetime = currentTime;

    return deltaTime;
}
/*
Vec3 find_free_direction(Vec3 origin, Vec3 forward, Torus* toruses, int torusCount, float max_distance) {
    Vec3 directions[NUM_DIRECTIONS] = {
        { 1,  0,  0 },
        {-1,  0,  0 },
        { 0,  0,  1 },
        { 0,  0, -1 },
        { 0.707f, 0,  0.707f },
        {-0.707f, 0,  0.707f },
        { 0.707f, 0, -0.707f },
        {-0.707f, 0, -0.707f }
    };

    for (int i = 0; i < NUM_DIRECTIONS; ++i) {
        Vec3 dir = vec3_normalize(directions[i]);
        bool isSafe = true;

        for (int h = 0; h < torusCount; ++h) {
            if (check_ray_torus_collision(origin, dir, toruses[h], max_distance)) {
                isSafe = false;
                break;
            }
        }

        if (isSafe) {
            return dir;
        }
    }

    return forward;
}*/

void draw_ray(Vec3 origin, Vec3 direction, float length, Vec3 color) {
    if(setting.debug){

        Vec3 end = {
            origin.x + direction.x * length,
            origin.y + direction.y * length,
            origin.z + direction.z * length
        };
    
        glColor3f(color.x, color.y, color.z);
        glBegin(GL_LINES);
            glVertex3f(origin.x, origin.y, origin.z);
            glVertex3f(end.x, end.y, end.z);
        glEnd();
    }
    
}

void render_fog(){
    if (setting.fog)
    {
        GLfloat fogColor[4] = {0.4f, 0.6f, 0.95f, 1.0f};
        glEnable(GL_FOG);
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, 0.01f);
        glFogi(GL_FOG_MODE, GL_EXP2);
    }
    else{
        glDisable(GL_FOG);
    }
 
}

void fog_setter(){
    setting.fog = !setting.fog;
}

void apply_water_setter(){
    setting.apply_water = !setting.apply_water;
}

void debug_setter(){
    setting.debug = !setting.debug;
}