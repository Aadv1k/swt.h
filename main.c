#define SWT_IMPLEMENTATION
#include "swt.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb_image_write.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }

    const char* input_filename = argv[1];
    const char* output_filename = argv[2];

    int width, height, channels;
    uint8_t* image_data = stbi_load(input_filename, &width, &height, &channels, 0);
    if (!image_data) {
        fprintf(stderr, "ERROR: unable to load image\n");
        return 1;
    }

    swt_apply(image_data, &width, &height, &channels);

    if (!stbi_write_jpg(output_filename, width, height, channels, image_data, 100)) {
        fprintf(stderr, "ERROR: unable to write image\n");
        stbi_image_free(image_data);
        return 1;
    }

    stbi_image_free(image_data);
    return 0;
}
