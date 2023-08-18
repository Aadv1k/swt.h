#include "../thirdparty/munit.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../thirdparty/stb_image.h"


static MunitResult
SWT_SmallImage_hasExpectedCharactersAsStrokes(const MunitParameter params[],
                                 void *user_data) {
    (void)params;
    (void)user_data;

    /*
    int width, height, channels;
    uint8_t *image_data = stbi_load(SWT_TEST_IMG_PATH, &width, &height, &channels, 0);

    SWTImage image = {
        .bytes = image_data,
        .width = width,
        .height = height,
        .channels = channels,
    };

    swt_apply_grayscale(&image);
    swt_apply_threshold(&image, 128);

    float *swt_map = swt_stroke_width_transform(&image);

    // Implement your assertion for the expected number of regions
    // Calculate the number of regions in the swt_map and assert accordingly
    // munit_assert_int(number_of_regions, ==, expected_number_of_regions);

    free(swt_map);

    stbi_image_free(image.bytes);
    */

    return MUNIT_SKIP;
}

MunitTest SWTTests[] = {
    {"/SWT_SmallImage_hasExpectedWidths",
     SWT_SmallImage_hasExpectedCharactersAsStrokes,
     NULL, // No setup needed
     NULL, // No teardown needed
     MUNIT_TEST_OPTION_NONE,
     NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};
