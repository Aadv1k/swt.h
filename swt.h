/* swt.h -- stb style header-only library for Stroke Width Transform (SWT)

   Note: This implementation isn't perfect; I will continue to improve/optimize
   the logic. Appreciate any input at -- https://github.com/aadv1k/swt/issues

How does it work?
   Stroke width transform is an algorithm that is useful for extracting text
   from natural scenes, making it useful for OCR. Here's a brief overview of the process:
   - Convert the image to grayscale for faster and easier computation.
   - Convert the image into a black and white mask; this will give us the foreground (fg) and background (bg).
   - Apply "Connected Component Analysis," which is a graph-based algorithm that finds all the white pixels in a connected region.
   - Loop through each component and each of its points, cast an imaginary ray, and find the next bg pixel to determine the width of the stroke.
   - Store these widths in an array and return their median. This is the "confidence" with which we can say a component is text.

Usage:
   #define SWT_IMPLEMENTATION
   #include "swt.h"

   Define SWT_ASSERT to avoid using <assert.h>

   SWTImage image = {
       .bytes = image_data,
       .width = width,
       .height = height,
       .channels = comp
   };

   For most cases, you can just call the primary function. For that, you first need to allocate the necessary space:

    SWTData *data = swt_allocate(width * height);

    swt_apply_stroke_width_transform(&image, data->components, data->results);
    swt_visualize_text_on_image(&image, data->results, 4); // 4 is the confidence threshold

   You must then also free the allocated memory.

    swt_free(data);

   Additionally, SWT exposes all the functions used for pre-processing and allocation publicly. Extensive documentation is provided further down.

Functions for primary transformation:

    void swt_apply_stroke_width_transform(SWTImage *image, SWTComponents *components, SWTResults *results);
    SWTResults *swt_allocate_results(int count);
    void swt_free_results(SWTResults *results);
    float swt_compute_stroke_width_for_component(SWTImage *image, SWTComponent *currentComponent);

Functions for CCA:

    SWTComponents *swt_allocate_components(int size);
    void swt_connected_component_analysis(SWTImage *image, SWTComponents *components);
    void swt_free_components(SWTComponents *components);

Processing and filter functions:

    SWTSobelNode swt_compute_sobel_for_point(SWTImage *image, SWTPoint point);
    void swt_apply_grayscale(SWTImage *image);
    void swt_apply_threshold(SWTImage *image, const int threshold);
*/

#ifndef SWT_H_
#define SWT_H_

#include <math.h> // pow, atan2, sqrt, floor, ceil
#include <stdlib.h> // qsort, malloc, calloc, free
#include <string.h> // memcpy

#ifndef SWT_ASSERT
#include <assert.h>
#define SWT_ASSERT(c) assert(c)
#endif


#ifndef SWTDEF
#ifdef SWT_STATIC
#define SWTDEF static
#else
#define SWTDEF extern
#endif
#endif 

