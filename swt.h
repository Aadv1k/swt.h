/*  Stroke Width Transform (SWT), stb style header-only implementation, no-deps.

    Reference:
        https://en.wikipedia.org/wiki/Connected-component_labeling.
        https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/1509.pdf

    Usage:
      #define SWT_IMPLEMENTATION
      #include "swt.h"

      SWTImage image = { image_data, width, height, comp }
      swt_stroke_width_transform(&image);

    Note: This library is quite unoptimized and unsafe. Things may break due to it's experimental nature
 */


#ifndef SWT_H_
#define SWT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>

#ifndef SWTDEF
#define SWTDEF static inline
#endif

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point* points;
    int numOfPoints;
} SWTComponent;

typedef struct {
    SWTComponent* components;
    int numOfComponents;
} SWTComponents;

typedef struct {
    uint8_t* bytes;
    int width;
    int height;
    int channels;
} SWTImage;

#ifndef SOBEL_K_SIZE
#define SOBEL_K_SIZE 3
#endif

#ifndef CLR_BLACK
#define CLR_BLACK 0
#endif // CLR_BLACK

#ifndef CLR_WHITE
#define CLR_WHITE 255
#endif // CLR_WHITE

#ifndef SWT_IF_NO_MEMORY_EXIT
#define SWT_IF_NO_MEMORY_EXIT(ptr) \
    do { \
        if ((ptr) == NULL) { \
            perror("Memory allocation failed"); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)
#endif // SWT_IF_NO_MEMORY_EXIT

SWTDEF void swt_apply_stroke_width_transform(SWTImage* image);
SWTDEF void swt_apply_sobel_operator(SWTImage* image);
SWTDEF void swt_apply_grayscale(SWTImage* image);
SWTDEF void swt_apply_threshold(SWTImage* image, const int threshold);

#endif // SWT_H_

#ifdef SWT_IMPLEMENTATION

static const int sobelX[SOBEL_K_SIZE][SOBEL_K_SIZE] = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}
};

static const int sobelY[SOBEL_K_SIZE][SOBEL_K_SIZE] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};


SWTComponents* swt_allocate_components(int width, int height) {
    SWTComponents* components = (SWTComponents*)malloc(sizeof(SWTComponents));
    if (!components) {
        perror("Memory allocation failed for SWTComponents");
        exit(EXIT_FAILURE);
    }

    components->numOfComponents = width * height;
    components->components = (SWTComponent*)malloc(components->numOfComponents * sizeof(SWTComponent));
    if (!components->components) {
        free(components);
        perror("Memory allocation failed for SWTComponent array");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < components->numOfComponents; i++) {
        components->components[i].numOfPoints = 0;
        components->components[i].points = NULL;
    }

    return components;
}

void swt_free_components(SWTComponents* components) {
    if (components) {
        for (int i = 0; i < components->numOfComponents; i++) {
            free(components->components[i].points);
        }
        free(components->components);
        free(components);
    }
}

SWTDEF void swt_connected_component_analysis(SWTImage* image, SWTComponents* components) {
  static_assert(false, "Not implemented");
}

SWTDEF void swt_apply_sobel_operator(SWTImage* image) {
    assert(image->channels == 1);

    uint8_t* result = (uint8_t*)malloc(image->width * image->height * sizeof(uint8_t));
    SWT_IF_NO_MEMORY_EXIT(result);

    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            int gradientX = 0;
            int gradientY = 0;

            for (int y = 0; y < SOBEL_K_SIZE; y++) {
                for (int x = 0; x < SOBEL_K_SIZE; x++) {
                    int offsetY = i - SOBEL_K_SIZE / 2 + y;
                    int offsetX = j - SOBEL_K_SIZE / 2 + x;

                    if (offsetY >= 0 && offsetY < image->height && offsetX >= 0 && offsetX < image->width) {
                        gradientX += sobelX[y][x] * image->bytes[offsetY * image->width + offsetX];
                        gradientY += sobelY[y][x] * image->bytes[offsetY * image->width + offsetX];
                    }
                }
            }
            int magnitude = (int)sqrt(gradientX * gradientX + gradientY * gradientY);
            result[i * image->width + j] = (uint8_t)(magnitude >= 128 ? 255 : 0);
        }
    }

    memcpy(image->bytes, result, image->height * image->width * sizeof(uint8_t));
    free(result);
}

SWTDEF void swt_apply_grayscale(SWTImage* image) {
    assert(image->channels == 3);

    int imageSize = image->width * image->height;

    uint8_t* grayscaleImage = (uint8_t*)malloc(imageSize * sizeof(uint8_t));
    SWT_IF_NO_MEMORY_EXIT(grayscaleImage);

    for (int i = 0; i < imageSize; i++) {
        uint8_t r = image->bytes[i * 3];
        uint8_t g = image->bytes[i * 3 + 1];
        uint8_t b = image->bytes[i * 3 + 2];
        grayscaleImage[i] = (uint8_t)(0.3 * r + 0.59 * g + 0.11 * b);
    }

    memcpy(image->bytes, grayscaleImage, imageSize * sizeof(uint8_t));
    image->channels = 1;
    free(grayscaleImage);
}

SWTDEF void swt_apply_threshold(SWTImage* image, const int threshold) {
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int index = (y * image->width + x) * image->channels;
            if (image->bytes[index] > threshold) {
                image->bytes[index] = CLR_BLACK;
            } else {
                image->bytes[index] = CLR_WHITE;
            }
        }
    }
}

SWTDEF void swt_apply_stroke_width_transform(SWTImage* image) {
    swt_apply_grayscale(image);
    swt_apply_threshold(image, 128);

    SWTComponents* components = swt_allocate_components(image->width, image->height);
    swt_connected_component_analysis(image, components);

    swt_free_components(components);
}

#endif // SWT_IMPLEMENTATION

/********************************************************************************
  This is free and unencumbered software released into the public domain.
  
  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.
  
  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
  
  For more information, please refer to <https://unlicense.org>
********************************************************************************/

