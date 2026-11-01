#include "crypto/SecureBuffer.hpp"
#include "common/Exceptions.hpp"
#include <sodium.h>
#include <cstring>
#include <algorithm>

namespace blockvault {

void SecureBuffer::allocate(size_t size) {
    if (size == 0) {
        m_data = nullptr;
        m_size = 0;
        m_is_sodium_allocated = false;
        return;
    }

    // Attempt allocation via sodium_malloc to obtain guard pages & physical memory locking
    void* ptr = sodium_malloc(size);
    if (ptr != nullptr) {
        m_data = static_cast<uint8_t*>(ptr);
        m_size = size;
        m_is_sodium_allocated = true;
        sodium_memzero(m_data, m_size);
    } else {
        // Fallback to standard dynamic heap allocation if sodium_malloc limit is exceeded
        m_data = new uint8_t[size];
        m_size = size;
        m_is_sodium_allocated = false;
        sodium_memzero(m_data, m_size);
    }
}

void SecureBuffer::deallocate() noexcept {
    if (m_data) {
        if (m_is_sodium_allocated) {
            // sodium_free() automatically zeroes memory before releasing guard pages
            sodium_free(m_data);
        } else {
            sodium_memzero(m_data, m_size);
            delete[] m_data;
        }
        m_data = nullptr;
        m_size = 0;
        m_is_sodium_allocated = false;
    }
}

SecureBuffer::SecureBuffer() : m_data(nullptr), m_size(0), m_is_sodium_allocated(false) {}

SecureBuffer::SecureBuffer(size_t size) {
    allocate(size);
}

SecureBuffer::SecureBuffer(const uint8_t* data, size_t size) {
    if (data && size > 0) {
        allocate(size);
        std::memcpy(m_data, data, size);
    }
}

SecureBuffer::SecureBuffer(const ByteVector& vec) {
    if (!vec.empty()) {
        allocate(vec.size());
        std::memcpy(m_data, vec.data(), vec.size());
    }
}

SecureBuffer::SecureBuffer(const std::string& str) {
    if (!str.empty()) {
        allocate(str.size());
        std::memcpy(m_data, str.data(), str.size());
    }
}

SecureBuffer::~SecureBuffer() noexcept {
    deallocate();
}

SecureBuffer::SecureBuffer(SecureBuffer&& other) noexcept 
    : m_data(other.m_data), 
      m_size(other.m_size), 
      m_is_sodium_allocated(other.m_is_sodium_allocated) {
    other.m_data = nullptr;
    other.m_size = 0;
    other.m_is_sodium_allocated = false;
}

SecureBuffer& SecureBuffer::operator=(SecureBuffer&& other) noexcept {
    if (this != &other) {
        deallocate();
        m_data = other.m_data;
        m_size = other.m_size;
        m_is_sodium_allocated = other.m_is_sodium_allocated;

        other.m_data = nullptr;
        other.m_size = 0;
        other.m_is_sodium_allocated = false;
    }
    return *this;
}

void SecureBuffer::make_readonly() noexcept {
    if (m_data && m_is_sodium_allocated) {
        sodium_mprotect_readonly(m_data);
    }
}

void SecureBuffer::make_readwrite() noexcept {
    if (m_data && m_is_sodium_allocated) {
        sodium_mprotect_readwrite(m_data);
    }
}

void SecureBuffer::make_noaccess() noexcept {
    if (m_data && m_is_sodium_allocated) {
        sodium_mprotect_noaccess(m_data);
    }
}

ByteVector SecureBuffer::to_vector() const {
    if (!m_data || m_size == 0) return {};
    return ByteVector(m_data, m_data + m_size);
}

std::string SecureBuffer::to_string() const {
    if (!m_data || m_size == 0) return {};
    return std::string(reinterpret_cast<const char*>(m_data), m_size);
}

void SecureBuffer::zeroize() noexcept {
    if (m_data && m_size > 0) {
        if (m_is_sodium_allocated) {
            sodium_mprotect_readwrite(m_data);
        }
        sodium_memzero(m_data, m_size);
    }
}

void SecureBuffer::clear() noexcept {
    deallocate();
}

uint8_t& SecureBuffer::operator[](size_t index) {
    if (index >= m_size) {
        throw CryptoException("SecureBuffer index out of bounds");
    }
    return m_data[index];
}

const uint8_t& SecureBuffer::operator[](size_t index) const {
    if (index >= m_size) {
        throw CryptoException("SecureBuffer index out of bounds");
    }
    return m_data[index];
}

} // namespace blockvault
