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

    if (image.Create(&image, 64, 64)) {
        uint32_t width  = image.getWidth(&image);
        uint32_t height = image.getHeight(&image);

        for (uint32_t y = 0; y < height; y++) {
            const uint32_t y_row = y * width;

            for (uint32_t x = 0; x < width; x++) {
                float h = 255 * (x / (float)width);

                image.r.ptr[y_row + x] = (uint8_t)h;
                image.g.ptr[y_row + x] = (uint8_t)h;
                image.b.ptr[y_row + x] = (uint8_t)h;
            }
        }

        image.Save(&image, "g_bmp_grayscale.bmp");
    }

    image.Destroy(&image);

    return 0;
}
