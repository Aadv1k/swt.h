#include "../thirdparty/munit.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../thirdparty/stb_image_write.h"

#define SWT_IMPLEMENTATION
#include "../swt.h"

#define CCA_TEST_IMG_PATH "./thirdparty/test1.jpg"

static MunitResult
CCA_SmallImage_hasExpectedComponents(const MunitParameter params[],
                                     void *user_data) {
  (void)params;
  (void)user_data;

  int width, height, channels;
  uint8_t *image_data =
      stbi_load(CCA_TEST_IMG_PATH, &width, &height, &channels, 0);

  SWTImage image = {
      .bytes = image_data,
      .width = width,
      .height = height,
      .channels = channels,
  };

  SWTComponents *components =
      swt__allocate_components(image.width, image.height);

  swt_apply_grayscale(&image);
  swt_apply_threshold(&image, 128);

  swt_connected_component_analysis(&image, components);

  for (int i = 0; i < components->itemCount; i++) {
    for (int j = 0; j < components->items[i].pointCount; j++) {
      int x = components->items[i].points[j].x;
      int y = components->items[i].points[j].y;

      image.bytes[y * width + x] = 100;
    }
  }

  stbi_write_jpg("output.jpg", width, height, channels, image.bytes, 100);

  munit_logf(1, "image_size = %d", width, height, width * height);

  munit_assert_int(components->itemCount, ==, 17);

  swt__free_components(components);

  stbi_image_free(image.bytes);

  return MUNIT_OK;
}

MunitTest CCATests[] = {{"/CCA_SmallImage_hasExpectedComponents",
                         CCA_SmallImage_hasExpectedComponents,
                         NULL, // No setup needed
                         NULL, // No teardown needed
                         MUNIT_TEST_OPTION_NONE, NULL},
                        {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};
