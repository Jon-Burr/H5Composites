#ifndef H5COMPOSITES_SMARTBUFFER_HXX
#define H5COMPOSITES_SMARTBUFFER_HXX

#include <cstddef>

namespace H5Composites {
    /**
     * @brief RAII behaviour for a memory buffer allocated with malloc and free
     */
    class SmartBuffer {
    public:
        /// Default constructor creates an empty pointer
        SmartBuffer();
        /// Create a new slice of memory with the given size (in bytes)
        SmartBuffer(std::size_t size);
        /// Create a new slice of memory with the given size (in bytes) and fill it with the
        /// specified value
        SmartBuffer(std::size_t size, unsigned char fill);
        /**
         * @brief Take ownership of a void pointer
         *
         * If the void* was not allocated by std::malloc then this will lead to undefined behaviour!
         */
        SmartBuffer(void *buffer);
        /// Take ownership from another SmartBuffer
        SmartBuffer(SmartBuffer &&other);
        /// Move assignment operator
        SmartBuffer &operator=(SmartBuffer &&other);
        /// Assign to null
        SmartBuffer &operator=(std::nullptr_t);
        /// Destroy the buffer, freeing its memory
        ~SmartBuffer();

        /// Create a smart buffer by copying a value
        template <typename T> static SmartBuffer copyValue(const T &value, std::size_t n = 1) {
            return copy(&value, sizeof(T), n);
        }

        static SmartBuffer copy(const void *ptr, std::size_t nBytes, std::size_t n = 1);

        explicit operator bool() const;

        /// Set a new managed object
        void reset(void *buffer);

        /// Get access to the buffer
        void *get();
        /// Get (const) access to the buffer
        const void *get() const;
        /// Get access to the buffer with an offset
        void *get(std::size_t offset);
        /// Get (const) access to the buffer with an offset
        const void *get(std::size_t offset) const;
        /// Release ownership of the buffer
        void *release();
        /// Resize the owned memory
        bool resize(std::size_t size);

    private:
        void *m_buffer;
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_SMARTBUFFER_HXX