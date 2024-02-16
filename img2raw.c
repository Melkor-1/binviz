#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MAP_SIZE 256

#define OUTPUT_FNAME ".binviz.png"

#define OUTPUT_FPATH_SIZE 256

#define CHUNK_SIZE (8 * 1024)

static size_t map[MAP_SIZE][MAP_SIZE];
static int32_t pixels[MAP_SIZE][MAP_SIZE];

static FILE *xfopen(const char *path)
{
    errno = 0;
    FILE *const fp = fopen(path, "r");

    if (!fp) {
        fprintf(stderr, "Error: could not open file %s: %s.\n", path,
            errno ? strerror(errno) : "");
        exit(EXIT_FAILURE);
    }
    return fp;
}

static void fill_map(size_t nbytes, const char content[static nbytes])
{
    for (size_t i = 0; i < nbytes - 1; ++i) {
        uint8_t x = content[i];
        uint8_t y = content[i + 1];

        map[y][x] += 1;
    }
}

static bool process_input(FILE *stream)
{
    while (true) {
        char chunk[CHUNK_SIZE];
        const size_t nread = fread(chunk, 1, CHUNK_SIZE, stream);

        if (ferror(stream)) {
            return false;
        }

        if (nread == 0) {
            break;
        }

        fill_map(nread, chunk);
    }

    return true;
}

static void fill_pixels(void)
{
    float max = 0;

    for (size_t y = 0; y < MAP_SIZE; ++y) {
        for (size_t x = 0; x < MAP_SIZE; ++x) {
            float f = 0.0f;

            if (map[y][x] > 0) {
                f = logf(map[y][x]);
            }

            if (f > max) {
                max = f;
            }
        }
    }

    for (size_t y = 0; y < MAP_SIZE; ++y) {
        for (size_t x = 0; x < MAP_SIZE; ++x) {
            float t = logf(map[y][x]) / max;
            uint32_t b = t * 255;

            pixels[y][x] = 0XFF000000 | b | (b << 8) | (b << 16);
        }
    }
}

static int generate_output_image(const char *in_fpath) 
{
    char out_fpath[OUTPUT_FPATH_SIZE];

    sprintf(out_fpath, "%s" OUTPUT_FNAME, in_fpath);
    int rv =
        stbi_write_png(out_fpath, MAP_SIZE, MAP_SIZE, 4, pixels,
        MAP_SIZE * sizeof pixels[0][0]);

    if (!rv) {
        fprintf(stderr, "Error: could not save image %s.\n", out_fpath);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) 
{     
    /* Sanity check. POSIX requires the invoking process to pass a non-NULL argv[0].  
     */ 
    if (!argv[0]) {
        fprintf(stderr,
            "A NULL argv[0] was passed through an exec system call.\n");
        return EXIT_FAILURE;
    }

    if (argc > 2) {
        fprintf(stderr, "Error: no file provided.\n"
            "Usage: %s <filename>.\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *const fp = argv[1] ? xfopen(argv[1]) : stdin;
    
    if (!process_input(fp)) {
        fprintf(stderr, "Error: failed to read file %s: %s.\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }
    
    fill_pixels();
    return generate_output_image(argv[1] ? argv[1] : "out");
}
