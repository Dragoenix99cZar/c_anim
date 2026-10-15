#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>

#define WIDTH 512
#define HEIGHT 512

extern unsigned char* get_buffer_pointer();
extern void set_alpha(int _alpha);
extern void compute_plasma(int frame_count);
extern void compute_waves(int frame_count);
extern void compute_julia(int frame_count);

// Allocate a dedicated layout memory block to organize raw RGB channel sequences safely
unsigned char rgb_staging_buffer[WIDTH * HEIGHT * 3];

int main(int argc, char *argv[]) {
    _setmode(_fileno(stdout), _O_BINARY);

    int total_frames = 300; 
    if (argc > 1) {
        total_frames = atoi(argv[1]);
    }

    set_alpha(255); 
    unsigned char* pixel_buffer = get_buffer_pointer();

    // Only print this once from the master thread
    // #pragma omp master
    // {
    //     fprintf(stderr, "=== Multi-Core Optimization Engaged ===\n");
    //     fprintf(stderr, "Rendering video using exactly [ %d ] CPU threads.\n", omp_get_num_threads());
    //     fprintf(stderr, "=======================================\n");
    // }

    for (int frame = 0; frame < total_frames; frame++) {
        // 1. Compute visual graphics mathematically (internally multithreaded within mainParallel.c)
        compute_plasma(frame);
        // if(frame < 0.5 * total_frames) compute_waves(frame);
        // else compute_julia(frame);

        // 2. Extract and format the channels to our staging area using multiple CPU cores safely
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            int rgba_idx = i * 4;
            int rgb_idx  = i * 3;
            rgb_staging_buffer[rgb_idx + 0] = pixel_buffer[rgba_idx + 0]; // R
            rgb_staging_buffer[rgb_idx + 1] = pixel_buffer[rgba_idx + 1]; // G
            rgb_staging_buffer[rgb_idx + 2] = pixel_buffer[rgba_idx + 2]; // B
        }

        // 3. Output the entire processed block to stdout using one single, fast block write operation
        fwrite(rgb_staging_buffer, 1, WIDTH * HEIGHT * 3, stdout);
    }

    return 0;
}

// cmd: gcc -O3 -fopenmp -Wall -Wextra mainParallel.c app_stream_parallel.c -o streamerp -lm

//run cmd: .\streamerp.exe 300 | ffmpeg -y -f rawvideo -pix_fmt rgb24 -s 1024x1024 -framerate 30 -i - -c:v libx264 -pix_fmt yuv420p outputp.mp4

//Save frames:
// streamerp.exe 200 | ffmpeg -y -f rawvideo -pix_fmt rgb24 -video_size 2560x1600 -framerate 30 -i - -filter_complex split=2[v1][v2] -map [v1] -c:v libx264 -pix_fmt yuv420p outputWall.mp4 -map [v2] -c:v png frames_output/frame_%04d.png