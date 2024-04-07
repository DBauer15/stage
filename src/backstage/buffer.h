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

    void resize(size_t newsize);

    size_t size() { return m_size_in_bytes; }

private:
    uint8_t* m_data { nullptr };
    size_t m_size_in_bytes { 0 };

    bool m_has_ownership { true };
};

template<typename T>
struct BufferView {

    BufferView() = default;
    BufferView(std::shared_ptr<Buffer>& source, size_t offset, size_t size) : m_offset(offset), m_size(size) {
        m_buffer = source;
    }

    void setBuffer(std::shared_ptr<Buffer>& source) {
        m_offset = source->size();
        m_size = 0;
        m_buffer = source;
    }

    void setBuffer(std::shared_ptr<Buffer>& source, size_t offset, size_t size) {
        m_offset = offset;
        m_size = size;
        m_buffer = source;
    }

    T* data() { 
        if (!m_buffer) return nullptr; 
        return (T*)(m_buffer->data() + m_offset); 
    }

    void push_back(const T& element) {
        if (!m_buffer)
            return;
        size_t oldsize_in_bytes = m_buffer->size();
        m_buffer->resize(oldsize_in_bytes + sizeof(T));
        std::memcpy(m_buffer->data() + oldsize_in_bytes, &element, sizeof(T));
        m_size += 1;
    }

    void push_back(const std::vector<T>& elements) {
        if (!m_buffer)
            return;
        size_t oldsize_in_bytes = m_buffer->size();
        m_buffer->resize(oldsize_in_bytes + sizeof(T) * elements.size());
        std::memcpy(m_buffer->data() + oldsize_in_bytes, elements.data(), sizeof(T) * elements.size());
        m_size += elements.size();
    }

    size_t sizeInBytes() { return m_size * sizeof(T); }
    size_t size() { return m_size; }
    size_t offset() { return m_offset; }

    T& operator[](uint32_t id) { return data()[id]; }

private:
    std::shared_ptr<Buffer> m_buffer;
    size_t m_offset;
    size_t m_size;
};

}
}