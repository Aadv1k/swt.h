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

SWTDEF void swt_apply_stroke_width_transform(SWTImage* image) {
    swt_apply_grayscale(image);
    swt_apply_threshold(image, 128);
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

#endif // SWT_IMPLEMENTATION
