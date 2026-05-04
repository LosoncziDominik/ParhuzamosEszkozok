#ifndef OPENCL_BOIDS_H
#define OPENCL_BOIDS_H

#include "boids.h"

int init_opencl_boids(void);
void update_boids_opencl(void);
void cleanup_opencl_boids(void);

#endif