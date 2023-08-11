#ifndef SWT_H_
#define SWT_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>


#ifndef SWTDEF
#define SWTDEF static inline
#endif


SWTDEF void swt_apply(uint8_t* data, int* height, int* width, int* channels);

#endif // SWT_H_

#ifdef SWT_IMPLEMENTATION

#ifndef SOBEL_K_SIZE
#define SOBEL_K_SIZE 3
#endif

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

#ifndef SWT_IF_NO_MEMORY_EXIT
#define SWT_IF_NO_MEMORY_EXIT(ptr) \
    do { \
        if ((ptr) == NULL) { \
            perror("Memory allocation failed"); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)
#endif // SWT_IF_NO_MEMORY_EXIT

void swt_apply_sobel_operator(uint8_t* data, int* height, int* width, int* channels) {
    assert(*channels == 1);

    uint8_t* result = (uint8_t*)malloc((*height) * (*width) * sizeof(uint8_t));
    SWT_IF_NO_MEMORY_EXIT(result);

    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            int gradientX = 0;
            int gradientY = 0;

            for (int y = 0; y < SOBEL_K_SIZE; y++) {
                for (int x = 0; x < SOBEL_K_SIZE; x++) {
                    int offsetY = i - SOBEL_K_SIZE / 2 + y;
                    int offsetX = j - SOBEL_K_SIZE / 2 + x;

                    if (offsetY >= 0 && offsetY < *height && offsetX >= 0 && offsetX < *width) {
                        gradientX += sobelX[y][x] * data[offsetY * (*width) + offsetX];
                        gradientY += sobelY[y][x] * data[offsetY * (*width) + offsetX];
                    }
                }
            }
            int magnitude = (int)sqrt(gradientX * gradientX + gradientY * gradientY);
            result[i * (*width) + j] = (uint8_t)(magnitude >= 128 ? 255 : 0);
        }
    }

    memcpy(data, result, (*height) * (*width) * sizeof(uint8_t));
    free(result);
}

SWTDEF void swt_apply(uint8_t* data, int* height, int* width, int* channels) {
  assert(*channels == 3 && "swt_apply expects a RGB image");

  int imageLength =  (*width) * (*height);
  uint8_t* grayscaleImage = (uint8_t*)malloc(sizeof(uint8_t) * imageLength);
  SWT_IF_NO_MEMORY_EXIT(grayscaleImage);

  for (int32_t i = 0; i < imageLength; i++) {
    uint8_t r = data[i * 3];
    uint8_t g = data[i * 3 + 1];
    uint8_t b = data[i * 3 + 2];
    grayscaleImage[i] = (uint8_t)(0.3 * r + 0.59 * g + 0.11 * b);
  }

  *channels = 1;

  memcpy(data, grayscaleImage, sizeof(uint8_t) * imageLength);
  free(grayscaleImage);

  swt_apply_sobel_operator(data, width, height, channels);
}

#endif // SWT_IMPLEMENTATION
