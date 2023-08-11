#ifndef SWT_H_
#define SWT_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SWTDEF static inline

void swt_apply(uint8_t* data, int* height, int* width, int* channels);

#endif // SWT_H_

#ifdef SWT_IMPLEMENTATION

inline void swt_apply(uint8_t* data, int* height, int* width, int* channels) {
  assert(*channels == 3 && "swt_apply expects a RGB image");

  int imageLength =  (*width) * (*height);
  uint8_t* grayscaleImage = (uint8_t*)malloc(sizeof(uint8_t) * imageLength);

  for (int32_t i = 0; i < imageLength; i++) {
    uint8_t r = data[i * 3];
    uint8_t g = data[i * 3 + 1];
    uint8_t b = data[i * 3 + 2];
    grayscaleImage[i] = (uint8_t)(0.3 * r + 0.59 * g + 0.11 * b);
  }

  *channels = 1;

  memcpy(data, grayscaleImage, sizeof(uint8_t) * imageLength);
  free(grayscaleImage);
}

#endif // SWT_IMPLEMENTATION
