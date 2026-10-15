#define WIDTH 512
#define HEIGHT 512


#define WASM_EXPORT __attribute__((visibility("default")))

float sinf(float x);
float cosf(float x);
float sqrtf(float x);
float atan2f(float y, float x);

typedef struct {
    float x, y, z;
} Star;

#define MAX_STARS 300
Star stars[MAX_STARS];
int initialized = 0;
int ALPHA_VALUE = 30;

// Allocate the buffer statically in Wasm memory
// Size = Width * Height * 4 bytes (RGBA)
unsigned char pixel_buffer[WIDTH * HEIGHT * 4];

// Attribute tells Clang to export these functions to Wasm
WASM_EXPORT unsigned char* get_buffer_pointer() {
    return pixel_buffer;
}

WASM_EXPORT void set_alpha(int _alpha) {
    ALPHA_VALUE = _alpha;
}

WASM_EXPORT void compute_waves(int frame_count) {
    float time = frame_count * 0.05f;
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = (y * WIDTH + x) * 4;

            float factor = (sinf(x * 0.05f + time) + 1.0f) * 0.5f;

            pixel_buffer[idx + 0] = (unsigned char)(factor * 255); // Red
            pixel_buffer[idx + 1] = y;                    // Green
            pixel_buffer[idx + 2] = 128;                    // Blue
            pixel_buffer[idx + 3] = ALPHA_VALUE;                    // Alpha (Opaque)
        }
    }
}

WASM_EXPORT void compute_plasma(int frame_count) {
    float time = frame_count * 0.03f;

    // Directs the compiler to split rows across all available CPU threads automatically
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = (y * WIDTH + x) * 4;

            float nx = (x / (float)WIDTH) * 2.0f - 1.0f;
            float ny = (y / (float)HEIGHT) * 2.0f - 1.0f;

            float wave1 = sinf(nx * 4.0f + time);
            float wave2 = cosf(ny * 4.0f - time);
            float wave3 = sinf((nx + ny) * 2.0f + time * 1.5f);

            float total = (wave1 + wave2 + wave3) / 3.0f;
            float factor = (total + 1.0f) * 0.5f;

            pixel_buffer[idx + 0] = (unsigned char)(factor * 255);         
            pixel_buffer[idx + 1] = (unsigned char)((1.0f - factor) * 255); 
            pixel_buffer[idx + 2] = (unsigned char)(sinf(time) * 127 + 128); 
            pixel_buffer[idx + 3] = ALPHA_VALUE;
        }
    }
}

WASM_EXPORT void compute_julia(int frame_count) {
    float time = frame_count * 0.01f;

    // Constant parameters that change over time to mutate the fractal shape
    float cx = -0.7f + sinf(time * 0.3f) * 0.1f;
    float cy = 0.27015f + cosf(time * 0.2f) * 0.05f;

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = (y * WIDTH + x) * 4;

            // Map pixel coordinates to complex plane (-1.5 to 1.5)
            float zx = 1.5f * (x - WIDTH / 2.0f) / (WIDTH / 2.0f);
            float zy = 1.5f * (y - HEIGHT / 2.0f) / (HEIGHT / 2.0f);

            int i = 0;
            int max_iterations = 32;

            // Iterate the complex function Z = Z^2 + C
            while (zx * zx + zy * zy < 4.0f && i < max_iterations) {
                float xtemp = zx * zx - zy * zy + cx;
                zy = 2.0f * zx * zy + cy;
                zx = xtemp;
                i++;
            }

            // Color map based on escape velocity (iterations reached)
            float ns = (float)i / (float)max_iterations;

            pixel_buffer[idx + 0] = (unsigned char)(ns * 255);                     // Red
            pixel_buffer[idx + 1] = (unsigned char)(sinf(ns * 3.14f) * 255);       // Green
            pixel_buffer[idx + 2] = (unsigned char)(sqrtf(ns) * 255);              // Blue
            pixel_buffer[idx + 3] = ALPHA_VALUE;
        }
    }
}

WASM_EXPORT void compute_warped(int frame_count) {
    float time = frame_count * 0.02f;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = (y * WIDTH + x) * 4;

            float u = x / (float)WIDTH;
            float v = y / (float)HEIGHT;

            // First layer of warping distortion (First domain)
            float qx = sinf(u * 5.0f + time * 0.5f) + cosf(v * 3.0f);
            float qy = cosf(v * 5.0f - time * 0.3f) + sinf(u * 2.0f);

            // Second layer feeding from the first (Warped Domain)
            float rx = sinf((u + qx) * 4.0f + time);
            float ry = cosf((v + qy) * 4.0f - time * 0.7f);

            // Final scalar field value
            float factor = (sinf(rx * ry * 3.0f) + 1.0f) * 0.5f;

            // Dynamic RGB assignment for trippy fluid gradients
            pixel_buffer[idx + 0] = (unsigned char)(factor * 255);
            pixel_buffer[idx + 1] = (unsigned char)(sinf(factor * 3.14f) * 255);
            pixel_buffer[idx + 2] = (unsigned char)(cosf(time) * 127 + 128);
            pixel_buffer[idx + 3] = ALPHA_VALUE;
        }
    }
}

// EFFECT 2: Concentric Tunnel Rings
WASM_EXPORT void compute_tunnel(int frame_count) {
    float time = frame_count * 0.1f;
    float cx = WIDTH / 2.0f;
    float cy = HEIGHT / 2.0f;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int idx = (y * WIDTH + x) * 4;
            float dx = x - cx;
            float dy = y - cy;
            float distance = sqrtf(dx * dx + dy * dy);
            float factor = (sinf(distance * 0.1f - time) + 1.0f) * 0.5f;

            pixel_buffer[idx + 0] = (unsigned char)(factor * 255);
            pixel_buffer[idx + 1] = (unsigned char)(factor * 50);
            pixel_buffer[idx + 2] = (unsigned char)(factor * 180);
            pixel_buffer[idx + 3] = ALPHA_VALUE;
        }
    }
}


/*
 clang --target=wasm32 -O3 -nostdlib -Wl,--no-entry -Wl,--export-all -o renderer.wasm main.c

 cmd: clang --target=wasm32 -O3 -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined -o renderer.wasm main.c
 powershell:  comma issue, use quots
    clang --target=wasm32 -O3 -nostdlib "-Wl,--no-entry" "-Wl,--export-all" "-Wl,--allow-undefined" -o renderer.wasm main.c

 clang -Wall -Wextra --target=wasm32 -o wasm.o main.c
 wasm-ld -m wasm32 --no-entry --export-all --allow-undefined -o renderer.wasm wasm.o
 */
