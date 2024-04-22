#pragma once
#include <vector>
#include <memory>
#include <cstdlib>
#include <cstring>

namespace stage {
namespace backstage {

struct Buffer {
    Buffer() = default;
    Buffer(uint8_t* blob, size_t size) { data(blob, size); }
    Buffer(std::vector<uint8_t> blob) { data(blob); }
    Buffer(Buffer& other);
    Buffer& operator=(Buffer& other);
    ~Buffer();

    uint8_t* data() { return m_data; };
    uint8_t* release() { 
        m_has_ownership = false;
        return m_data;
    };

    void data(uint8_t* blob, size_t size);
    void data(std::vector<uint8_t> blob);

    void resize(size_t newsize_in_bytes);

    size_t size() { return m_size_in_bytes; }

private:
    uint8_t* m_data { nullptr };
    size_t m_size_in_bytes { 0 };
    size_t m_capacity_in_bytes { 0 };

    bool m_has_ownership { true };
};

template<typename T>
struct BufferView {

    BufferView() = default;
    BufferView(std::shared_ptr<Buffer> source, size_t offset, size_t num_elements, size_t stride = sizeof(T)) : m_offset(offset), m_size(num_elements), m_stride(stride) {
        m_buffer = source;
    }

    void setBuffer(std::shared_ptr<Buffer> source) {
        m_offset = source->size();
        m_size = 0;
        m_stride = sizeof(T);
        m_buffer = source;
    }

    void setBuffer(std::shared_ptr<Buffer> source, size_t offset, size_t num_elements, size_t stride = sizeof(T)) {
        m_offset = source->size() + offset;
        m_size = num_elements;
        m_stride = stride;
        m_buffer = source;
    }

    uint8_t* data() const { 
        if (!m_buffer) return nullptr; 
        return (m_buffer->data() + m_offset); 
    }

    void push_back(const T& element) {
        if (!m_buffer)
            return;
        size_t oldsize_in_bytes = m_buffer->size();
        if (oldsize_in_bytes <= positionInBuffer())
            m_buffer->resize(oldsize_in_bytes + m_stride);
        std::memcpy(m_buffer->data() + positionInBuffer(), &element, sizeof(T));
        m_size += 1;
    }

    void push_back(std::vector<T>& elements) {
        if (!m_buffer)
            return;
        size_t oldsize_in_bytes = m_buffer->size();
        if (oldsize_in_bytes <= positionInBuffer())
            m_buffer->resize(positionInBuffer() + m_stride * elements.size());
        
        if (m_stride == sizeof(T)) {
            std::memcpy(m_buffer->data() + positionInBuffer(), elements.data(), sizeof(T) * elements.size());
        } else {
            for (size_t i = 0; i < elements.size(); i++) {
                std::memcpy(m_buffer->data() + positionInBuffer() + (i * m_stride), elements.data() + i, sizeof(T));
            }
        }
        m_size += elements.size();
    }

    size_t sizeInBytes() const { return m_size * sizeof(T); }
    size_t size() const { return m_size; }
    size_t offset() const { return m_offset; }
    size_t stride() const { return m_stride; }

    T& operator[](uint32_t id) const { 
        return *(T*)(data() + (id * m_stride));
    }

private:
    std::shared_ptr<Buffer> m_buffer;
    size_t m_offset;
    size_t m_stride;
    size_t m_size { 0 };

    size_t positionInBuffer() {
        return (m_offset + m_size * m_stride);
    }
};

}
}