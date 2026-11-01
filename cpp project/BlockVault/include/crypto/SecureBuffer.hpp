#pragma once

#include "common/Types.hpp"
#include <cstddef>
#include <vector>
#include <string>

namespace blockvault {

/**
 * @class SecureBuffer
 * @brief RAII memory container for sensitive cryptographic secrets.
 * 
 * SecureBuffer ensures that sensitive key material (KEK, DEK, master password)
 * is kept in locked physical memory (preventing disk swapping), cannot be 
 * accidentally duplicated via copying, has optional canary guard pages to detect
 * buffer overruns, and is guaranteed to be securely zeroized using libsodium's
 * sodium_memzero() upon destruction.
 */
class SecureBuffer {
private:
    uint8_t* m_data{nullptr};
    size_t   m_size{0};
    bool     m_is_sodium_allocated{false};

    void allocate(size_t size);
    void deallocate() noexcept;

public:
    SecureBuffer();
    explicit SecureBuffer(size_t size);
    SecureBuffer(const uint8_t* data, size_t size);
    explicit SecureBuffer(const ByteVector& vec);
    explicit SecureBuffer(const std::string& str);

    ~SecureBuffer() noexcept;

    // Non-copyable (Prevents accidental secret duplication in RAM)
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;

    // Move-constructible and Move-assignable
    SecureBuffer(SecureBuffer&& other) noexcept;
    SecureBuffer& operator=(SecureBuffer&& other) noexcept;

    // Observers
    [[nodiscard]] const uint8_t* data() const noexcept { return m_data; }
    [[nodiscard]] uint8_t* data() noexcept { return m_data; }
    [[nodiscard]] size_t size() const noexcept { return m_size; }
    [[nodiscard]] bool empty() const noexcept { return m_size == 0 || m_data == nullptr; }

    // Memory protection helpers (when backed by sodium_malloc)
    void make_readonly() noexcept;
    void make_readwrite() noexcept;
    void make_noaccess() noexcept;

    // Conversion helpers (creates a transient copy; caller must zeroize)
    [[nodiscard]] ByteVector to_vector() const;
    [[nodiscard]] std::string to_string() const;

    // Secure memory zeroization
    void zeroize() noexcept;
    
    // Clear and free memory
    void clear() noexcept;

    // Subscript access
    uint8_t& operator[](size_t index);
    const uint8_t& operator[](size_t index) const;
};

} // namespace blockvault
