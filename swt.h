/*  swt.h -- stb style header-only library for Stroke Width Transform (SWT)

    Note: This implementation isn't perfect, I will continue to improve/optimize
   the logic. Appreciate any input at -- https://github.com/aadv1k/swt/issues

    Reference:
        https://en.wikipedia.org/wiki/Connected-component_labeling.
        https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/1509.pdf

    Usage:
      #define SWT_IMPLEMENTATION
      #include "swt.h"

      SWTImage image = {
        .bytes = image_data,
        .width = width,
        .height = height,
        .channels = comp
      };

    For most cases you can just call the primary function

        swt_stroke_width_transform(&image);

    Here is an "unwrapped" implementation which might provide you greater
   control

        swt_apply_grayscale(&image);
        swt_apply_threshold(&image, SWT_THRESHOLD);

        SWTComponents *components = swt_allocate_components(image.width,
   image.height); swt_connected_component_analysis(image, components);

        swt_free_components(components);

    For further info on how to use these function, scroll further
 */

#ifndef SWT_H_
#define SWT_H_

#include <assert.h>
#include <math.h> // pow, atan2, sqrt, floor, ceil
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SWTDEF
#define SWTDEF static inline
#endif

typedef struct {
  int x;
  int y;
} SWTPoint;

typedef struct {
  SWTPoint *points;
  int pointCount;
} SWTComponent;

typedef struct {
  SWTComponent *items;
  int itemCount;
} SWTComponents;

typedef struct {
  SWTComponent *component; // holds a reference to a component elsewhere
  float confidence;
} SWTResult;

typedef struct {
  SWTResult *items;
  int itemCount;
} SWTResults;

typedef struct {
  uint8_t *bytes;
  int width;
  int height;
  int channels;
} SWTImage;

typedef struct {
  int magnitude;
  int direction;
  int gradientX;
  int gradientY;
} SWTSobelNode;

#ifndef SWT_CLR_BLACK
#define SWT_CLR_BLACK 0
#endif // SWT_CLR_BLACK

#ifndef SWT_SWAP
#define SWT_SWAP(dest, src) do { \
    int temp = dest; \
    dest = src; \
    src = temp; \
} while (0)
#endif // SWT_SWAP    

#ifndef SWT_CLR_WHITE
#define SWT_CLR_WHITE 255
#endif // SWT_CLR_WHITE

#ifndef SWT_THRESHOLD
#define SWT_THRESHOLD 128
#endif // SWT_THRESHOLD

