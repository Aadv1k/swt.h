# Stroke Width Transform

Zero-dependency, public domain header-only library that implements [Stroke Width Transform](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/1509.pdf)

See:

- [Quickstart](#quickstart)
- [Tests](#tests)
- [Examples](#examples)
- [Credits](#credits)

## Quickstart

Grab a copy of [swt.h](./swt.h) and load it in your project as a stb-style lib

The documentation for functions is also contained within the header file

```c
#define SWT_IMPLEMENTATION
#include "swt.h"
```

## Tests

The tests are written using [µnit](https://nemequ.github.io/munit/) find them at [`tests/`](./tests)

```
.\build.bat TEST
.\swt_test.exe
```


## Examples

Here is how you would use this with stb

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

## Credits

- [µnit](https://nemequ.github.io/munit/)
- [stb](https://github.com/nothings/stb)
- [C Logo](https://commons.wikimedia.org/wiki/File:C_Logo.png)
