#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <tgmath.h>

/* C2X/C23 or later? */
#if defined(__STDC__VERSION) && __STDC_VERSION__ >= 202000L
    #include <stddef.h>     /* nullptr_t */
#else
    #include <stdbool.h>    /* bool, true. false */
    #define nullptr ((void *)0)
    typedef void * nullptr_t;
#endif                      /* nullptr/nullptr_t */

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "stb_image_write.h"

#define MAP_SIZE            256
#define OUTPUT_FNAME_EXT    ".binviz.png"
#define OUTPUT_FPATH_SIZE   256
#define CHUNK_SIZE          1024 * 8

static int32_t pixels[MAP_SIZE][MAP_SIZE];

static FILE *xfopen(const char path[static 1])
{
    errno = 0;
    FILE *const fp = fopen(path, "rb");

    if (!fp) {
        fprintf(stderr, "Error: could not open file %s: %s.\n", path,
            errno ? strerror(errno) : "unknown error");
        exit(EXIT_FAILURE);
    }
    return fp;
}

static bool read_next_chunk(FILE stream[restrict static 1], 
                            char chunk[restrict static CHUNK_SIZE], 
                            size_t *size)
{
    if (size) {
        *size = 0;
    }
    
    const size_t rcount = fread(chunk, 1, CHUNK_SIZE, stream);

    if (rcount < CHUNK_SIZE) {
        if (!feof(stream)) {
            /* A read error occurred. */
            return false;
        }

        if (rcount == 0) {
            return false;
        }
    }
    
    chunk[rcount] = '\0';

    if (size) {
        *size = rcount;
    }

    return true;;
}

static void render_pixels(size_t max_val)
{
    float max = 0;
    
    if (max_val) {
        max = log(max_val);
    } else {
        return;
    }
    
    for (size_t y = 0; y < MAP_SIZE; ++y) {
        for (size_t x = 0; x < MAP_SIZE; ++x) {
            const float t = log(pixels[y][x]) / max;
            const uint32_t b = t * MAP_SIZE;

            pixels[y][x] = 0xff000000 | b | (b << 8u) | (b << 16u);
        }
    }
}

static bool process_file(FILE stream[static 1], int32_t *max_val)
{
    char content[CHUNK_SIZE];
    size_t nbytes = 0;
    bool is_first = true;
    uint8_t last_byte = 0;
    *max_val = 0;

    while (true) {
        bool rv = read_next_chunk(stream, content, &nbytes);

        if (!rv) {
            break;
        }
        
        if (!is_first) {
            pixels[last_byte][(uint8_t)content[0]] += 1;
        } else {
            is_first = false;
        }

        for (size_t i = 0; i < nbytes - 1; ++i) {
            const uint8_t x = (uint8_t)content[i];
            const uint8_t y = (uint8_t)content[i + 1];

            pixels[y][x] += 1;

            if (pixels[y][x] > *max_val) {
                *max_val = pixels[y][x];
            }
        }

        last_byte = (uint8_t)content[nbytes - 1];
    }

    return feof(stream);
}

int main(int argc, char **argv)
{
    /* Sanity check. POSIX requires the invoking process to pass a non-NULL 
     * argv[0]. 
     */
    if (!argv[0]) {
        fprintf(stderr,
            "A NULL argv[0] was passed through an exec system call.\n");
        return EXIT_FAILURE;
    }

    if (argc != 2) {
        fprintf(stderr, "Error: no file provided.\n"
            "Usage: %s <filename>.\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *const input = argv[1];
    FILE *const fp = xfopen(input);
    int32_t max_val = 0;

    if (!process_file(fp, &max_val)) {
        fprintf(stderr, "Error: an unexpected error occurred while reading.\n");
        fclose(fp);
        return EXIT_FAILURE;
    }
    
    fclose(fp);
    render_pixels(max_val);

    char out_fpath[OUTPUT_FPATH_SIZE];

    snprintf(out_fpath, OUTPUT_FPATH_SIZE, "%s" OUTPUT_FNAME_EXT, input);
    const int rv =
        stbi_write_png(out_fpath, MAP_SIZE, MAP_SIZE, sizeof pixels[0][0], pixels,
        MAP_SIZE * sizeof pixels[0][0]);

    if (rv == 0) {
        fprintf(stderr, "Error: could not generate image %s.\n", out_fpath);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