#ifndef SWT_IF_NO_MEMORY_EXIT
#define SWT_IF_NO_MEMORY_EXIT(ptr)                                             \
  do {                                                                         \
    if ((ptr) == NULL) {                                                       \
      perror("Memory allocation failed");                                      \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)
#endif // SWT_IF_NO_MEMORY_EXIT

    // TODO: add usage info for these
    SWTDEF void
    swt_free_results(SWTResults *results);
SWTDEF SWTResults *swt_allocate_results(int count);

// This function computes the gradient info via sobel operator, for the pixel
// located at (x, y)
// Usage:
//    SWTSobelNode node = swt_compute_sobel_for_point(image, (Point){0, 1});
//      node.gradientX
//      node.gradientY
//      node.magnitude
//      node.direction
SWTDEF SWTSobelNode swt_compute_sobel_for_point(SWTImage *image,
                                                SWTPoint point);

// Does a Connective Component Analysis for the image, it DOES NOT handle
// binarization, is must be handled outside.
// Usage:
//
//    SWTComponents *components =
//        swt_allocate_components(image->width, image->height);
//    swt_connected_component_analysis(image, components);
//    swt_free_components(components);

SWTDEF SWTComponents *swt_allocate_components(int width, int height);
SWTDEF void swt_connected_component_analysis(SWTImage *image,
                                             SWTComponents *components);
SWTDEF void swt_free_components(SWTComponents *components);

// pre-processing apply filters destructively, eg they will resize and/or modify
// the image->bytes
SWTDEF void swt_apply_grayscale(SWTImage *image);
SWTDEF void swt_apply_threshold(SWTImage *image, const int threshold);

#endif // SWT_H_

#ifdef SWT_IMPLEMENTATION

SWTDEF void swt_connected_component_analysis(SWTImage *image,
                                             SWTComponents *components) {
  int width = image->width, height = image->height;
  uint8_t *data = image->bytes;

  const int directions[8][2] = {{-1, 0},  {1, 0},  {0, -1}, {0, 1},
                                {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
  const int cardinals = 8;

  bool *visited = (bool *)calloc(width * height, sizeof(bool));
  SWTPoint *queue = (SWTPoint *)malloc(width * height * sizeof(SWTPoint));

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      if (data[i * width + j] == SWT_CLR_WHITE || visited[i * width + j])
        continue;

      SWTComponent currentComponent;
      currentComponent.pointCount = 0;
      currentComponent.points = (SWTPoint *)malloc(
          width * height * sizeof(SWTPoint)); // Allocate space

      int qEnd = 0, qBegin = 0;

      queue[qEnd] = (SWTPoint){j, i};
      qEnd++;
      visited[i * width + j] = true;

      while (qEnd > qBegin) {
        int x = queue[qBegin].x, y = queue[qBegin].y;
        qBegin++;

        for (int d = 0; d < cardinals; d++) {
          int xx = x + directions[d][0];
          int yy = y + directions[d][1];

          if (xx < 0 || xx >= width || yy < 0 || yy >= height)
            continue;
          if (data[yy * width + xx] == SWT_CLR_BLACK ||
              visited[yy * width + xx])
            continue;

          queue[qEnd] = (SWTPoint){xx, yy};
          qEnd++;
          visited[yy * width + xx] = true;

          currentComponent.points[currentComponent.pointCount] =
              (SWTPoint){xx, yy};
          currentComponent.pointCount++;
        }
      }

      if (currentComponent.pointCount > 0) {
        components->items[components->itemCount] = currentComponent;
        components->itemCount++;
      } else {
        free(currentComponent.points); // Free allocated memory if no points
      }
    }
  }

  free(visited);
  free(queue);
}

SWTDEF void swt_apply_grayscale(SWTImage *image) {
  assert(image->channels == 3);

  int imageSize = image->width * image->height;

  uint8_t *grayscaleImage = (uint8_t *)malloc(imageSize * sizeof(uint8_t));
  SWT_IF_NO_MEMORY_EXIT(grayscaleImage);

  for (int i = 0; i <= imageSize; i++) {
    uint8_t r = image->bytes[i * 3];
    uint8_t g = image->bytes[i * 3 + 1];
    uint8_t b = image->bytes[i * 3 + 2];
    grayscaleImage[i] = (uint8_t)(0.3 * r + 0.59 * g + 0.11 * b);
  }

  memcpy(image->bytes, grayscaleImage, imageSize * sizeof(uint8_t));
  image->channels = 1;
  free(grayscaleImage);
}

SWTDEF void swt_apply_threshold(SWTImage *image, const int threshold) {
  for (int y = 0; y < image->height; y++) {
    for (int x = 0; x < image->width; x++) {
      int index = (y * image->width + x) * image->channels;
      if (image->bytes[index] > threshold) {
        image->bytes[index] = SWT_CLR_WHITE;
      } else {
        image->bytes[index] = SWT_CLR_BLACK;
      }
    }
  }
}

SWTDEF SWTComponents *swt_allocate_components(int width, int height) {
  SWTComponents *components = (SWTComponents *)malloc(sizeof(SWTComponents));
  components->itemCount = 0;
  components->items =
      (SWTComponent *)malloc(width * height * sizeof(SWTComponent));

  return components;
}

SWTDEF void swt_free_components(SWTComponents *components) {
  if (components) {
    for (int i = 0; i < components->itemCount; i++) {
      free(components->items[i].points);
    }
    free(components->items);
    free(components);
  }
}

SWTDEF SWTResults *swt_allocate_results(int count) {
  SWTResults *results = (SWTResults *)malloc(sizeof(SWTResults));

  results->items = (SWTResult *)malloc(sizeof(SWTResult) * count);
  SWT_IF_NO_MEMORY_EXIT(results->items);

  results->itemCount = 0;

  for (int i = 0; i <= count; i++) {
    results->items[i].confidence = 0.0f;
    results->items[i].component = NULL;
  }

  return results;
}

SWTDEF void swt_free_results(SWTResults *results) { free(results->items); }

SWTDEF SWTSobelNode swt_compute_sobel_for_point(SWTImage *image,
                                                SWTPoint point) {
  const int sobelX[][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
  const int sobelY[][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};

  SWTSobelNode node = {0};

  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      int xx = point.x + x;
      int yy = point.y + y;

      if (xx >= 0 && xx < image->width && yy >= 0 && yy < image->height) {
        int currentIndex = yy * image->width + xx;

        node.gradientX += image->bytes[currentIndex] * sobelX[y][x];
        node.gradientY += image->bytes[currentIndex] * sobelY[y][x];
      }
    }
  }

  node.magnitude =
      sqrt(node.gradientX * node.gradientX + node.gradientY * node.gradientY);
  node.direction = atan2(node.gradientY, node.gradientX);

  return node;
}

SWTDEF void swt__bubble_sort(int *nums, int len) {
    for (int i = 0; i < len - 1; i++) {
        for (int j = 0; j < len - i - 1; j++) {
            if (nums[j + 1] < nums[j]) {
                SWT_SWAP(nums[j], nums[j + 1]);
            }
        }
    }
}

SWTDEF float swt__median(int *nums, float len) {
  swt__bubble_sort(nums, len);

  const float half = len / 2;
  if (half == 0.0)
    return nums[(int)half];
  return (nums[(int)floor(half)] + nums[(int)ceil(half)]) / 2;
}

SWTDEF void swt_apply_stroke_width_transform(SWTImage *image) {
  swt_apply_grayscale(image);
  swt_apply_threshold(image, SWT_THRESHOLD);

  SWTComponents *components =
      swt_allocate_components(image->width, image->height);
  swt_connected_component_analysis(image, components);
  SWTResults *results = swt_allocate_results(components->itemCount);

  for (int i = 0; i < components->itemCount; i++) {
    SWTComponent currentComponent = components->items[i];

    int *strokes = malloc(sizeof(int) * image->width * image->height);
    int strokeCount = 0;

    for (int j = 0; j < currentComponent.pointCount; j++) {
      SWTSobelNode sobelNode =
          swt_compute_sobel_for_point(image, currentComponent.points[j]);

      int distancePositive = 0;

      int xx = currentComponent.points[j].x;
      int yy = currentComponent.points[j].y;

      float step_x = cos(sobelNode.direction);
      float step_y = sin(sobelNode.direction);

      for (int i = 0; i < image->width * image->height; i++) {
        int x = xx + (int)(i * step_x);
        int y = yy + (int)(i * step_y);

        if (x < 0 || x >= image->width || y < 0 || y >= image->height) break;
        if (image->bytes[y * image->width + x] == SWT_CLR_BLACK) break;

        distancePositive++;
      }

      strokes[strokeCount] = distancePositive;
      strokeCount++;
    }

    float median = swt__median(strokes, strokeCount);
    results->items[results->itemCount].component = &currentComponent;
    results->items[results->itemCount].confidence = median;

    results->itemCount++;
    free(strokes);
  }

  swt_free_results(results);
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
