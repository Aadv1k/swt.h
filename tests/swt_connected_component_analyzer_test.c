#include "../thirdparty/munit.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb_image.h"

#define SWT_IMPLEMENTATION
#include "../swt.h"

// NOTE: the components were counted manually since the images are simple

#define CCA_TEST_1_PATH "./thirdparty/test2.jpg"
#define CCA_TEST_1_COUNT 21 

#define CCA_TEST_2_PATH "./thirdparty/test3.jpg"
#define CCA_TEST_2_COUNT 61

static MunitResult
CCA_SmallImage_hasExpectedComponents(const MunitParameter params[],
                                     void *user_data) {
  (void)params;
  (void)user_data;

  int width, height, channels;
  uint8_t *image_data =
      stbi_load(CCA_TEST_1_PATH, &width, &height, &channels, 0);

  SWTImage image = {
      .bytes = image_data,
      .width = width,
      .height = height,
      .channels = channels,
  };

  SWTComponents *components =
      swt_allocate_components(image.width * image.height);

  swt_apply_grayscale(&image);
  swt_apply_threshold(&image, 128);

  swt_connected_component_analysis(&image, components);

  munit_assert_int(components->itemCount, ==, CCA_TEST_1_COUNT);

  swt_free_components(components);

  stbi_image_free(image.bytes);

  return MUNIT_OK;
}

static MunitResult
CCA_MediumImage_hasExpectedComponents(const MunitParameter params[],
                                      void *user_data) {
  (void)params;
  (void)user_data;

  int width, height, channels;
  uint8_t *image_data =
      stbi_load(CCA_TEST_2_PATH, &width, &height, &channels, 0);

  SWTImage image = {
      .bytes = image_data,
      .width = width,
      .height = height,
      .channels = channels,
  };

  SWTComponents *components =
      swt_allocate_components(image.width * image.height);

  swt_apply_grayscale(&image);
  swt_apply_threshold(&image, 128);

  swt_connected_component_analysis(&image, components);

  munit_assert_int(components->itemCount, ==, CCA_TEST_2_COUNT);

  swt_free_components(components);

  stbi_image_free(image.bytes);

  return MUNIT_OK;
}

MunitTest CCATests[] = {{"/CCA_SmallImage_hasExpectedComponents",
                         CCA_SmallImage_hasExpectedComponents,
                         NULL, // No setup needed
                         NULL, // No teardown needed
                         MUNIT_TEST_OPTION_NONE, NULL},
                        {"/CCA_MediumImage_hasExpectedComponents",
                         CCA_MediumImage_hasExpectedComponents,
                         NULL, // No setup needed
                         NULL, // No teardown needed
                         MUNIT_TEST_OPTION_NONE, NULL},
                        {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}

};
