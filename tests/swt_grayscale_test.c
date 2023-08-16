#include "../thirdparty/munit.h"

#define SWT_IMPLEMENTATION
#include "../swt.h"


static uint8_t test_image_data[] = {
    100, 150, 200,
    75,  220, 180,
    30,  190, 240,
    160, 90,  110
};

static uint8_t expected_output_data[5] = {159, 183,147,113, 220};


static SWTImage image;

static void* setup(const MunitParameter params[], void* user_data) {
    (void)params;
    (void)user_data;

    image.bytes = &test_image_data[0];
    image.width = 4;
    image.height = 1;
    image.channels = 3;

    return &image;
}

static void tear_down(void* fixture) {
    (void)fixture;
}

static MunitResult Grayscale_Image_isMonoChannel(const MunitParameter params[],
                                                 void* fixture) {
    (void)params;
    SWTImage* image = (SWTImage*)fixture;

    swt_apply_grayscale(image);
    munit_assert_uint(image->channels, ==, 1);

    return MUNIT_OK;
}

static MunitResult Grayscale_Image_isEqualToExpected(const MunitParameter params[],
                                                     void* fixture) {
    (void)params;
    SWTImage* image = (SWTImage*)fixture;

    swt_apply_grayscale(image);

    for (int i = 0; i <= image->width * image->height; i++) {
        munit_assert_uint8(image->bytes[i], ==, expected_output_data[i]);
    }

    return MUNIT_OK;
}


MunitTest GrayscaleTests[] = {
    {
        "/Grayscale_Image_isMonoChannel",
        Grayscale_Image_isMonoChannel,   
        setup,                           
        tear_down,                       
        MUNIT_TEST_OPTION_NONE,          
        NULL                             
    },
    {
        "/Grayscale_Image_isEqualToExpected",
        Grayscale_Image_isEqualToExpected,   
        setup,                               
        tear_down,                           
        MUNIT_TEST_OPTION_NONE,              
        NULL                                 
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};
