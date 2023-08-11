#define SWT_IMPLEMENTATION
#include "swt.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb_image_write.h"

int main(void) {
    int width, height, channels;
    uint8_t* image_data = stbi_load("examples/e1.jpg", &width, &height, &channels, 0);
    if (!image_data) {
        fprintf(stderr, "ERROR: unable to load image\n");
        return 1;
    }

    swt_apply(image_data, width, height, channels);

    if (!stbi_write_jpg("output.jpg", width, height, channels, image_data, 100)) {
        fprintf(stderr, "ERROR: unable to write image\n");
        stbi_image_free(image_data);
        return 1;
    }

    stbi_image_free(image_data);
    return 0;
}
