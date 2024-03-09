#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <tgmath.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "stb_image_write.h"

#define MAP_SIZE            256
#define OUTPUT_FNAME_EXT    ".binviz.png"
#define OUTPUT_FPATH_SIZE   256
#define CHUNK_SIZE          1024 * 8

static size_t map[MAP_SIZE][MAP_SIZE];
static int32_t pixels[MAP_SIZE][MAP_SIZE];

static FILE *xfopen(const char path[static 1])
{
    errno = 0;
    FILE *const fp = fopen(path, "rb");

    if (!fp) {
        fprintf(stderr, "Error: could not open file %s: %s.\n", path,
            errno ? strerror(errno) : "");
        exit(EXIT_FAILURE);
    }
    return fp;
}

static char *read_next_chunk(FILE *stream, char *chunk, size_t *size)
{
    if (size) {
        *size = 0;
    }
    
    const size_t rcount = fread(chunk, 1, CHUNK_SIZE, stream);

    if (rcount < CHUNK_SIZE) {
        if (!feof(stream)) {
            /* A read error occured. */
            return NULL;
        }

        if (rcount == 0) {
            return NULL;
        }
    }
    
    chunk[rcount] = '\0';

    if (size) {
        *size = rcount;
    }

    return chunk;
}

static _Bool process_file(FILE stream[static 1])
{
    char content[CHUNK_SIZE];
    char *p = NULL;
    size_t nbytes = 0;

    while ((p = read_next_chunk(stream, content, &nbytes))) {
        for (size_t i = 0; i < nbytes - 1; ++i) {
            uint8_t x = (uint8_t)content[i];
            uint8_t y = (uint8_t)content[i + 1];

            map[y][x] += 1;
        }

        float max = 0;

        for (size_t y = 0; y < MAP_SIZE; ++y) {
            for (size_t x = 0; x < MAP_SIZE; ++x) {
                float f = 0.0f;

                if (map[y][x] > 0) {
                    f = log(map[y][x]);
                }

                if (f > max) {
                    max = f;
                }
            }
        }

        for (size_t y = 0; y < MAP_SIZE; ++y) {
            for (size_t x = 0; x < MAP_SIZE; ++x) {
                float t = log(map[y][x]) / max;
                uint32_t b = t * MAP_SIZE;

                pixels[y][x] = 0XFF000000 | b | (b << 8) | (b << 16);
            }
        }
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
    
    if (!process_file(fp)) {
        fprintf(stderr, "Error: an unexpected error occured while reading.\n");
        fclose(fp);
        return EXIT_FAILURE;
    }

    fclose(fp);

    char out_fpath[OUTPUT_FPATH_SIZE];

    sprintf(out_fpath, "%s" OUTPUT_FNAME_EXT, input);
    const int rv =
        stbi_write_png(out_fpath, MAP_SIZE, MAP_SIZE, 4, pixels,
        MAP_SIZE * sizeof pixels[0][0]);

    if (rv == 0) {
        fprintf(stderr, "Error: could not save image %s.\n", out_fpath);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
