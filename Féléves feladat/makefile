CC = gcc

CFLAGS = -Wall -O2 -DCL_TARGET_OPENCL_VERSION=120 -Iinclude -I"C:/msys64/mingw64/include"

LDFLAGS = -L"C:/msys64/mingw64/lib" \
          -lmingw32 -lSDL2main -lSDL2 \
          -lopengl32 -lglu32 \
          -lOpenCL \
          -mconsole -lm

TARGET = opengl.exe
OBJS = main.o glad.o boids.o camera.o opencl_boids.o

BENCHMARK_TARGET = benchmark.exe
BENCHMARK_OBJS = benchmark.o boids.o opencl_boids.o camera.o glad.o

.PHONY: all clean run benchmark bench

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

bench: $(BENCHMARK_TARGET)

benchmark: bench

$(BENCHMARK_TARGET): $(BENCHMARK_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f *.o *.exe