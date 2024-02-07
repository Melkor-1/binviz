#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define OUTPUT_FPATH_SIZE 256

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
    int width, height, channels;
    const uint32_t *const pixels =
        (const uint32_t * const) stbi_load(input, &width, &height, &channels, 4);

    if (!pixels) {
        fprintf(stderr, "Error: could not load file %s.\n", input);
        return EXIT_FAILURE;
    }

    char output[OUTPUT_FPATH_SIZE];
    sprintf(output, "%s.raw", input);

    errno = 0;
    FILE *const out_fp = fopen(output, "w");

    if (!out_fp) {
        fprintf(stderr, "Error: could not open file %s: %s.\n", output,
            errno ? strerror(errno) : "");
        stbi_image_free(pixels);
        return EXIT_FAILURE;
    }

    size_t total_sz = width * height * sizeof (uint32_t);
    size_t nwritten = fwrite(pixels, 1, width * height * channels, out_fp);

    if (nwritten != total_sz) {
        fprintf(stderr, "Error: failed to generate %s.", output);
        fclose(out_fp);
        stbi_image_free(pixels);
        return EXIT_FAILURE;
    }

    stbi_image_free(pixels);
    fclose(out_fp);
    return EXIT_SUCCESS;
}
