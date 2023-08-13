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

typedef struct {int x; int y;} Point;

#ifndef SOBEL_K_SIZE
#define SOBEL_K_SIZE 3
#endif

#ifndef SWT_BLACK
#define SWT_BLACK 0
#endif // SWT_BLACK

#ifndef SWT_WHITE
#define SWT_WHITE 0
#endif // SWT_WHITE

#ifndef SWT_IF_NO_MEMORY_EXIT
#define SWT_IF_NO_MEMORY_EXIT(ptr) \
    do { \
        if ((ptr) == NULL) { \
            perror("Memory allocation failed"); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)
#endif // SWT_IF_NO_MEMORY_EXIT

SWTDEF void swt_apply_stroke_width_transform(uint8_t* data, int* height, int* width, int* channels);
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


void swt_apply_grayscale(uint8_t* data, int* height, int* width, int* channels) {
    assert(*channels == 3);

    int imageSize = (*width) * (*height);

    uint8_t* grayscaleImage = (uint8_t*)malloc(imageSize * sizeof(uint8_t));
    SWT_IF_NO_MEMORY_EXIT(grayscaleImage);

    for (int i = 0; i < imageSize; i++) {
        uint8_t r = data[i * 3];
        uint8_t g = data[i * 3 + 1];
        uint8_t b = data[i * 3 + 2];
        grayscaleImage[i] = (uint8_t)(0.3 * r + 0.59 * g + 0.11 * b);
    }

    memcpy(data, grayscaleImage, imageSize * sizeof(uint8_t));
    *channels = 1;
    free(grayscaleImage);
}


#define CARDINALS 4

static int directions[4][2] = {
        {0, -1},
  {-1, 0},      {1, 0},
        {0, 1},
};



// OPERATING ON A THRESHOLD BINARY IMAGE
void swt_connected_component_analysis(uint8_t* data, int* height, int* width, int* channels) {
    assert(*channels == 1);
    
    // Allocate memory for an array to store visited points
    Point* visitedPoints = (Point*)malloc(sizeof(Point) * (*width) * (*height));
    if (visitedPoints == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    int numOfVisited = 0;
    int currentLabel = 1;

    // Iterate through each pixel in the image
    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            int currentIndex = (i * (*width)) + j;
            if (data[currentIndex] > 0) {
                // Store the coordinates of the visited pixel
                visitedPoints[numOfVisited].y = i;
                visitedPoints[numOfVisited].x = j;
                numOfVisited++;

                // Assign a label to the connected component
                data[currentIndex] = currentLabel;
                currentLabel++;
            }
        }
    }

    // Free the memory allocated for visitedPoints
    free(visitedPoints);
}

void swt_apply_threshold(uint8_t* data, int* height, int* width, int* channels, const int threshold) {
    for (int y = 0; y < *height; y++) {
        for (int x = 0; x < *width; x++) {
            int index = (y * (*width) + x) * (*channels);
            if (data[index] > threshold) {
                data[index] = 255;
            } else {
                data[index] = 0;
            }
        }
    }
}

SWTDEF void swt_apply_stroke_width_transform(uint8_t* data, int* height, int* width, int* channels) {
  int imageSize = *width * *height * *channels;

  uint8_t* grayscaleImage = (uint8_t*)malloc(sizeof(uint8_t) * imageSize);
  memcpy(grayscaleImage, data, sizeof(uint8_t) * imageSize);

  swt_apply_grayscale(grayscaleImage, width, height, channels);
  swt_apply_threshold(grayscaleImage, width, height, channels, 128);
  //swt_apply_sobel_operator(grayscaleImage, width, height, channels);

  swt_connected_component_analysis(grayscaleImage, width, height, channels);

  memcpy(data, grayscaleImage, sizeof(uint8_t) * imageSize); 
  free(grayscaleImage);
}

#endif // SWT_IMPLEMENTATION
