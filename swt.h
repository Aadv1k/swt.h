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

    For most cases you can just call the primary function, for that you first
    need to allocate the necessary space

        SWTComponents* connectedComponents = swt_allocate_components(image.width * image.height);
        SWTResults* swtResults = swt_allocate_results(image.width * image.height);

        swt_stroke_width_transform(&image);

    You must then also free the allocated memory. This order is important since
    results hold pointers to the connected components

        swt_free_results(swtResults);
        swt_free_components(connectedComponents);

   Additionally, SWT exposes all the functions used for pre-processing,
   allocation publically. Extensive documentatiomn is provided further down

   Functions for primary transformation
   
        void swt_apply_stroke_width_transform(SWTImage *image, SWTComponents *components, SWTResults *results);
        SWTResults *swt_allocate_results(int count);
        void swt_free_results(SWTResults *results);
        float swt_compute_stroke_width_for_component(SWTImage *image, SWTComponent *currentComponent);

   Functions for CCA

        SWTComponents *swt_allocate_components(int size);
        void swt_connected_component_analysis(SWTImage *image, SWTComponents *components);
        void swt_free_components(SWTComponents *components);

   Processing and filter functions

        SWTSobelNode swt_compute_sobel_for_point(SWTImage *image, SWTPoint point);
        void swt_apply_grayscale(SWTImage *image);
        void swt_apply_threshold(SWTImage *image, const int threshold);
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

// The below is the primary function, it encapsulates the logic for calling CCA,
// looping through the results and computing the stroke width likelihood for
// them. Instructs on how to use this are given at the top
SWTDEF void swt_free_results(SWTResults *results);
SWTDEF void swt_apply_stroke_width_transform(SWTImage *image,
                                             SWTComponents *components,
                                             SWTResults *results);
SWTDEF SWTResults *swt_allocate_results(int count);

// This function actually carries out the "computation" part of the SWT, it
// loops through each point in the given components, calculates their gradient
// direction and extracts a stroke width and returns the median of all the
// widths in the component
SWTDEF float
swt_compute_stroke_width_for_component(SWTImage *image,
                                       SWTComponent *currentComponent);

// Does a Connective Component Analysis for the image, it DOES NOT handle
// binarization, is must be handled outside.
// Usage:
//
//    SWTComponents *components =
//        swt_allocate_components(image->width, image->height);
//    swt_connected_component_analysis(image, components);
//    swt_free_components(components);

SWTDEF SWTComponents *swt_allocate_components(int size);
SWTDEF void swt_connected_component_analysis(SWTImage * image,
                                             SWTComponents * components);
SWTDEF void swt_free_components(SWTComponents * components);

// This function computes the gradient info via sobel operator, for the pixel
// located at (x, y)
// Usage:
//    SWTSobelNode node = swt_compute_sobel_for_point(image, (Point){0, 1});
//      node.gradientX
//      node.gradientY
//      node.magnitude
//      node.direction
SWTDEF SWTSobelNode swt_compute_sobel_for_point(SWTImage * image,
                                                  SWTPoint point);

// pre-processing apply filters destructively, eg they will resize and/or
// modify the image->bytes
SWTDEF void swt_apply_grayscale(SWTImage * image);
SWTDEF void swt_apply_threshold(SWTImage * image, const int threshold);

#endif // SWT_H_

#ifdef SWT_IMPLEMENTATION

 SWTDEF void swt_connected_component_analysis(SWTImage * image,
                                              SWTComponents * components) {
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
       currentComponent.points =
           (SWTPoint *)malloc(width * height * sizeof(SWTPoint));

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
         free(currentComponent.points);
       }
     }
   }

   free(visited);
   free(queue);
 }

 SWTDEF void swt_apply_grayscale(SWTImage * image) {
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

 SWTDEF void swt_apply_threshold(SWTImage * image, const int threshold) {
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

SWTComponents *swt_allocate_components(int size) {
    SWTComponents *components = (SWTComponents *)malloc(sizeof(SWTComponents));
    
    if (components != NULL) {
        components->itemCount = 0;
        components->items = (SWTComponent *)malloc(size * sizeof(SWTComponent));

        SWT_IF_NO_MEMORY_EXIT(components->items);
    }

    return components;
}

void swt_free_components(SWTComponents *components) {
    if (components) {
        components->items = NULL; // Set to NULL to avoid accidental double-free
        components->itemCount = 0; // Reset itemCount for clarity
        free(components); // Free the entire structure
    }
}

SWTResults *swt_allocate_results(int count) {
    SWTResults *results = (SWTResults *)malloc(sizeof(SWTResults));

    if (results != NULL) {
        results->items = (SWTResult *)malloc(sizeof(SWTResult) * count);
        
        if (results->items != NULL) {
            results->itemCount = count;

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


SWTDEF void swt_free_results(SWTResults *results) {
    if (results != NULL) {
        free(results->items);
        results->items = NULL;
        results->itemCount = 0; 
        free(results); 
    }
}

SWTDEF SWTSobelNode swt_compute_sobel_for_point(SWTImage * image,
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

float swt_compute_stroke_width_for_component(SWTImage *image, SWTComponent *currentComponent) {
    int *strokes = (int *)malloc(sizeof(int) * currentComponent->pointCount);
    SWT_IF_NO_MEMORY_EXIT(strokes);

    int strokeCount = 0;

    for (int j = 0; j < currentComponent->pointCount; j++) {
        SWTSobelNode sobelNode = swt_compute_sobel_for_point(image, currentComponent->points[j]);

        int distancePositive = 0;

        int xx = currentComponent->points[j].x;
        int yy = currentComponent->points[j].y;

        float step_x = cos(sobelNode.direction);
        float step_y = sin(sobelNode.direction);

        for (int i = 0; i < image->width * image->height; i++) {
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

    float median = swt__median(strokes, strokeCount);
    free(strokes);

    return median;
}

 SWTDEF void swt_apply_stroke_width_transform(
     SWTImage * image, SWTComponents * components, SWTResults * results) {
   swt_apply_grayscale(image);
   swt_apply_threshold(image, SWT_THRESHOLD);

   swt_connected_component_analysis(image, components);

   for (int i = 0; i < components->itemCount; i++) {

     results->items[results->itemCount].component = &components->items[i];
     results->items[results->itemCount].confidence =
         swt_compute_stroke_width_for_component(image, &components->items[i]);

     results->itemCount++;
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
