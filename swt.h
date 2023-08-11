#ifndef SWT_H_
#define SWT_H_

#include <stdint.h>
#include <stdio.h>

#define SWTDEF static inline

uint8_t* swt_apply(uint8_t* data, int32_t height, int32_t width, uint8_t channels);

#endif // SWT_H_

#ifdef SWT_IMPLEMENTATION

inline uint8_t* swt_apply(uint8_t* data, int32_t height, int32_t width, uint8_t channels) {
  return data;
}

#endif // SWT_IMPLEMENTATION
