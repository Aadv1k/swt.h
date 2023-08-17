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

    SWTImage image = { image_data, width, height, channels };
    SWTComponents *components = swt_allocate_components(image.width * image.height);
    SWTResults *results = swt_allocate_results(image.width * image.height);

    swt_apply_stroke_width_transform(&image, components, results);
    swt_visualize_text_on_image(&image, results);

    swt_free_components(components);
    swt_free_results(results);

    if (!stbi_write_jpg(output_filename, image.width, image.height, image.channels, image.bytes, 100)) {
        fprintf(stderr, "ERROR: unable to write image\n");
        stbi_image_free(image.bytes);
        return 1;
    }
    stbi_image_free(image.bytes);
    return 0;
}
