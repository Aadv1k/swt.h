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
typedef struct { Point *points; int numberOfPoints; } SWCharacter;
typedef struct { SWCharacter *characters; int numCharacters; } SWCharacterArray;

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


void swt_free_character_points(SWCharacter* character) {
  free(character->points);
  character->numberOfPoints = 0;
}
 
void swt_extract_character_from_pixel(uint8_t* grayscaleImage, int height, int width, Point loc, SWCharacter* character) {
    bool traversing = true;
    const int CARDINALS = 4;

    character->points = (Point*)malloc(sizeof(Point) * (height * width));
    character->numberOfPoints = 0;

    while (traversing) {
      Point directions[CARDINALS];
      directions[0].x = loc.x;
      directions[0].y = loc.y - 1; // UP

      directions[1].x = loc.x;
      directions[1].y = loc.y + 1; // DOWN

      directions[2].x = loc.x + 1;
      directions[2].y = loc.y;     // RIGHT

      directions[3].x = loc.x - 1;
      directions[3].y = loc.y;     // LEFT

        bool isEdgePixel =
            (grayscaleImage[directions[0].y * width + directions[0].x] == SWT_BLACK &&
             grayscaleImage[directions[3].y * width + directions[3].x] == SWT_BLACK) ||
            (grayscaleImage[directions[1].y * width + directions[1].x] == SWT_BLACK &&
             grayscaleImage[directions[2].y * width + directions[2].x] == SWT_BLACK);

        if (isEdgePixel) {
            character->points[character->numberOfPoints].x = loc.x;
            character->points[character->numberOfPoints].y = loc.y;
            character->numberOfPoints++;
            
            loc = directions[0];

            bool alreadyVisited = false;
            for (int i = 0; i < character->numberOfPoints - 1; i++) {
                if (character->points[i].x == loc.x && character->points[i].y == loc.y) {
                    alreadyVisited = true;
                    break;
                }
            }
            if (alreadyVisited) {
                traversing = false;
            }
        }
    }
}

SWTDEF void swt_apply_stroke_width_transform(uint8_t* data, int* height, int* width, int* channels) {
  int imageSize = *width * *height * *channels;

  uint8_t* grayscaleImage = (uint8_t*)malloc(sizeof(uint8_t) * imageSize);
  memcpy(grayscaleImage, data, sizeof(uint8_t) * imageSize);

  swt_apply_grayscale(grayscaleImage, width, height, channels);
  swt_apply_sobel_operator(grayscaleImage, width, height, channels);

  for (int i = 0; i < *height; i++) {
    for (int j = 0; j < *width; j++) {
      int currentIndex = (i * *width) + j;
      if (grayscaleImage[currentIndex] == SWT_BLACK) continue;

      Point location = { .x = j, .y = i };
      SWCharacter character;
      swt_extract_character_from_pixel(grayscaleImage, *width, *height, location, &character);
      swt_free_character_points(&character);
    }
  }

  memcpy(data, grayscaleImage, sizeof(uint8_t) * imageSize); 
  free(grayscaleImage);
}

#endif // SWT_IMPLEMENTATION
