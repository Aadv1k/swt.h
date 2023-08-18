# Stroke Width Transform

Zero-dependency, public domain header-only library that implements [Stroke Width Transform](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/1509.pdf)

See:

- [Quickstart](#quickstart)
- [Examples](#examples)
- [Gallery](#gallery)
- [Tests](#tests)

## Quickstart

Grab a copy of [swt.h](./swt.h) and load it in your project as a stb-style lib

The documentation for functions is also contained within the header file

```c
#define SWT_IMPLEMENTATION
#include "swt.h"
```

## Examples

Here is how you would use this with stb

```c
  /* ... */

    SWTImage image = { image_data, width, height, channels };
    SWTComponents *components = swt_allocate_components(image.width * image.height);
    SWTResults *results = swt_allocate_results(image.width * image.height);

    swt_apply_stroke_width_transform(&image, components, results);
    
    swt_visualize_text_on_image(&image, results);

    swt_free_components(components);
    swt_free_results(results);

  /* ... */
```

this will produce the following output

## Gallery

| Original image                            | Detected text (Highlighted in Gray)            |
|-------------------------------------------|------------------------------------------------|
| <img alt="original image" width="250" src="./thirdparty/test2.jpg" /> | <img alt="detected text in gray" width="250"  src="./thirdparty/test2_output.jpg" /> |


| Original image                            | Detected text (Highlighted in Gray)            |
|-------------------------------------------|------------------------------------------------|
| <img alt="original image" width="250" src="./thirdparty/freedom.jpg" /> | <img alt="detected text in gray" width="250"  src="./thirdparty/freedom_output.jpg" /> |


## Tests

The tests are written using [µnit](https://nemequ.github.io/munit/) find them at [`tests/`](./tests)

```
.\build.bat TEST
.\swt_test.exe
```

