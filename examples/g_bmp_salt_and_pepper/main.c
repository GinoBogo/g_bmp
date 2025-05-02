// -----------------------------------------------------------------------------
// @file main.c
//
// @date April, 2025
//
// @author Gino Francesco Bogo
// -----------------------------------------------------------------------------

#include <stdint.h> // uint8_t, uint32_t
#include <time.h>   // time

#include "g_bmp.h"
#include "g_random.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    g_bmp_t image;

    g_bmp_link(&image);

    if (image.Create(&image, 256, 256)) {
        uint32_t width  = image.getWidth(&image);
        uint32_t height = image.getHeight(&image);

        g_random_seed(time(NULL));

        for (uint32_t y = 0; y < height; y++) {
            const uint32_t y_row = y * width;

            for (uint32_t x = 0; x < width; x++) {
                image.r.ptr[y_row + x] = (uint8_t)(g_random_range(0, 255));
                image.g.ptr[y_row + x] = (uint8_t)(g_random_range(0, 255));
                image.b.ptr[y_row + x] = (uint8_t)(g_random_range(0, 255));
            }
        }

        image.Save(&image, "g_bmp_salt_and_pepper.bmp");
    }

    image.Destroy(&image);

    if (image.Load(&image, "g_bmp_salt_and_pepper.bmp")) {
        if (image.toGrayscale(&image)) {
            image.Save(&image, "g_bmp_salt_and_pepper_grayscale.bmp");
        }
    }

    image.Destroy(&image);

    return 0;
}
