// -----------------------------------------------------------------------------
// @file g_bmp.c
//
// @date April, 2025
//
// @author Gino Francesco Bogo
// -----------------------------------------------------------------------------

#include "g_bmp.h"

#include <assert.h> // assert
#include <stddef.h> // NULL
#include <stdio.h>  // FILE, fopen, fclose, fread, fwrite
#include <stdlib.h> // free, malloc
#include <string.h> // memset

// -----------------------------------------------------------------------------

static void __unsafe_reset(g_bmp_t *self) {
    assert(self != NULL);
    // variables
    (void)memset(&self->r, 0, sizeof(g_bmp_channel_t));
    (void)memset(&self->g, 0, sizeof(g_bmp_channel_t));
    (void)memset(&self->b, 0, sizeof(g_bmp_channel_t));

    (void)memset(&self->bmp_header, 0, sizeof(g_bmp_header_t));
    (void)memset(&self->dib_header, 0, sizeof(g_dib_header_t));

    // intrinsic
    self->_is_safe = false;
}

static bool Create(struct g_bmp_t *self, int32_t width, int32_t height) {
    bool rvalue = false;

    if (self != NULL) {
        self->Destroy(self);

        self->r.ptr = (uint8_t *)malloc(width * height * sizeof(uint8_t));
        self->g.ptr = (uint8_t *)malloc(width * height * sizeof(uint8_t));
        self->b.ptr = (uint8_t *)malloc(width * height * sizeof(uint8_t));

        rvalue = true;

        rvalue = rvalue && (self->r.ptr != NULL);
        rvalue = rvalue && (self->g.ptr != NULL);
        rvalue = rvalue && (self->b.ptr != NULL);

        if (rvalue) {
            self->r.width  = width;
            self->r.height = height;

            self->g.width  = width;
            self->g.height = height;

            self->b.width  = width;
            self->b.height = height;

            const uint32_t bits       = (uint32_t)(3 * 8); // 24-bit color space
            const uint32_t bmp_h_size = (uint32_t)sizeof(g_bmp_header_t);
            const uint32_t dib_h_size = (uint32_t)sizeof(g_dib_header_t);
            const uint32_t row_size   = ((bits * width + 31) / 32) * 4; // 32-bit aligned
            const uint32_t image_size = row_size * height;
            const uint32_t total_size = (bmp_h_size + dib_h_size) + image_size;

            self->bmp_header.type       = 0x4D42; // "BM"
            self->bmp_header.size       = total_size;
            self->bmp_header.reserved_1 = 0;
            self->bmp_header.reserved_2 = 0;
            self->bmp_header.offset     = (bmp_h_size + dib_h_size);

            self->dib_header.size             = dib_h_size;
            self->dib_header.width            = width;
            self->dib_header.height           = height;
            self->dib_header.planes           = 1;
            self->dib_header.bits             = bits; // 24-bit
            self->dib_header.compression      = 0;    // uncompressed
            self->dib_header.image_size       = image_size;
            self->dib_header.x_resolution     = 2835; // 72 DPI
            self->dib_header.y_resolution     = 2835; // 72 DPI
            self->dib_header.colors           = 0;    // no palette
            self->dib_header.important_colors = 0;    // all colors are important

            self->_is_safe = true;
        }
    }

    return rvalue;
}

static void Destroy(struct g_bmp_t *self) {
    if (self != NULL) {
        free(self->r.ptr);
        free(self->g.ptr);
        free(self->b.ptr);

        __unsafe_reset(self);
    }
}

static bool Load(struct g_bmp_t *self, const char *filename) {
    bool rvalue = false;

    if (self != NULL) {
        FILE *file = fopen(filename, "rb");

        if (file != NULL) {
            self->Destroy(self);

            fread(&self->bmp_header, sizeof(g_bmp_header_t), 1, file);
            fread(&self->dib_header, sizeof(g_dib_header_t), 1, file);

            uint32_t width  = (uint32_t)self->dib_header.width;
            uint32_t height = (uint32_t)self->dib_header.height;

            if (self->Create(self, width, height)) {
                for (uint32_t y = 0; y < height; ++y) {
                    const uint32_t y_row = y * width;

                    for (uint32_t x = 0; x < width; ++x) {
                        fread(self->r.ptr + y_row + x, sizeof(uint8_t), 1, file);
                        fread(self->g.ptr + y_row + x, sizeof(uint8_t), 1, file);
                        fread(self->b.ptr + y_row + x, sizeof(uint8_t), 1, file);
                    }
                }

                rvalue = true;
            }

            fclose(file);
        }
    }

    return rvalue;
}

static bool Save(struct g_bmp_t *self, const char *filename) {
    bool rvalue = false;

    if ((self != NULL) && self->_is_safe) {
        FILE *file = fopen(filename, "wb");

        if (file != NULL) {
            fwrite(&self->bmp_header, sizeof(g_bmp_header_t), 1, file);
            fwrite(&self->dib_header, sizeof(g_dib_header_t), 1, file);

            uint32_t width  = (uint32_t)self->r.width;
            uint32_t height = (uint32_t)self->r.height;

            for (uint32_t y = 0; y < height; ++y) {
                const uint32_t y_row = y * width;

                for (uint32_t x = 0; x < width; ++x) {
                    fwrite(self->r.ptr + y_row + x, sizeof(uint8_t), 1, file);
                    fwrite(self->g.ptr + y_row + x, sizeof(uint8_t), 1, file);
                    fwrite(self->b.ptr + y_row + x, sizeof(uint8_t), 1, file);
                }
            }

            fclose(file);

            rvalue = true;
        }
    }

    return rvalue;
}

static int32_t GetWidth(struct g_bmp_t *self) {
    if ((self != NULL) && self->_is_safe) {
        return self->r.width;
    }
    return 0;
}

static int32_t GetHeight(struct g_bmp_t *self) {
    if ((self != NULL) && self->_is_safe) {
        return self->r.height;
    }
    return 0;
}

void g_bmp_link(g_bmp_t *self) {
    if (self != NULL) {
        // variables & intrinsic
        __unsafe_reset(self);

        // functions
        self->Create    = Create;
        self->Destroy   = Destroy;
        self->Load      = Load;
        self->Save      = Save;
        self->GetWidth  = GetWidth;
        self->GetHeight = GetHeight;
    }
}

// -----------------------------------------------------------------------------
// End of File
