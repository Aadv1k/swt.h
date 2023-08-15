# Stroke Width Transform

header only, zero-dep implementation for [Stroke Width Transform](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/1509.pdf) in C
 
## Quickstart

Grab a copy of [swt.h](./swt.h) and load it in your project as a stb-style lib

```c
#define SWT_IMPLEMENTATION
#include "swt.h"
```

## Examples

For a CLI implementations see [`main.c`](./main.c)

```c
/* ... */

int width, height, channels;
uint8_t* image_data = stbi_load("./thirdparty/example.jpg", &width, &height, &channels, 0);

SWTImage image = { image_data, width, height, channels };

swt_apply_stroke_width_transform(&image, 128);

stbi_write_jpg("output.jpg", image.width, image.height, image.channels, image.bytes, 100);

stbi_image_free(image.bytes);

/* ... */
```


