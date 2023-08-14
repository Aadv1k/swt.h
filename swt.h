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


static const int CARDINALS = 1;
static const int directions[][2] = {
    {-1, 0},
    {1, 0},
    {0, 1},
    {0, -1}
};

bool is_point_visited(Point* points, int visitedCount, Point target) {
    for (int i = 0; i < visitedCount; i++) {
        if (points[i].x == target.x && points[i].y == target.y) {
            return true;
        }
    }
    return false;
}

void swt_connected_component_analysis(uint8_t* data, uint8_t* connected, int* width, int* height) {
    int label = 1;

    Point* visitedPoints = (Point*)malloc(sizeof(Point) * (*width) * (*height));
    int visitedCount = 0;

    Point* visitedPoints2 = (Point*)malloc(sizeof(Point) * (*width) * (*height));
    int visitedCount2 = 0;

    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            int index = i * (*width) + j;
            Point currentPoint = {j, i};
            
            if (data[index] == CLR_WHITE && !is_point_visited(visitedPoints, visitedCount, currentPoint)) {
                visitedPoints[visitedCount++] = currentPoint; // Add the current point to visitedPoints

                for (int d = 0; d < CARDINALS; d++) {
                    int xx = j + directions[d][0];
                    int yy = i + directions[d][1];
                    Point neighborPoint = {xx, yy};

                    if (xx >= 0 && xx < *width && yy >= 0 && yy < *height &&
                        data[yy * (*width) + xx] == CLR_WHITE &&
                        !is_point_visited(visitedPoints, visitedCount, neighborPoint)) {

                        visitedPoints[visitedCount++] = neighborPoint;
                    }
                }

                while (visitedCount > 0) {
                    visitedCount--; // Decrement the visitedCount

                    for (int d = 0; d < CARDINALS; d++) {
                        int xx = visitedPoints[visitedCount].x + directions[d][0];
                        int yy = visitedPoints[visitedCount].y + directions[d][1];
                        Point neighborPoint = {xx, yy};

                        if (xx >= 0 && xx < *width && yy >= 0 && yy < *height &&
                            data[yy * (*width) + xx] == CLR_WHITE &&
                            !is_point_visited(visitedPoints, visitedCount, neighborPoint)) {
                            
                            visitedPoints2[visitedCount2++] = neighborPoint;
                        }

                        connected[yy * (*width) + xx] = label; // Visualize connected component
                    }

                    // Remove the current element from visitedPoints (by not copying it to visitedPoints2)
                    // Copy visitedPoints2 into visitedPoints
                    for (int i = 0; i < visitedCount2; i++) {
                        visitedPoints[i] = visitedPoints2[i];
                    }
                    visitedCount = visitedCount2;
                    visitedCount2 = 0;
                }
                
                label++;
            }
        }
    }

    printf("%d\n", label);

    free(visitedPoints);
    free(visitedPoints2);
}


void swt_apply_threshold(uint8_t* data, int* height, int* width, int* channels, const int threshold) {
    for (int y = 0; y < *height; y++) {
        for (int x = 0; x < *width; x++) {
            int index = (y * (*width) + x) * (*channels);
            if (data[index] > threshold) {
                data[index] = CLR_BLACK;
            } else {
                data[index] = CLR_WHITE;
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

  uint8_t* connectedComponentBuffer = (uint8_t*)malloc(sizeof(uint8_t) * imageSize);
  memset(connectedComponentBuffer, 0, imageSize);
  swt_connected_component_analysis(grayscaleImage, connectedComponentBuffer, width, height);

  memcpy(data, connectedComponentBuffer, sizeof(uint8_t) * imageSize); 
  free(grayscaleImage);
  free(connectedComponentBuffer);
}

#endif // SWT_IMPLEMENTATION
