// -----------------------------------------------------------------------------
// @file g_bmp.h
//
// @date April, 2025
//
// @author Gino Francesco Bogo
// -----------------------------------------------------------------------------

#ifndef G_BMP_H
#define G_BMP_H

#include <stdbool.h> // bool
#include <stdint.h>  // int32_t, uint8_t, uint16_t, uint32_t

// -----------------------------------------------------------------------------

typedef struct __attribute__((packed)) g_bmp_header_t {
    uint16_t type;       // Magic identifier: "BM"
    uint32_t size;       // File size in bytes
    uint16_t reserved_1; // Not used
    uint16_t reserved_2; // Not used
    uint32_t offset;     // Offset to image data in bytes
} g_bmp_header_t;

typedef struct __attribute__((packed)) g_dib_header_t {
    uint32_t size;             // Header size in bytes
    int32_t  width;            // Image width in pixels
    int32_t  height;           // Image height in pixels
    uint16_t planes;           // Number of color planes (must be 1)
    uint16_t bits;             // Bits per pixel (1, 4, 8, 16, 24, or 32)
    uint32_t compression;      // Compression type (0 = none)
    uint32_t image_size;       // Image size in bytes (0 for uncompressed)
    int32_t  x_resolution;     // Pixels per meter (horizontal)
    int32_t  y_resolution;     // Pixels per meter (vertical)
    uint32_t colors;           // Number of colors in palette
    uint32_t important_colors; // Important colors (0 = all)
} g_dib_header_t;

typedef struct g_bmp_channel_t {
    uint8_t *ptr;
    int32_t  width;
    int32_t  height;
} g_bmp_channel_t;

typedef struct g_bmp_t {
    // variables
    g_bmp_channel_t r;
    g_bmp_channel_t g;
    g_bmp_channel_t b;
    g_bmp_header_t  bmp_header;
    g_dib_header_t  dib_header;

    // functions
    bool (*Create)(struct g_bmp_t *self, int32_t width, int32_t height);
    void (*Destroy)(struct g_bmp_t *self);

    bool (*Load)(struct g_bmp_t *self, const char *filename);
    bool (*Save)(struct g_bmp_t *self, const char *filename);

    int32_t (*getWidth)(struct g_bmp_t *self);
    int32_t (*getHeight)(struct g_bmp_t *self);

    bool (*toGrayscale)(struct g_bmp_t *self);

    // intrinsic
    bool _is_safe;
} g_bmp_t;

// -----------------------------------------------------------------------------

extern void g_bmp_link(g_bmp_t *self);

#endif // G_BMP_H

// -----------------------------------------------------------------------------
// End of File
