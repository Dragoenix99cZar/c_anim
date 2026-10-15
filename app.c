#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// 1. Declare the width and height constants to match main.c
#define WIDTH 1024
#define HEIGHT 1024

// 2. Import external functions and buffers defined in main.c
extern unsigned char* get_buffer_pointer();
extern void set_alpha(int _alpha);
extern void compute_plasma(int frame_count); // Using Plasma effect as an example

// Helper function to create cross-platform directories
void create_directory(const char* dir) {
#if defined(_WIN32)
    mkdir(dir);
#else
    mkdir(dir, 0777);
#endif
}

int main(int argc, char *argv[]) {
    // Default configurations
    const char* output_dir = "frames";
    int total_frames = 120; // Default to 4 seconds of video at 30fps

    // Allow override via command line arguments
    if (argc > 1) {
        output_dir = argv[1];
    }
    if (argc > 2) {
        total_frames = atoi(argv[2]);
    }

    // Initialize environment
    create_directory(output_dir);
    set_alpha(255); // Force opaque output for standard image structures
    unsigned char* pixel_buffer = get_buffer_pointer();

    printf("Starting render: Generating %d frames into directory '%s'...\n", total_frames, output_dir);

    for (int frame = 0; frame < total_frames; frame++) {
        // 3. Compute the algorithmic visual layout for this specific frame
        // (You can swap this out with compute_julia, compute_warped, etc.)
        compute_plasma(frame);

        // 4. Construct sequential output filepath (e.g., frames/frame_0000.ppm)
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/frame_%04d.ppm", output_dir, frame);

        // 5. Open file in binary write mode
        FILE *fp = fopen(filepath, "wb");
        if (!fp) {
            fprintf(stderr, "Error: Could not open file %s for writing.\n", filepath);
            return 1;
        }

        // 6. Write PPM P6 Header (Format, Dimensions, Max Color Value)
        // P6 represents raw binary RGB data
        fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);

        // 7. Write data block frame-by-frame stripping out the Alpha channel
        // PPM expects a 3-byte RGB structure, while main.c provides a 4-byte RGBA buffer
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            int rgba_idx = i * 4;
            // Write Red, Green, Blue bytes sequentially
            fputc(pixel_buffer[rgba_idx + 0], fp);
            fputc(pixel_buffer[rgba_idx + 1], fp);
            fputc(pixel_buffer[rgba_idx + 2], fp);
        }

        fclose(fp);

        // Print progress tracker line
        if ((frame + 1) % 10 == 0 || frame == total_frames - 1) {
            printf(" Progress: [%d/%d] frames written.\n", frame + 1, total_frames);
        }
    }

    printf("Render complete! Standard sequence saved safely.\n");
    return 0;
}


// gcc -O3 -Wall -Wextra main.c app.c -o generator -lm
// .\generator.exe frames 1200
// ffmpeg -framerate 30 -i frames/frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p output.mp4   