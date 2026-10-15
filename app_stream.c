#include <stdio.h>
#include <stdlib.h>

// Windows-specific headers for system I/O operation overrides
#include <io.h>
#include <fcntl.h>

#define WIDTH 1024
#define HEIGHT 1024

// Import the external WebAssembly engine pipelines defined in main.c
extern unsigned char* get_buffer_pointer();
extern void set_alpha(int _alpha);
extern void compute_plasma(int frame_count); // Swap with compute_julia, compute_matrix, etc.

int main(int argc, char *argv[]) {
    // 1. Force Windows standard output stream into strict raw binary translation mode.
    // This stops Windows from mutating line endings (LF '\n' -> CR+LF '\r\n'),
    // which completely prevents byte corruption in video decoding pipes.
    _setmode(_fileno(stdout), _O_BINARY);

    // 2. Read frame targets or fallback safely to a baseline (e.g., 10 seconds at 30 FPS)
    int total_frames = 300; 
    if (argc > 1) {
        total_frames = atoi(argv[1]);
    }

    // 3. Initialize visual environments
    set_alpha(255); // Opaque visual profiles
    unsigned char* pixel_buffer = get_buffer_pointer();

    // 4. Heavy performance rendering loop
    for (int frame = 0; frame < total_frames; frame++) {
        compute_plasma(frame);

        // 5. Serialize data chunk directly to standard out
        // PPM/FFmpeg expects 3-byte RGB24 data. Stripping the alpha channel 
        // byte-by-byte keeps standard layout processing lightweight.
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            int rgba_idx = i * 4;
            
            putchar(pixel_buffer[rgba_idx + 0]); // Red Channel
            putchar(pixel_buffer[rgba_idx + 1]); // Green Channel
            putchar(pixel_buffer[rgba_idx + 2]); // Blue Channel
        }
    }

    return 0;
}


// cmd: streamer.exe 9999 | ffplay -f rawvideo -pixel_format rgb24 -video_size 1024x1024 -framerate 30 -i -

// cmd: .\streamer.exe 300 | ffmpeg -y -f rawvideo -pix_fmt rgb24 -s 1024x1024 -framerate 30 -i - -c:v libx264 -pix_fmt yuv420p output.mp4