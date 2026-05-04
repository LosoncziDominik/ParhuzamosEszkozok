#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

#include "boids.h"
#include "opencl_boids.h"

extern Boid boids[NUM_BOIDS];

typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float targetSpeed;
    float speeds;
    float targetspeeds;
    int collision;
} BoidCL;

static cl_platform_id platform = NULL;
static cl_device_id device = NULL;
static cl_context context = NULL;
static cl_command_queue queue = NULL;
static cl_program program = NULL;
static cl_kernel kernel = NULL;

static cl_mem bufferA = NULL;
static cl_mem bufferB = NULL;

static int useBufferAAsInput = 1;
static BoidCL boidsCL[NUM_BOIDS];

static char* load_kernel_source(const char* filename)
{
    FILE* file = fopen(filename, "rb");

    if (!file) {
        printf("Nem sikerült megnyitni a kernel fájlt: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* source = (char*)malloc(size + 1);

    if (!source) {
        fclose(file);
        return NULL;
    }

    fread(source, 1, size, file);
    source[size] = '\0';

    fclose(file);
    return source;
}

static void print_build_log(void)
{
    size_t logSize = 0;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);

    if (logSize > 1) {
        char* log = (char*)malloc(logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
        printf("\nOpenCL build log:\n%s\n", log);
        free(log);
    }
}

static void copy_cpu_to_cl(void)
{
    for (int i = 0; i < NUM_BOIDS; i++) {
        boidsCL[i].x = boids[i].x;
        boidsCL[i].y = boids[i].y;
        boidsCL[i].z = boids[i].z;

        boidsCL[i].vx = boids[i].vx;
        boidsCL[i].vy = boids[i].vy;
        boidsCL[i].vz = boids[i].vz;

        boidsCL[i].targetSpeed = boids[i].targetSpeed;
        boidsCL[i].speeds = boids[i].speeds;
        boidsCL[i].targetspeeds = boids[i].targetspeeds;
        boidsCL[i].collision = boids[i].collision ? 1 : 0;
    }
}

static void copy_cl_to_cpu(void)
{
    for (int i = 0; i < NUM_BOIDS; i++) {
        boids[i].x = boidsCL[i].x;
        boids[i].y = boidsCL[i].y;
        boids[i].z = boidsCL[i].z;

        boids[i].vx = boidsCL[i].vx;
        boids[i].vy = boidsCL[i].vy;
        boids[i].vz = boidsCL[i].vz;

        boids[i].targetSpeed = boidsCL[i].targetSpeed;
        boids[i].speeds = boidsCL[i].speeds;
        boids[i].targetspeeds = boidsCL[i].targetspeeds;
        boids[i].collision = boidsCL[i].collision != 0;
    }
}

int init_opencl_boids(void)
{
    cl_int err;

    err = clGetPlatformIDs(1, &platform, NULL);

    if (err != CL_SUCCESS) {
        printf("Nincs OpenCL platform.\n");
        return 0;
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    if (err != CL_SUCCESS) {
        printf("Nincs GPU OpenCL eszköz, CPU-t próbálok.\n");

        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);

        if (err != CL_SUCCESS) {
            printf("Nincs használható OpenCL eszköz.\n");
            return 0;
        }
    }

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);

    if (err != CL_SUCCESS) {
        printf("OpenCL context létrehozási hiba: %d\n", err);
        return 0;
    }

    queue = clCreateCommandQueue(context, device, 0, &err);

    if (err != CL_SUCCESS) {
        printf("OpenCL command queue hiba: %d\n", err);
        return 0;
    }

    char* source = load_kernel_source("boids_kernel.cl");

    if (!source) {
        return 0;
    }

    program = clCreateProgramWithSource(
        context,
        1,
        (const char**)&source,
        NULL,
        &err
    );

    free(source);

    if (err != CL_SUCCESS) {
        printf("OpenCL program létrehozási hiba: %d\n", err);
        return 0;
    }

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    if (err != CL_SUCCESS) {
        printf("OpenCL kernel fordítási hiba: %d\n", err);
        print_build_log();
        return 0;
    }

    kernel = clCreateKernel(program, "update_boids_kernel", &err);

    if (err != CL_SUCCESS) {
        printf("OpenCL kernel létrehozási hiba: %d\n", err);
        return 0;
    }

    bufferA = clCreateBuffer(
        context,
        CL_MEM_READ_WRITE,
        sizeof(BoidCL) * NUM_BOIDS,
        NULL,
        &err
    );

    if (err != CL_SUCCESS) {
        printf("OpenCL bufferA hiba: %d\n", err);
        return 0;
    }

    bufferB = clCreateBuffer(
        context,
        CL_MEM_READ_WRITE,
        sizeof(BoidCL) * NUM_BOIDS,
        NULL,
        &err
    );

    if (err != CL_SUCCESS) {
        printf("OpenCL bufferB hiba: %d\n", err);
        return 0;
    }

    copy_cpu_to_cl();

    err = clEnqueueWriteBuffer(
        queue,
        bufferA,
        CL_TRUE,
        0,
        sizeof(BoidCL) * NUM_BOIDS,
        boidsCL,
        0,
        NULL,
        NULL
    );

    if (err != CL_SUCCESS) {
        printf("OpenCL kezdő buffer írási hiba: %d\n", err);
        return 0;
    }

    useBufferAAsInput = 1;

    printf("OpenCL inicializálva.\n");
    return 1;
}

void update_boids_opencl(void)
{
    cl_int err;

    cl_mem inputBuffer;
    cl_mem outputBuffer;

    if (useBufferAAsInput) {
        inputBuffer = bufferA;
        outputBuffer = bufferB;
    } else {
        inputBuffer = bufferB;
        outputBuffer = bufferA;
    }

    int numBoids = NUM_BOIDS;

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputBuffer);
    err |= clSetKernelArg(kernel, 2, sizeof(int), &numBoids);

    if (err != CL_SUCCESS) {
        printf("OpenCL kernel argument hiba: %d\n", err);
        return;
    }

    size_t globalSize = NUM_BOIDS;

    err = clEnqueueNDRangeKernel(
        queue,
        kernel,
        1,
        NULL,
        &globalSize,
        NULL,
        0,
        NULL,
        NULL
    );

    if (err != CL_SUCCESS) {
        printf("OpenCL kernel futtatási hiba: %d\n", err);
        return;
    }

    clFinish(queue);

    err = clEnqueueReadBuffer(
        queue,
        outputBuffer,
        CL_TRUE,
        0,
        sizeof(BoidCL) * NUM_BOIDS,
        boidsCL,
        0,
        NULL,
        NULL
    );

    if (err != CL_SUCCESS) {
        printf("OpenCL buffer olvasási hiba: %d\n", err);
        return;
    }

    copy_cl_to_cpu();

    useBufferAAsInput = !useBufferAAsInput;
}

void cleanup_opencl_boids(void)
{
    if (bufferA) clReleaseMemObject(bufferA);
    if (bufferB) clReleaseMemObject(bufferB);
    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);
    if (queue) clReleaseCommandQueue(queue);
    if (context) clReleaseContext(context);

    bufferA = NULL;
    bufferB = NULL;
    kernel = NULL;
    program = NULL;
    queue = NULL;
    context = NULL;
}