typedef unsigned char uint8_t;

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
  SWTComponents *components;
  SWTResults *results;
} SWTData;

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
#define SWT_SWAP(dest, src)                                                    \
  do {                                                                         \
    int temp = dest;                                                           \
    dest = src;                                                                \
    src = temp;                                                                \
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

// These functions manage the memory for the stroke width component 
SWTDEF SWTData* swt_allocate(int size);
SWTDEF void swt_free(SWTData *data);

// The below is the primary function, it encapsulates the logic for calling CCA,
// looping through the results and computing the stroke width likelihood for
// them. Instructs on how to use this are given at the top
SWTDEF void swt_apply_stroke_width_transform(SWTImage *image,
                                             SWTComponents *components,
                                             SWTResults *results);

// This function actually carries out the "computation" part of the SWT, it
// loops through each point in the given components, calculates their gradient
// direction and extracts a stroke width and returns the median of all the
// widths in the component
SWTDEF int
swt_compute_stroke_width_for_component(SWTImage *image,
                                       SWTComponent *currentComponent);

// Does a Connective Component Analysis for the image, it DOES NOT handle
// binarization, is must be handled outside.
// Usage:
//
//    SWTComponents *components =
//        swt__allocate_components(image->width, image->height);
//    swt_connected_component_analysis(image, components);
//    swt__free_components(components);
SWTDEF SWTComponents *swt__allocate_components(int size);
SWTDEF void swt_connected_component_analysis(SWTImage *image,
                                             SWTComponents *components);
SWTDEF void swt__free_components(SWTComponents *components);

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

// pre-processing apply filters destructively, eg they will resize and/or
// modify the image->bytes
SWTDEF void swt_apply_grayscale(SWTImage *image);
SWTDEF void swt_apply_threshold(SWTImage *image, const int threshold);

// Computes a threshold for the image based on the image itself
SWTDEF uint8_t swt_compute_otsu_threshold(SWTImage *image);

SWTDEF void swt_visualize_text_on_image(SWTImage *image, SWTResults *results, const int confidenceThreshold);

#endif // SWT_H_

#ifdef SWT_IMPLEMENTATION

SWTDEF void swt_connected_component_analysis(SWTImage *image,
                                             SWTComponents *components) {
  int width = image->width, height = image->height;
  uint8_t *data = image->bytes;

  /*
  const int directions[8][2] = {{-1, 0},  {1, 0},  {0, -1}, {0, 1},
                                {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
  const int cardinals = 8;

  */

    const int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    const int cardinals = 4;


  int *visited = (int *)calloc(width * height, sizeof(int));
  SWTPoint *queue = (SWTPoint *)malloc(width * height * sizeof(SWTPoint));

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      if (data[i * width + j] == SWT_CLR_WHITE || visited[i * width + j])
        continue;

      SWTComponent currentComponent;
      currentComponent.pointCount = 0;
      currentComponent.points =
          (SWTPoint *)malloc(width * height * sizeof(SWTPoint));

      int qEnd = 0, qBegin = 0;

      queue[qEnd] = (SWTPoint){j, i};
      qEnd++;
      visited[i * width + j] = 1;

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
          visited[yy * width + xx] = 1;

          currentComponent.points[currentComponent.pointCount] =
              (SWTPoint){xx, yy};
          currentComponent.pointCount++;
        }
      }

      if (currentComponent.pointCount > 0) {
        components->items[components->itemCount] = currentComponent;
        components->itemCount++;
      } else {
        free(currentComponent.points);
      }
    }
  }

  free(visited);
  free(queue);
}

