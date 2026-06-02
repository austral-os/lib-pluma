#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cairo/cairo.h>
#include <iostream>

int main() {
    int img_w, img_h, channels;
    unsigned char* data = stbi_load("/home/horacio/Downloads/linux-creator.jpg", &img_w, &img_h, &channels, 4);
    if (data) {
        std::cout << "Loaded linux-creator.jpg: " << img_w << "x" << img_h << std::endl;
        int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, img_w);
        std::cout << "Stride: " << stride << ", expected: " << img_w * 4 << std::endl;
        cairo_surface_t* image = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, img_w, img_h, stride);
        if (cairo_surface_status(image) == CAIRO_STATUS_SUCCESS) {
            std::cout << "Cairo surface created successfully!" << std::endl;
        } else {
            std::cout << "Failed to create Cairo surface: " << cairo_status_to_string(cairo_surface_status(image)) << std::endl;
        }
        if (image) cairo_surface_destroy(image);
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load linux-creator.jpg: " << stbi_failure_reason() << std::endl;
    }
    return 0;
}
