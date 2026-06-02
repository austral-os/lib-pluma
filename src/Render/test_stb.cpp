#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

int main() {
    int w, h, c;
    unsigned char* data = stbi_load("/home/horacio/Downloads/linux-creator.jpg", &w, &h, &c, 4);
    if (data) {
        std::cout << "Loaded linux-creator.jpg: " << w << "x" << h << std::endl;
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load linux-creator.jpg: " << stbi_failure_reason() << std::endl;
    }
    return 0;
}
