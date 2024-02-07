#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define MAP_SIZE 256
#define OUTPUT_FNAME ".binviz.png"
#define OUTPUT_FPATH_SIZE 256

static size_t map[MAP_SIZE][MAP_SIZE];
static int32_t pixels[MAP_SIZE][MAP_SIZE];

static char *read_file(FILE * fp, size_t *nbytes)
{
    static const size_t page_size = 4096;
    char *content = NULL;
    size_t len = 0;

    for (size_t rcount = 1; rcount; len += rcount) {
        void *const tmp = realloc(content, len + page_size);

        if (!tmp) {
            free(content);
            content = NULL;
/* ENOMEM is not a C standard error code, yet quite common. */
#ifdef ENOMEM
            errno = ENOMEM;
#endif
            return NULL;
        }
        content = tmp;
        rcount = fread(content + len, 1, page_size - 1, fp);

        if (ferror(fp)) {
            free(content);
            content = NULL;
#ifdef ENOMEM
            errno = ENOMEM;
#endif
            return NULL;
        }
    }
    content[len] = '\0';
    *nbytes = len;
    return content;
}

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

static char *xread_file(const char *fpath, FILE * stream, size_t *nbytes)
{
    size_t size = 0;
    char *const content = read_file(stream, &size);

    if (!content) {
        fprintf(stderr, "Error: failed to read file %s: %s.\n", fpath,
            errno ? strerror(errno) : "");
        fclose(stream);
        exit(EXIT_FAILURE);
    }

    if (!size) {
        fprintf(stderr, "Error: the file %s is empty.\n", fpath);
        fclose(stream);
        exit(EXIT_FAILURE);
    }
    *nbytes = size;
    return content;
}

static void process_file(size_t nbytes, const char content[static nbytes])
{
    for (size_t i = 0; i < nbytes - 1; ++i) {
        uint8_t x = content[i];
        uint8_t y = content[i + 1];

        map[y][x] += 1;
    }

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

    const char *input = argv[1];
    FILE *const fp = xfopen(input);
    size_t nbytes = 0;
    char *const content = xread_file(input, fp, &nbytes);

    process_file(nbytes, content);
    fclose(fp);
    free(content);

    char out_fpath[OUTPUT_FPATH_SIZE];

    sprintf(out_fpath, "%s" OUTPUT_FNAME, input);
    int rv =
        stbi_write_png(out_fpath, MAP_SIZE, MAP_SIZE, 4, pixels,
        MAP_SIZE * sizeof pixels[0][0]);

    if (!rv) {
        fprintf(stderr, "Error: could not save image %s.\n", out_fpath);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
