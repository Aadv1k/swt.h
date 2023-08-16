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
      swt_stroke_width_transform(&image, 128); // applies threshold of 128 

    Additionally, you can use any one of the several functions used internally for pre-processing:

      void swt_connected_component_analysis(SWTImage *image, SWTComponents *components);
      void swt_apply_sobel_operator(SWTImage *image);
      void swt_apply_grayscale(SWTImage *image);
      void swt_apply_threshold(SWTImage *image, const int threshold);
 */

#ifndef SWT_H_
#define SWT_H_

#include <assert.h>
#include <math.h>
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
  uint8_t *bytes;
  int width;
  int height;
  int channels;
} SWTImage;

#ifndef SWT_CLR_BLACK
#define SWT_CLR_BLACK 0
#endif // SWT_CLR_BLACK

#ifndef SWT_CLR_WHITE
#define SWT_CLR_WHITE 255
#endif // SWT_CLR_WHITE

#ifndef SWT_IF_NO_MEMORY_EXIT
#define SWT_IF_NO_MEMORY_EXIT(ptr)                                             \
  do {                                                                         \
    if ((ptr) == NULL) {                                                       \
      perror("Memory allocation failed");                                      \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)
#endif // SWT_IF_NO_MEMORY_EXIT

SWTDEF void swt_apply_stroke_width_transform(SWTImage *image, const uint8_t threshold);
SWTDEF void swt_connected_component_analysis(SWTImage *image,
                                             SWTComponents *components);
SWTDEF void swt_apply_sobel_operator(SWTImage *image);
SWTDEF void swt_apply_grayscale(SWTImage *image);
SWTDEF void swt_apply_threshold(SWTImage *image, const int threshold);

#endif // SWT_H_

#ifdef SWT_IMPLEMENTATION

SWTDEF void swt_connected_component_analysis(SWTImage *image,
                                             SWTComponents *components) {
  int width = image->width, height = image->height;
  uint8_t *data = image->bytes;

  const int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
  const int cardinals = 4;

  bool *visited = (bool *)calloc(width * height, sizeof(bool));
  SWTPoint *queue = (SWTPoint *)malloc(width * height * sizeof(SWTPoint));
  SWTPoint *queue2 = (SWTPoint *)malloc(width * height * sizeof(SWTPoint));

  int qEnd = 0, qBegin = 0, q2End = 0;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      if (data[i * width + j] == SWT_CLR_BLACK || visited[i * width + j])
        continue;

      SWTComponent currentComponent;
      currentComponent.pointCount = 0;
      currentComponent.points =
          (SWTPoint *)malloc(width * height * sizeof(SWTPoint));

      queue[qEnd] = (SWTPoint){j, i};
      qEnd++;

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

          queue2[q2End] = (SWTPoint){xx, yy};
          q2End++;
          visited[yy * width + xx] = true;

          currentComponent.points[currentComponent.pointCount] =
              (SWTPoint){xx, yy};
          currentComponent.pointCount++;
        }
      }

      components->items[components->itemCount] = currentComponent;
      components->itemCount++;
    }
  }

  free(visited);
  free(queue);
  free(queue2);
}

SWTDEF void swt_apply_sobel_operator(SWTImage *image) {
  assert(image->channels == 1);

  const int sobel_k_size = 3;

  const int sobelX[][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
  const int sobelY[][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};

  uint8_t *result =
      (uint8_t *)malloc(image->width * image->height * sizeof(uint8_t));
  SWT_IF_NO_MEMORY_EXIT(result);

  for (int i = 0; i < image->height; i++) {
    for (int j = 0; j < image->width; j++) {
      int gradientX = 0;
      int gradientY = 0;

      for (int y = 0; y < sobel_k_size; y++) {
        for (int x = 0; x < sobel_k_size; x++) {
          int offsetY = i - sobel_k_size / 2 + y;
          int offsetX = j - sobel_k_size / 2 + x;

          if (offsetY >= 0 && offsetY < image->height && offsetX >= 0 &&
              offsetX < image->width) {
            gradientX +=
                sobelX[y][x] * image->bytes[offsetY * image->width + offsetX];
            gradientY +=
                sobelY[y][x] * image->bytes[offsetY * image->width + offsetX];
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
        image->bytes[index] = SWT_CLR_BLACK;
      } else {
        image->bytes[index] = SWT_CLR_WHITE;
      }
    }
  }
}

static SWTComponents *swt__allocate_components(int width, int height) {
  SWTComponents *components = (SWTComponents *)malloc(sizeof(SWTComponents));
  components->itemCount = 0;
  components->items =
      (SWTComponent *)malloc(width * height * sizeof(SWTComponent));

  return components;
}

static void swt__free_components(SWTComponents *components) {
  if (components) {
    for (int i = 0; i < components->itemCount; i++) {
      free(components->items[i].points);
    }
    free(components->items);
    free(components);
  }
}

SWTDEF void swt_apply_stroke_width_transform(SWTImage *image, const uint8_t threshold) {
  swt_apply_grayscale(image);
  swt_apply_threshold(image, threshold);

  SWTComponents *components =
      swt__allocate_components(image->width, image->height);
  swt_connected_component_analysis(image, components);

  printf("DEBUG: itemCount: %d  Size: %d\n", components->itemCount,
         image->width * image->height);
  printf("DEBUG: %d\n", components->items[0].pointCount);
  printf("DEBUG: %d\n", components->items[1].pointCount);
  printf("DEBUG: %d\n", components->items[2].pointCount);
  printf("DEBUG: %d\n", components->items[3].pointCount);
  printf("DEBUG: %d\n", components->items[4].pointCount);

  swt__free_components(components);
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
