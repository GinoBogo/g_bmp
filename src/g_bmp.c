// -----------------------------------------------------------------------------
// @file g_bmp.c
//
// @date April, 2025
//
// @author Gino Francesco Bogo
// -----------------------------------------------------------------------------

#include "g_bmp.h"

#include <assert.h> // assert
#include <math.h>   // M_2_PI, M_PI, fmaxf, fminf, sqrtf
#include <stddef.h> // NULL
#include <stdio.h>  // FILE, fopen, fclose, fread, fwrite
#include <stdlib.h> // free, malloc
#include <string.h> // memset

// -----------------------------------------------------------------------------
// Internal Functions
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

static g_hsi_t __rgb_to_hsi(g_rgb_t rgb) {
    g_hsi_t hsi = {0};

    const uint8_t R = rgb.r;
    const uint8_t G = rgb.g;
    const uint8_t B = rgb.b;

    const uint8_t  max_RGB = (R > G) ? ((R > B) ? R : B) : ((G > B) ? G : B);
    const uint8_t  min_RGB = (R < G) ? ((R < B) ? R : B) : ((G < B) ? G : B);
    const uint8_t  delta   = max_RGB - min_RGB;
    const uint16_t sum     = R + G + B;

    // Intensity (I)
    hsi.i = (float)sum / 765.0f; // 3 * 255 = 765

    // Saturation (S)
    if (delta >= 10) { // Threshold
        hsi.s = 1.0f - ((float)min_RGB / 255.0f) / hsi.i;
    }

    // Hue (H)
    if (delta >= 10) { // Threshold
        float hue;
        if (max_RGB == R) {
            hue = (G - B) / (float)delta;
        } else if (max_RGB == G) {
            hue = 2.0f + (B - R) / (float)delta;
        } else {
            hue = 4.0f + (R - G) / (float)delta;
        }

        // Scale hue to [0, 2π]
        hsi.h = (hue < 0.0f) ? hue + (float)(2.0f * M_PI) : hue;
        hsi.h *= (float)M_PI / 3.0f;
    }

    return hsi;
}

// -----------------------------------------------------------------------------
// Linked Functions
// -----------------------------------------------------------------------------

