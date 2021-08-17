#ifndef H5COMPOSITES_HANDLE_H
#define H5COMPOSITES_HANDLE_H

#include "H5Cpp.h"
#include "H5Composites/DTypes.h"

namespace H5Composites {
    template <typename T>
    class Handle
    {
    public:
        Handle(H5::Group& group, const std::string& name, const T& value);

        ~Handle();

        // Explicitly disallow copying
        Handle(const Handle& other) = delete;
        Handle(Handle&& other);

        bool enabled() const { return m_enabled; }
        void disable() { m_enabled = false; }
        void enable() { m_enabled = true; }

        T& operator*() { return m_value; }
        const T& operator*() const { return m_value; }
        T* operator->() { return &m_value; }
        const T* operator->() const { return &m_value; }
    private:
        H5::Group& m_targetGroup;
        std::string m_name;
        T m_value;
        bool m_enabled{true};
    }; //> end class Handle<T>

    template <typename T>
    Handle<T> makeHandle(H5::Group& group, const std::string& name, const T& value)
    {
        return Handle<T>(group, name, value);
    }
} //> end namespace H5Composites

#include "H5Composites/Handle.icc"

#endif //> !H5COMPOSITES_HANDLE_H