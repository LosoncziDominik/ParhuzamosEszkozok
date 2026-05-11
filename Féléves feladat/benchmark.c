#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "boids.h"
#include "opencl_boids.h"

#define TEST_FRAMES 1000
#define REPEAT_COUNT 5

extern Boid boids[NUM_BOIDS];

static double get_time_seconds()
{
    return (double)clock() / CLOCKS_PER_SEC;
}

static void reset_boids_for_test()
{
    srand(1);

    for (int i = 0; i < NUM_BOIDS; i++) {
        boids[i].x = ((float)rand() / RAND_MAX) * 400.0f - 200.0f;
        boids[i].y = ((float)rand() / RAND_MAX) * 250.0f - 125.0f;
        boids[i].z = ((float)rand() / RAND_MAX) * 400.0f - 200.0f;

        boids[i].vx = ((float)rand() / RAND_MAX) * 0.4f - 0.2f;
        boids[i].vy = ((float)rand() / RAND_MAX) * 0.4f - 0.2f;
        boids[i].vz = ((float)rand() / RAND_MAX) * 0.4f - 0.2f;

        boids[i].targetSpeed = 0.5f;
        boids[i].speeds = 0.0f;
        boids[i].targetspeeds = 0.5f;
        boids[i].collision = 0;
    }
}

static double benchmark_cpu()
{
    reset_boids_for_test();

    double start = get_time_seconds();

    for (int i = 0; i < TEST_FRAMES; i++) {
        update_boids();
    }

    double end = get_time_seconds();

    return end - start;
}

static double benchmark_opencl()
{
    reset_boids_for_test();

    if (!init_opencl_boids()) {
        printf("OpenCL inicializálás sikertelen.\n");
        return -1.0;
    }

    double start = get_time_seconds();

    for (int i = 0; i < TEST_FRAMES; i++) {
        update_boids_opencl();
    }

    double end = get_time_seconds();

    cleanup_opencl_boids();

    return end - start;
}

int main(int argc, char* argv[])
{
    FILE* file = fopen("benchmark_results.csv", "w");

    if (!file) {
        printf("Nem sikerült létrehozni a benchmark_results.csv fájlt.\n");
        return 1;
    }

    fprintf(file, "run,method,num_boids,frames,total_time_sec,avg_frame_ms,fps\n");

    printf("Benchmark indul...\n");
    printf("Boidok száma: %d\n", NUM_BOIDS);
    printf("Frame-ek száma tesztenként: %d\n\n", TEST_FRAMES);

    for (int run = 1; run <= REPEAT_COUNT; run++) {
        double cpuTime = benchmark_cpu();

        double cpuAvgMs = (cpuTime / TEST_FRAMES) * 1000.0;
        double cpuFps = TEST_FRAMES / cpuTime;

        printf("CPU run %d: %.4f sec | %.4f ms/frame | %.2f FPS\n",
               run, cpuTime, cpuAvgMs, cpuFps);

        fprintf(file, "%d,CPU,%d,%d,%.6f,%.6f,%.6f\n",
                run, NUM_BOIDS, TEST_FRAMES, cpuTime, cpuAvgMs, cpuFps);

        double openclTime = benchmark_opencl();

        if (openclTime > 0.0) {
            double openclAvgMs = (openclTime / TEST_FRAMES) * 1000.0;
            double openclFps = TEST_FRAMES / openclTime;

            printf("OpenCL run %d: %.4f sec | %.4f ms/frame | %.2f FPS\n\n",
                   run, openclTime, openclAvgMs, openclFps);

            fprintf(file, "%d,OpenCL,%d,%d,%.6f,%.6f,%.6f\n",
                    run, NUM_BOIDS, TEST_FRAMES, openclTime, openclAvgMs, openclFps);
        }
    }

    fclose(file);

    printf("\nKész. Eredmény: benchmark_results.csv\n");

    return 0;
}