static bool Create(struct g_bmp_t *self, int32_t width, int32_t height) {
    bool rvalue = false;

    if ((self != NULL) && (width > 0) && (height > 0)) {
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
            const uint32_t row_size   = ((bits * width * 3 + 3) & ~3); // 32-bit aligned
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

    if ((self != NULL) && (filename != NULL)) {
        FILE *file = fopen(filename, "rb");

        if (file != NULL) {
            self->Destroy(self);

            fread(&self->bmp_header, sizeof(g_bmp_header_t), 1, file);
            fread(&self->dib_header, sizeof(g_dib_header_t), 1, file);

            const int32_t width  = self->dib_header.width;
            const int32_t height = self->dib_header.height;

            if (self->Create(self, width, height)) {
                const int32_t row_size = (width * 3 + 3) & ~3; // 32-bit aligned

                uint8_t *buffer = (uint8_t *)malloc(row_size);

                if (buffer != NULL) {
                    for (int32_t y = 0; y < height; ++y) {
                        const int32_t y_row = (height - 1 - y) * width;

                        if (fread(buffer, sizeof(uint8_t), row_size, file) != (uint32_t)row_size) {
                            free(buffer);
                            rvalue = false;
                            break;
                        }

                        for (int32_t x = 0; x < width; ++x) {
                            const int32_t x_col = x * 3;

                            self->b.ptr[y_row + x] = buffer[x_col + 0];
                            self->g.ptr[y_row + x] = buffer[x_col + 1];
                            self->r.ptr[y_row + x] = buffer[x_col + 2];
                        }
                    }

                    self->r.width  = width;
                    self->r.height = height;

                    self->g.width  = width;
                    self->g.height = height;

                    self->b.width  = width;
                    self->b.height = height;

                    free(buffer);
                    rvalue = true;
                }
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

            const int32_t width  = self->r.width;
            const int32_t height = self->r.height;

            const int32_t row_size = (width * 3 + 3) & ~3; // 32-bit aligned

            uint8_t *buffer = (uint8_t *)malloc(row_size);

            if (buffer != NULL) {
                for (int32_t y = 0; y < height; ++y) {
                    const int32_t y_row = (height - 1 - y) * width;

                    for (int32_t x = 0; x < width; ++x) {
                        const int32_t x_col = x * 3;

                        buffer[x_col + 0] = self->b.ptr[y_row + x];
                        buffer[x_col + 1] = self->g.ptr[y_row + x];
                        buffer[x_col + 2] = self->r.ptr[y_row + x];
                    }

                    if (fwrite(buffer, sizeof(uint8_t), row_size, file) != (uint32_t)row_size) {
                        free(buffer);
                        rvalue = false;
                        break;
                    }
                }

                free(buffer);
                rvalue = true;
            }

            fclose(file);
        }
    }

    return rvalue;
}

static int32_t getWidth(struct g_bmp_t *self) {
    if ((self != NULL) && self->_is_safe) {
        return self->r.width;
    }
    return 0;
}

static int32_t getHeight(struct g_bmp_t *self) {
    if ((self != NULL) && self->_is_safe) {
        return self->r.height;
    }
    return 0;
}

static bool toGrayscale(struct g_bmp_t *self) {
    bool rvalue = false;

    if ((self != NULL) && self->_is_safe) {
        const int32_t width  = self->r.width;
        const int32_t height = self->r.height;

        for (int32_t y = 0; y < height; ++y) {
            const int32_t y_row = (height - 1 - y) * width;

            for (int32_t x = 0; x < width; ++x) {
                const uint8_t r = self->r.ptr[y_row + x];
                const uint8_t g = self->g.ptr[y_row + x];
                const uint8_t b = self->b.ptr[y_row + x];

                // NOTE: luminance (Y) formula
                const uint8_t gray = (uint8_t)((r * 0.299) + (g * 0.587) + (b * 0.114));

                self->r.ptr[y_row + x] = gray;
                self->g.ptr[y_row + x] = gray;
                self->b.ptr[y_row + x] = gray;
            }
        }

        rvalue = true;
    }

    return rvalue;
}

static bool applyFilter(struct g_bmp_t *self, struct g_bmp_t *output, float *filter_ptr, int32_t filter_len) {
    bool rvalue = false;

    if ((self != NULL) && self->_is_safe) {
        rvalue = true;

        const int32_t filter_dim = (int32_t)sqrtf((float)filter_len);
        const int32_t filter_pad = (filter_dim - 1) / 2;

        rvalue = rvalue && (output != NULL);
        rvalue = rvalue && (filter_ptr != NULL);
        rvalue = rvalue && (filter_len > 1);
        rvalue = rvalue && (filter_dim * filter_dim == filter_len);
        rvalue = rvalue && (filter_dim % 2 == 1); // odd-sized filters only

        if (rvalue) {
            const int32_t width  = self->r.width;
            const int32_t height = self->r.height;

            rvalue = output->Create(output, width, height);

            if (rvalue) {
                for (int32_t y = -filter_pad; y < height - filter_pad; ++y) {
                    const int32_t dst_y = y + filter_pad;

                    for (int32_t x = -filter_pad; x < width - filter_pad; ++x) {
                        const int32_t dst_x = x + filter_pad;

                        float sum_r = 0.0f;
                        float sum_g = 0.0f;
                        float sum_b = 0.0f;

                        for (int32_t ky = 0; ky < filter_dim; ++ky) {
                            // Clamp to edge for y coordinate
                            const int32_t src_y = (y + ky < 0)       ? 0          //
                                                : (y + ky >= height) ? height - 1 //
                                                                     : y + ky;    //

                            for (int32_t kx = 0; kx < filter_dim; ++kx) {
                                // Clamp to edge for x coordinate
                                const int32_t src_x = (x + kx < 0)      ? 0         //
                                                    : (x + kx >= width) ? width - 1 //
                                                                        : x + kx;   //

                                const int32_t filter_idx = ky * filter_dim + kx;
                                const float   filter_val = filter_ptr[filter_idx];

                                const int32_t pixel_idx = src_y * width + src_x;

                                sum_r += ((float)(self->r.ptr[pixel_idx]) * filter_val);
                                sum_g += ((float)(self->g.ptr[pixel_idx]) * filter_val);
                                sum_b += ((float)(self->b.ptr[pixel_idx]) * filter_val);
                            }
                        }

                        const int32_t dst_idx = dst_y * width + dst_x;

                        output->r.ptr[dst_idx] = (uint8_t)fminf(fmaxf(sum_r, 0.0f), 255.0f);
                        output->g.ptr[dst_idx] = (uint8_t)fminf(fmaxf(sum_g, 0.0f), 255.0f);
                        output->b.ptr[dst_idx] = (uint8_t)fminf(fmaxf(sum_b, 0.0f), 255.0f);
                    }
                }
            }
        }
    }

    return rvalue;
}

static bool selectColor(struct g_bmp_t *self, struct g_bmp_t *output, g_rgb_t color, g_hsi_t range) {
    bool rvalue = false;

    if ((self != NULL) && self->_is_safe) {
        rvalue = true;

        const int32_t width  = (int32_t)self->r.width;
        const int32_t height = (int32_t)self->r.height;

        if (output->Create(output, width, height)) {
            const g_hsi_t ref = __rgb_to_hsi(color);

            for (int32_t y = 0; y < height; ++y) {
                const int32_t y_row = y * width;

                for (int32_t x = 0; x < width; ++x) {
                    const int32_t pixel_idx = y_row + x;

                    g_rgb_t rgb = {
                        .r = self->r.ptr[pixel_idx],
                        .g = self->g.ptr[pixel_idx],
                        .b = self->b.ptr[pixel_idx],
                    };

                    g_hsi_t hsi = __rgb_to_hsi(rgb);

                    const bool check_1 = ((ref.h - range.h) <= hsi.h) && (hsi.h <= (ref.h + range.h));
                    const bool check_2 = ((ref.s - range.s) <= hsi.s) && (hsi.s <= (ref.s + range.s));
                    const bool check_3 = ((ref.i - range.i) <= hsi.i) && (hsi.i <= (ref.i + range.i));

                    if (check_1 && check_2 && check_3) {
                        output->r.ptr[pixel_idx] = rgb.r;
                        output->g.ptr[pixel_idx] = rgb.g;
                        output->b.ptr[pixel_idx] = rgb.b;
                    } else {
                        output->r.ptr[pixel_idx] = 0;
                        output->g.ptr[pixel_idx] = 0;
                        output->b.ptr[pixel_idx] = 0;
                    }
                }
            }

            rvalue = true;
        }
    }

    return rvalue;
}

void g_bmp_link(g_bmp_t *self) {
    if (self != NULL) {
        // variables & intrinsic
        __unsafe_reset(self);

        // functions
        self->Create      = Create;
        self->Destroy     = Destroy;
        self->Load        = Load;
        self->Save        = Save;
        self->getWidth    = getWidth;
        self->getHeight   = getHeight;
        self->toGrayscale = toGrayscale;
        self->applyFilter = applyFilter;
        self->selectColor = selectColor;
    }
}

// -----------------------------------------------------------------------------
// End of File
