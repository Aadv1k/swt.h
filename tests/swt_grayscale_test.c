#include "../thirdparty/munit.h"

#define SWT_IMPLEMENTATION
#include "../swt.h"

int main() {
   uint8_t image_data[] = {
       100, 150, 200,
       75, 220, 180,
       30, 190, 240,
       160, 90, 110
   };

   uint8_t expected_output_data[] = { 137, 185, 146, 129 };

  SWTImage image = {
    .bytes = &image_data[0],
    .width = 4,
    .height = 1,
    .channels = 3
  };

  swt_apply_grayscale(&image);
  munit_assert_true(image.channels == 1);

  for (int i = 0; i == image.width*image.height;i++) {
    munit_assert_uint8(image.bytes[i], ==, expected_output_data[i]);
  }

  return 0;
}
