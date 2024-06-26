#include "image.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>

#include <tbb/tbb.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINY_EXRIMPLEMENTATION
#include "tinyexr.h"

#include "log.h"

namespace stage {
namespace backstage {

Image::Image(std::string filename, bool is_hdr) : m_is_hdr(is_hdr) {
    uint8_t* image = nullptr;
    size_t image_size = 0;

    std::filesystem::path filepath(filename);
    if (filepath.extension().string() == ".exr") {
        const char* err = nullptr;
        float* image_float = nullptr;
        int ret = LoadEXR(&image_float, &m_width, &m_height, filepath.string().c_str(), &err);
        m_channels = 4;

        if (ret != TINYEXR_SUCCESS) {
            ERR("Unable to load image '" + filepath.string() + "'");
            if (err) {
                ERR(err);
                FreeEXRErrorMessage(err);
            }
            return;
        }

        // Flip y axis
        tbb::parallel_for(tbb::blocked_range<uint32_t>(0, m_height/2), [&](const auto& r) {
        for (uint32_t row = r.begin(); row != r.end(); row++) {
            tbb::parallel_for(tbb::blocked_range<uint32_t>(0, m_width * m_channels), [&](const auto& rr) {
            for (uint32_t el = rr.begin(); el != rr.end(); el++) {
                uint32_t id_a = row * m_width * m_channels + el;
                uint32_t id_b = (m_height - row - 1) * m_width * m_channels + el;
                std::swap(image_float[id_a], image_float[id_b]);
            }
            });
        }
        });

        m_is_hdr = true;
        image = (uint8_t*)image_float;

        image_size = sizeof(float) * m_width * m_height * m_channels;
    } else {
        stbi_set_flip_vertically_on_load(true);  
        if (is_hdr)
            image = (uint8_t*)stbi_loadf(filepath.string().c_str(), &m_width, &m_height, &m_channels, 4);
        else
            image = stbi_load(filepath.string().c_str(), &m_width, &m_height, &m_channels, 4);
        m_channels = 4;

        if (image == nullptr) {
            ERR("Unable to load image '" + filepath.string() + "'");
            return;
        }

        image_size = (is_hdr ? sizeof(float) : sizeof(uint8_t)) * m_width * m_height * m_channels;
    }
    
    if (image) {
        m_image = (uint8_t*)std::malloc(image_size);
        std::memcpy(m_image, image, image_size);
        std::free(image);
    }
}

Image::Image(uint8_t* blob, size_t size, bool is_hdr) {
    m_is_hdr = is_hdr;
    uint8_t* image = nullptr;
    size_t image_size = 0;

    // Try stbi first, assuming the image is not EXR
    stbi_set_flip_vertically_on_load(true);  
    if (is_hdr)
        image = (uint8_t*)stbi_loadf_from_memory(blob, size, &m_width, &m_height, &m_channels, 4);
    else
        image = stbi_load_from_memory(blob, size, &m_width, &m_height, &m_channels, 4);
    m_channels = 4;
    image_size = (is_hdr ? sizeof(float) : sizeof(uint8_t)) * m_width * m_height * m_channels;

    // If the blob failed to load with stbi, try tinyexr
    if (image == nullptr) {
        const char* err = nullptr;
        float* image_float;
        int ret = LoadEXRFromMemory(&image_float, &m_width, &m_height, blob, size, &err);
        if (ret != TINYEXR_SUCCESS) {
            ERR("Unable to load image blob");
            if (err) {
                ERR(err);
                FreeEXRErrorMessage(err);
            }
            return;
        }

        // Flip y axis
        tbb::parallel_for(tbb::blocked_range<uint32_t>(0, m_height/2), [&](const auto& r) {
        for (uint32_t row = r.begin(); row != r.end(); row++) {
            tbb::parallel_for(tbb::blocked_range<uint32_t>(0, m_width * m_channels), [&](const auto& rr) {
            for (uint32_t el = rr.begin(); el != rr.end(); el++) {
                uint32_t id_a = row * m_width * m_channels + el;
                uint32_t id_b = (m_height - row - 1) * m_width * m_channels + el;
                std::swap(image_float[id_a], image_float[id_b]);
            }
            });
        }
        });

        m_is_hdr = true;
        image = (uint8_t*)image_float;
        image_size = sizeof(float) * m_width * m_height * m_channels;
    }


    if (image == nullptr) {
        ERR("Unable to load image blob");
        return;
    }

    m_image = (uint8_t*)std::malloc(image_size);
    std::memcpy(m_image, image, image_size);
    std::free(image);
}

Image::Image(stage_vec3f color) {
    m_image = (uint8_t*)std::malloc(sizeof(uint8_t) * 4);
    m_image[0] = color.x * 255;
    m_image[1] = color.y * 255;
    m_image[2] = color.z * 255;
    m_image[3] = 255;

    m_width = 1;
    m_height = 1;
    m_channels = 4;
}

Image::Image(Image&& other) {
    m_image = other.m_image;
    m_width = other.m_width;
    m_height = other.m_height;
    m_channels = other.m_channels;
    m_is_hdr = other.m_is_hdr;
    other.m_image = nullptr;
}

Image&
Image::operator=(Image&& other) {
    if (m_image != nullptr)
        std::free(m_image);
    m_image = other.m_image;
    m_width = other.m_width;
    m_height = other.m_height;
    m_channels = other.m_channels;
    m_is_hdr = other.m_is_hdr;
    other.m_image = nullptr;

    return *this;
}

Image::~Image() {
    if (m_image != nullptr)
        std::free(m_image);
}

void
Image::scale(stage_vec3f scale) {
    if (!isValid()) return;
    for (int i = 0; i < m_width * m_height * m_channels; i++) {
        int channel = i % m_channels;
        if (channel == m_channels-1) continue; // Don't scale alpha
        m_image[i] *= scale[channel];
    }
}

void
Image::scale(Image& other) {
    if (!isValid() || !other.isValid()) return;
    if (m_width != other.getWidth() || m_height != other.getHeight() || m_channels != other.getChannels()) {
        WARN("Cannot scale image with another image of different dimensions");
        return;
    }
    
    for (int i = 0; i < m_width * m_height * m_channels; i++) {
        m_image[i] *= other.getData()[i];
    }
}

void
Image::mix(stage_vec3f color, stage_vec3f amount) {
    if (!isValid()) return;
    for (int i = 0; i < m_width * m_height * m_channels; i++) {
        int channel = i % m_channels;
        if (channel == m_channels-1) continue; // Don't scale alpha
        m_image[i] = m_image[i] * (1.f - amount[channel]) + color[channel] * amount[channel];
    }
}

void
Image::mix(Image& other, stage_vec3f amount) {
    if (!isValid() || !other.isValid()) return;
    if (m_width != other.getWidth() || m_height != other.getHeight() || m_channels != other.getChannels()) {
        WARN("Cannot mix image with another image of different dimensions");
        return;
    }
    
    for (int i = 0; i < m_width * m_height * m_channels; i++) {
        int channel = i % m_channels;
        if (channel == m_channels-1) continue; // Don't scale alpha
        m_image[i] = m_image[i] * (1.f - amount[channel]) + other.getData()[i] * amount[channel];
    }
}

}
}