SWTDEF void swt_apply_grayscale(SWTImage *image) {
  SWT_ASSERT(image->channels == 3);

  int imageSize = image->width * image->height;

  uint8_t *grayscaleImage = (uint8_t *)malloc(imageSize * sizeof(uint8_t));
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

// TODO: this will break black on white
 SWTDEF void swt_apply_threshold(SWTImage * image, const int threshold) {
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

SWTDEF SWTComponents *swt__allocate_components(int size) {
  SWTComponents *components = (SWTComponents *)malloc(sizeof(SWTComponents));

  if (components != NULL) {
    components->itemCount = 0;
    components->items = (SWTComponent *)malloc(size * sizeof(SWTComponent));

    SWT_IF_NO_MEMORY_EXIT(components->items);
  }

  return components;
}

SWTDEF void swt__free_components(SWTComponents *components) {
  if (components) {
    components->items = NULL;
    components->itemCount = 0;
    free(components);
  }
}

static SWTResults *swt__allocate_results(int count) {
  SWTResults *results = (SWTResults *)malloc(sizeof(SWTResults));

  if (results != NULL) {
    results->items = (SWTResult *)malloc(sizeof(SWTResult) * count);

    if (results->items != NULL) {
      results->itemCount = 0;

      for (int i = 0; i < count; i++) {
        results->items[i].confidence = 0.0f;
        results->items[i].component = NULL;
      }
    } else {
      // Handle memory allocation error for items array
      free(results);
      return NULL;
    }
  }

  return results;
}

static void swt__free_results(SWTResults *results) {
  if (results != NULL) {
    free(results->items);
    results->items = NULL;
    results->itemCount = 0;
    free(results);
  }
}

SWTDEF SWTData* swt_allocate(int size) {
    SWTData* data = (SWTData*)malloc(sizeof(SWTData));
    SWT_IF_NO_MEMORY_EXIT(data);

    data->components = swt__allocate_components(size);
    data->results = swt__allocate_results(size);

    return data;
}

SWTDEF void swt_free(SWTData *data) {
    swt__free_components(data->components);
    swt__free_results(data->results);
}

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


SWTDEF uint8_t swt_compute_otsu_threshold(SWTImage *image) {
    if (image == NULL) {
        return 0;
    }

    int histogram[256] = {0};
    int totalPixels = image->width * image->height;

    // Calculate histogram
    for (int i = 0; i < totalPixels; i++) {
        int pixelValue = image->bytes[i];
        histogram[pixelValue]++;
    }

    float sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += i * histogram[i];
    }

    int sumB = 0;
    int wB = 0;
    int wF = 0;

    float maxVariance = 0;
    uint8_t threshold = 0;

    for (int t = 0; t < 256; t++) {
        wB += histogram[t];
        if (wB == 0) continue;

        wF = totalPixels - wB;
        if (wF == 0) break;

        sumB += t * histogram[t];
        float meanB = (float)sumB / wB;
        float meanF = (float)(sum - sumB) / wF;

        float varianceBetween = wB * wF * (meanB - meanF) * (meanB - meanF);

        if (varianceBetween > maxVariance) {
            maxVariance = varianceBetween;
            threshold = t;
        }
    }

    return threshold;
}


static int swt__qsort_compare(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}

static float swt__median(int *nums, int len) {
  qsort(nums, len, sizeof(int), swt__qsort_compare);

  const float half = len / 2.0;
  if (len % 2 == 0)
    return (nums[(int)half - 1] + nums[(int)half]) / 2.0;

  return nums[(int)half];
}

SWTDEF int swt_compute_stroke_width_for_component(SWTImage *image, SWTComponent *currentComponent) {
    SWT_ASSERT(image->channels == 1 && "swt_compute_stroke_width_for_component expects a BINARY image");
    int *strokes = (int *)malloc(sizeof(int) * currentComponent->pointCount);
    SWT_IF_NO_MEMORY_EXIT(strokes);

    int strokeCount = 0;
    int maxDistance = (int)(image->width * image->height) / 4;

    for (int j = 0; j < currentComponent->pointCount; j++) {
        SWTSobelNode sobelNode = swt_compute_sobel_for_point(image, currentComponent->points[j]);

        int distancePositive = 0;

        int xx = currentComponent->points[j].x;
        int yy = currentComponent->points[j].y;

        float step_x = cos(sobelNode.direction);
        float step_y = sin(sobelNode.direction);

        for (int i = 0; i < maxDistance; i++) {
            int x = xx + (int)(i * step_x);
            int y = yy + (int)(i * step_y);

            if (x < 0 || x >= image->width || y < 0 || y >= image->height) {
                break;
            }
            if (image->bytes[y * image->width + x] == SWT_CLR_BLACK) {
                break;
            }

            distancePositive++;
        }

        strokes[strokeCount] = distancePositive;
        strokeCount++;
    }

    int median = swt__median(strokes, strokeCount);
    free(strokes);

    return median;
}

SWTDEF void swt_apply_stroke_width_transform(SWTImage *image,
                                             SWTComponents *components,
                                             SWTResults *results) {
  swt_apply_grayscale(image);

  /* This makes the logic for visualization needlessly complex since gray and black don't contrast well
    SWTImage binaryImage;
    binaryImage.width = image->width;
    binaryImage.height = image->height;
    binaryImage.channels = image->channels;
    binaryImage.bytes = (uint8_t *)malloc(sizeof(uint8_t) * image->width * image->height * image->channels);
    memcpy(binaryImage.bytes, image->bytes, sizeof(uint8_t) * image->width * image->height * image->channels);
  */

  // threshold is inverted such that WHITE is the foreground
  swt_apply_threshold(image, SWT_THRESHOLD);

  swt_connected_component_analysis(image, components);

  for (int i = 0; i < components->itemCount; i++) {
    results->items[i].component = &components->items[i];
    results->items[i].confidence =
        swt_compute_stroke_width_for_component(image, &components->items[i]);
    //printf("confidence for component#%d is %f\n", i, results->items[i].confidence);
    results->itemCount++;
  }

  // free(binaryImage.bytes);
}


#pragma GCC diagnostic ignored "-Wunused-function"

static int swt__confidence_sum(SWTResults *results) {
    int sum = 0;
    for (int i = 0; i < results->itemCount; i++) {
        sum += results->items[i].confidence;
    }
    return sum;
}


SWTDEF void swt_visualize_text_on_image(SWTImage *image, SWTResults *results, const int confidenceThreshold) {
  if (image == NULL || results == NULL) {
    return;
  }

  for (int i = 1; i < results->itemCount; i++) {
    SWTComponent *component = results->items[i].component;
    int confidence = results->items[i].confidence;

    if (confidenceThreshold < confidence) continue;

    if (component == NULL) {
      continue;
    }

    for (int j = 0; j < component->pointCount; j++) {
      SWTPoint point = component->points[j];

      int index = (point.y * image->width + point.x) * image->channels;
      image->bytes[index] = 128; 
    }
  }
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
