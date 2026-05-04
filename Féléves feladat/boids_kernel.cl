typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float targetSpeed;
    float speeds;
    float targetspeeds;
    int collision;
} BoidCL;

#define SPACE_WIDTH 400.0f
#define SPACE_HEIGHT 250.0f
#define SPACE_DEPTH 400.0f

#define MAX_SPEED 0.7f
#define MIN_SPEED 0.15f

#define VIEW_RADIUS 35.0f
#define SEPARATION_RADIUS 12.0f

#define SEPARATION_RADIUS 8.0f
#define COHESION_WEIGHT 0.3f
#define ALIGNMENT_WEIGHT 0.04f

#define TURNING_RATE 0.025f
#define WALL_FORCE 0.012f
#define DT 1.0f

__kernel void update_boids_kernel(
    __global const BoidCL* input,
    __global BoidCL* output,
    const int numBoids
) {
    int i = get_global_id(0);

    if (i >= numBoids) {
        return;
    }

    BoidCL self = input[i];

    float avgX = 0.0f;
    float avgY = 0.0f;
    float avgZ = 0.0f;

    float avgVX = 0.0f;
    float avgVY = 0.0f;
    float avgVZ = 0.0f;

    float sepX = 0.0f;
    float sepY = 0.0f;
    float sepZ = 0.0f;
    int sepCount = 0;

    int count = 0;

    float viewRadiusSq = VIEW_RADIUS * VIEW_RADIUS;
    float separationRadiusSq = SEPARATION_RADIUS * SEPARATION_RADIUS;

    for (int j = 0; j < numBoids; j++) {
        if (i == j) {
            continue;
        }

        float dx = input[j].x - self.x;
        float dy = input[j].y - self.y;
        float dz = input[j].z - self.z;

        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq < viewRadiusSq) {
            avgX += input[j].x;
            avgY += input[j].y;
            avgZ += input[j].z;

            avgVX += input[j].vx;
            avgVY += input[j].vy;
            avgVZ += input[j].vz;

            count++;
        }

        if (distSq < separationRadiusSq && distSq > 0.0001f) {
            float invDist = native_rsqrt(distSq);

            sepX -= dx * invDist;
            sepY -= dy * invDist;
            sepZ -= dz * invDist;

            sepCount++;
        }
    }

    if (count > 0) {
        float invCount = 1.0f / (float)count;

        avgX *= invCount;
        avgY *= invCount;
        avgZ *= invCount;

        avgVX *= invCount;
        avgVY *= invCount;
        avgVZ *= invCount;

        float cohesionX = avgX - self.x;
        float cohesionY = avgY - self.y;
        float cohesionZ = avgZ - self.z;

        self.vx += cohesionX * COHESION_WEIGHT;
        self.vy += cohesionY * COHESION_WEIGHT;
        self.vz += cohesionZ * COHESION_WEIGHT;

        self.vx += (avgVX - self.vx) * ALIGNMENT_WEIGHT;
        self.vy += (avgVY - self.vy) * ALIGNMENT_WEIGHT;
        self.vz += (avgVZ - self.vz) * ALIGNMENT_WEIGHT;
    }

    if (sepCount > 0) {
        float invSepCount = 1.0f / (float)sepCount;

        sepX *= invSepCount;
        sepY *= invSepCount;
        sepZ *= invSepCount;
    }

    self.vx += sepX * SEPARATION_WEIGHT;
    self.vy += sepY * SEPARATION_WEIGHT;
    self.vz += sepZ * SEPARATION_WEIGHT;

    float halfW = SPACE_WIDTH * 0.5f;
    float halfH = SPACE_HEIGHT * 0.5f;
    float halfD = SPACE_DEPTH * 0.5f;

    float margin = 35.0f;

    if (self.x > halfW - margin) {
        self.vx -= WALL_FORCE;
    }

    if (self.x < -halfW + margin) {
        self.vx += WALL_FORCE;
    }

    if (self.y > halfH - margin) {
        self.vy -= WALL_FORCE;
    }

    if (self.y < -halfH + margin) {
        self.vy += WALL_FORCE;
    }

    if (self.z > halfD - margin) {
        self.vz -= WALL_FORCE;
    }

    if (self.z < -halfD + margin) {
        self.vz += WALL_FORCE;
    }

    float speedSq = self.vx * self.vx + self.vy * self.vy + self.vz * self.vz;

    if (speedSq < 0.0001f) {
        self.vx = 0.2f;
        self.vy = 0.1f;
        self.vz = 0.15f;
        speedSq = self.vx * self.vx + self.vy * self.vy + self.vz * self.vz;
    }

    float speed = sqrt(speedSq);

    if (speed > MAX_SPEED) {
        float scale = MAX_SPEED / speed;
        self.vx *= scale;
        self.vy *= scale;
        self.vz *= scale;
        speed = MAX_SPEED;
    }

    if (speed < MIN_SPEED) {
        float scale = MIN_SPEED / speed;
        self.vx *= scale;
        self.vy *= scale;
        self.vz *= scale;
        speed = MIN_SPEED;
    }

    self.x += self.vx * DT;
    self.y += self.vy * DT;
    self.z += self.vz * DT;

    if (self.x > halfW) self.x = halfW;
    if (self.x < -halfW) self.x = -halfW;

    if (self.y > halfH) self.y = halfH;
    if (self.y < -halfH) self.y = -halfH;

    if (self.z > halfD) self.z = halfD;
    if (self.z < -halfD) self.z = -halfD;

    self.speeds = speed;
    self.targetspeeds = MAX_SPEED;
    self.targetSpeed = MAX_SPEED;
    self.collision = 0;

    output[i] = self;
}