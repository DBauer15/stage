#include "buffer.h"

#include <stdexcept>

#include "log.h"

namespace stage {
namespace backstage {


Buffer::Buffer(Buffer& other) {
    m_data = other.m_data;
    m_size_in_bytes = other.m_size_in_bytes;
    m_has_ownership = other.m_has_ownership;

    other.m_has_ownership = false;
}

Buffer&
Buffer::operator=(Buffer& other) {
    m_data = other.m_data;
    m_size_in_bytes = other.m_size_in_bytes;
    m_has_ownership = other.m_has_ownership;

    other.m_has_ownership = false;
    return *this;
}


Buffer::~Buffer() {
    if (m_data != nullptr && m_has_ownership) {
        std::free(m_data);
    }
}

void
Buffer::data(uint8_t* blob, size_t size) {
    resize(size);
    std::memcpy(m_data, blob, m_size_in_bytes);
}

void
Buffer::data(std::vector<uint8_t> blob) {
    resize(blob.size());
    std::memcpy(m_data, blob.data(), m_size_in_bytes);
}

void
Buffer::resize(size_t newsize_in_bytes) {
    if (newsize_in_bytes == m_size_in_bytes) return;
    if (!m_has_ownership)
        throw std::runtime_error("Cannot resize buffer that is not owned");

    if (m_data != nullptr) {
        m_data = (uint8_t*)std::realloc(m_data, newsize_in_bytes);
    } else {
        m_data = (uint8_t*)std::malloc(newsize_in_bytes);
    }
    m_size_in_bytes = newsize_in_bytes;
}

}
}