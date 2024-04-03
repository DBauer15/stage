#pragma once

#include <cstdint>
#include <string>
#include "math.h"

namespace stage {
namespace backstage {

struct Image {

    public:
        Image(std::string filename, bool is_hdr = false);
        Image(uint8_t* blob, size_t size, bool is_hdr = false);
        Image(stage_vec3f color);
        Image(Image& other) = delete;
        Image(Image&& other);
        Image& operator=(Image& other) = delete;
        Image& operator=(Image&& other);
        ~Image();

        uint8_t* getData() { return m_image; }
        uint32_t getWidth() { return m_width; }
        uint32_t getHeight() { return m_height; }
        uint32_t getChannels() { return m_channels; }

        void scale(stage_vec3f scale);
        void scale(Image& other);

        void mix(stage_vec3f color, stage_vec3f amount);
        void mix(Image& other, stage_vec3f amount);

        bool isHDR() { return m_is_hdr; }
        bool isValid() { return m_image != nullptr; }

    private:
        uint8_t* m_image { nullptr };

        int32_t m_width;
        int32_t m_height;
        int32_t m_channels;

        bool m_is_hdr { false };
};

}
}