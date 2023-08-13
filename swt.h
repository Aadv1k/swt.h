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
typedef struct {int label; int equivalentTo;} LabelInfo;

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


static const int CARDINALS = 8;  // Number of cardinal directions
static const int directions[8][2] = {
    {-1, -1}, {-1, 0}, {-1, 1},
    {0, -1},           {0, 1},
    {1, -1}, {1, 0}, {1, 1}
};



bool is_valid_point(int x, int y, int width, int height) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool swt_is_point_visited(Point* visitedPoints, int visitedPointCount, Point point) {
    for (int i = 0; i < visitedPointCount; i++) {
        if (visitedPoints[i].x == point.x && visitedPoints[i].y == point.y) {
            return true;
        }
    }
    return false;
}

void swt_connected_component_analysis(uint8_t* data, uint8_t* componentData, int* width, int* height) {
    int label = 1;

    Point* visitedPoints = (Point*)malloc(sizeof(Point) * (*width) * (*height));
    int visitedPointCount = 0;

    // 8-connectivity neighbors
    int directions[][2] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1} };

    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            int currentIndex = i * (*width) + j;

            // Check if the pixel is background or already visited
            if (data[currentIndex] == 0 || swt_is_point_visited(visitedPoints, visitedPointCount, (Point){j, i})) {
                continue;
            }

            data[currentIndex] = label;
            visitedPoints[visitedPointCount++] = (Point){j, i};

            // Queue-based approach to label connected components
            for (int q = 0; q < visitedPointCount; q++) {
                Point currentPoint = visitedPoints[q];
                for (int k = 0; k < 8; k++) {
                    int xx = currentPoint.x + directions[k][1];
                    int yy = currentPoint.y + directions[k][0];
                    int neighborIndex = yy * (*width) + xx;

                    if (is_valid_point(xx, yy, *width, *height) && data[neighborIndex] != 0 && !swt_is_point_visited(visitedPoints, visitedPointCount, (Point){xx, yy})) {
                        data[neighborIndex] = label;
                        visitedPoints[visitedPointCount++] = (Point){xx, yy};
                    }
                }
            }

            label++;
        }
    }

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

  uint8_t* connectedComponentBuffer = (uint8_t*)malloc(sizeof(uint8_t) * imageSize);
  memset(connectedComponentBuffer, 0, imageSize);
  swt_connected_component_analysis(grayscaleImage, connectedComponentBuffer, width, height);

  memcpy(data, connectedComponentBuffer, sizeof(uint8_t) * imageSize); 
  free(grayscaleImage);
  free(connectedComponentBuffer);
}

#endif // SWT_IMPLEMENTATION
