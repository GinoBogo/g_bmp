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
        uint32_t width  = image.GetWidth(&image);
        uint32_t height = image.GetHeight(&image);

        for (uint32_t y = 0; y < height; y++) {
            const uint32_t y_row = y * width;

            float h = 255 * (y / (float)height);

            for (uint32_t x = 0; x < width; x++) {
                image.r.ptr[y_row + x] = (uint8_t)h;
                image.g.ptr[y_row + x] = 0;
                image.b.ptr[y_row + x] = 0;
            }
        }

        image.Save(&image, "g_bmp_redscale.bmp");
    }

    image.Destroy(&image);

    return 0;
}
