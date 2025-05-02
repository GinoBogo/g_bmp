// -----------------------------------------------------------------------------
// @file main.c
//
// @date April, 2025
//
// @author Gino Francesco Bogo
// -----------------------------------------------------------------------------

#include <stdint.h>

#include "g_bmp.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    g_bmp_t image;

    g_bmp_link(&image);

    if (image.Load(&image, "sample-00.bmp")) {
        if (image.toGrayscale(&image)) {
            image.Save(&image, "sample-00_grayscale.bmp");
        }
    }

    image.Destroy(&image);

    // NOTE: use of aggressive laplacian kernel
    // clang-format off
    const float kernel_ptr[] = {
         0, -2,  0,
        -2,  8, -2,
         0, -2,  0
    };
    // clang-format on
    const uint32_t kernel_len = sizeof(kernel_ptr) / sizeof(kernel_ptr[0]);

    if (image.Load(&image, "sample-00.bmp")) {
        g_bmp_t output;

        g_bmp_link(&output);

        if (image.applyKernel(&image, &output, (float *)kernel_ptr, kernel_len)) {
            output.Save(&output, "sample-00_laplacian.bmp");
        }

        output.Destroy(&output);
    }

    image.Destroy(&image);

    if (image.Load(&image, "sample-01.bmp")) {
        if (image.toGrayscale(&image)) {
            image.Save(&image, "sample-01_grayscale.bmp");
        }
    }

    image.Destroy(&image);

    if (image.Load(&image, "sample-01.bmp")) {
        g_bmp_t output;

        g_bmp_link(&output);

        if (image.applyKernel(&image, &output, (float *)kernel_ptr, kernel_len)) {
            output.Save(&output, "sample-01_laplacian.bmp");
        }

        output.Destroy(&output);
    }

    image.Destroy(&image);

    return 0;
}
