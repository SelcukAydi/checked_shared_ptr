#include <memory>
#include <type_traits>

namespace sia
{
    template <typename T>
    struct MyPtr : public std::shared_ptr<T>
    {
    };

    struct CheckedNullPtrException : std::exception
    {
    };

    template <typename T>
    class checked_shared_ptr final
    {
        template <typename U>
        friend class checked_shared_ptr;

        std::shared_ptr<T> m_ptr{nullptr};

        template <typename... Args>
        using Constructible = std::enable_if_t<std::is_constructible_v<std::shared_ptr<T>, Args...>>;

        template <typename... Args>
        using Assignable = std::enable_if_t<std::is_assignable_v<std::shared_ptr<T>, Args...>>;

    public:
        using element_type = typename std::shared_ptr<T>::element_type;

        // Conversion constructor.
        //
        checked_shared_ptr(const std::shared_ptr<T> &ptr) : m_ptr(ptr) {}

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U *, T *>>>
        checked_shared_ptr(const std::shared_ptr<U> &ptr) : m_ptr(ptr) {}

        // template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<std::shared_ptr<T>, Args...>>>
        // checked_shared_ptr(Args &&...args)
        // {
        //     m_ptr = std::make_shared<T>(std::forward<Args>(args)...);
        // }

        constexpr checked_shared_ptr() noexcept = default;
        constexpr checked_shared_ptr(std::nullptr_t) noexcept {}

        template <typename U, typename = Constructible<U *>>
        checked_shared_ptr(U *ptr) : m_ptr(ptr) {}

        template <typename U>
        checked_shared_ptr(const checked_shared_ptr<U> &r, element_type *ptr) noexcept : m_ptr(r.m_ptr, ptr) {}

        template <typename U>
        checked_shared_ptr(checked_shared_ptr<U> &&r, element_type *ptr) noexcept : m_ptr(std::move(r.m_ptr), ptr) {}

        checked_shared_ptr(const checked_shared_ptr &r) noexcept = default;

        template <typename U, typename = Constructible<std::shared_ptr<U>>>
        checked_shared_ptr(checked_shared_ptr<U> &r) noexcept : m_ptr(r.m_ptr) {}

        checked_shared_ptr(checked_shared_ptr &&r) noexcept : m_ptr(std::move(r.m_ptr)) {}

        template <typename U, typename = Constructible<std::shared_ptr<U>>>
        checked_shared_ptr(checked_shared_ptr<U> &&r) noexcept : m_ptr(std::move(r)) {}

        checked_shared_ptr &operator=(const checked_shared_ptr &) noexcept = default;

        template <typename U, typename = Assignable<const std::shared_ptr<U> &>>
        checked_shared_ptr &operator=(const checked_shared_ptr<U> &r)
        {
            m_ptr = r;
        }

        checked_shared_ptr &operator=(checked_shared_ptr &&r) noexcept
        {
            m_ptr = r;
        }

        template <typename U, typename = Assignable<std::shared_ptr<U>>>
        checked_shared_ptr &operator=(checked_shared_ptr &&r) noexcept
        {
            m_ptr = r;
        }

        void reset() noexcept
        {
            m_ptr.reset();
        }

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U *, T *>>>
        void reset(U *ptr) noexcept
        {
            m_ptr.reset(ptr);
        }

        void swap(checked_shared_ptr &r) noexcept
        {
            m_ptr.swap(r);
        }

        element_type *get() const noexcept
        {
            return m_ptr.get();
        }

        element_type &operator*() const noexcept(false)
        {
            throwIfNullPtr();
            return *m_ptr;
        }

        element_type *operator->() const noexcept(false)
        {
            throwIfNullPtr();
            return m_ptr.get();
        }

        explicit operator bool() const noexcept
        {
            return m_ptr.operator bool();
        }

        auto managedSharedPointer() const noexcept
        {
            return m_ptr;
        }

    private:
        inline void throwIfNullPtr() const noexcept(false)
        {
            if (get() == nullptr)
                throw CheckedNullPtrException();
        }
    };

    template <typename Ch, typename Tr, typename Tp>
    inline std::basic_ostream<Ch, Tr> &operator<<(std::basic_ostream<Ch, Tr> &os,
                                                  const checked_shared_ptr<Tp> &p)
    {
        os << p.get();
        return os;
    }

    template <typename T>
    inline bool operator==(const ::sia::checked_shared_ptr<T> &lhs, std::nullptr_t) noexcept
    {
        return lhs.get() == nullptr;
    }

    template <typename T>
    inline bool operator==(std::nullptr_t, const ::sia::checked_shared_ptr<T> &lhs) noexcept
    {
        return lhs.get() == nullptr;
    }

    template <typename T, typename U>
    inline bool
    operator==(const checked_shared_ptr<T> &lhs, const checked_shared_ptr<U> &rhs) noexcept(std::declval<checked_shared_ptr<T>>().get() == std::declval<checked_shared_ptr<U>>().get())
    {
        return lhs.get() == rhs.get();
    }

    template <typename T>
    inline bool operator!=(const ::sia::checked_shared_ptr<T> &lhs, std::nullptr_t) noexcept
    {
        return lhs.get() != nullptr;
    }

    template <typename T>
    inline bool operator!=(std::nullptr_t, const ::sia::checked_shared_ptr<T> &lhs) noexcept
    {
        return lhs.get() != nullptr;
    }

    template <typename T, typename U>
    inline bool
    operator!=(const checked_shared_ptr<T> &lhs, const checked_shared_ptr<U> &rhs) noexcept(std::declval<checked_shared_ptr<T>>().get() != std::declval<checked_shared_ptr<U>>().get())
    {
        return lhs.get() != rhs.get();
    }

    template <typename T, typename U>
    inline bool operator<(const checked_shared_ptr<T> &lhs, const checked_shared_ptr<U> &rhs) noexcept
    {
        using LhsT = typename checked_shared_ptr<T>::element_type;
        using RhsT = typename checked_shared_ptr<U>::element_type;
        using RsT = std::common_type_t<LhsT *, RhsT *>;
        return std::less<RsT>()(lhs.get(), rhs.get());
    }

    template <typename T>
    inline bool operator<(const checked_shared_ptr<T> &r, std::nullptr_t) noexcept
    {
        using RsT = typename checked_shared_ptr<T>::element_type;
        return std::less<RsT *>()(r.get(), nullptr);
    }

    template <typename T>
    inline bool operator<(std::nullptr_t, const checked_shared_ptr<T> &r) noexcept
    {
        using RsT = typename checked_shared_ptr<T>::element_type;
        return std::less<RsT *>()(r.get(), nullptr);
    }

    template <typename T, typename U>
    inline bool operator<=(const checked_shared_ptr<T> &lhs, const checked_shared_ptr<U> &rhs) noexcept
    {
        return !(rhs < lhs);
    }

    template <typename T>
    inline bool operator<=(const checked_shared_ptr<T> &r, std::nullptr_t) noexcept
    {
        return !(nullptr < r);
    }

    template <typename T>
    inline bool operator<=(std::nullptr_t, const checked_shared_ptr<T> &r) noexcept
    {
        return !(r < nullptr);
    }

    template <typename T, typename U>
    inline bool operator>(const checked_shared_ptr<T> &lhs, const checked_shared_ptr<U> &rhs) noexcept
    {
        return rhs < lhs;
    }

    template <typename T>
    inline bool operator>(const checked_shared_ptr<T> &r, std::nullptr_t) noexcept
    {
        return nullptr < r;
    }

    template <typename T>
    inline bool operator>(std::nullptr_t, const checked_shared_ptr<T> &r) noexcept
    {
        return r < nullptr;
    }

    template <typename T, typename U>
    inline bool operator>=(const checked_shared_ptr<T> &lhs, const checked_shared_ptr<U> &rhs) noexcept
    {
        return !(lhs < rhs);
    }

    template <typename T>
    inline bool operator>=(const checked_shared_ptr<T> &r, std::nullptr_t) noexcept
    {
        return !(r < nullptr);
    }

    template <typename T>
    inline bool operator>=(std::nullptr_t, const checked_shared_ptr<T> &r) noexcept
    {
        return !(nullptr < r);
    }

    template <typename T>
    inline void swap(checked_shared_ptr<T> &a, checked_shared_ptr<T> &b)
    {
        a.m_ptr.swap(b.m_ptr);
    }
}

namespace std
{
    template <typename _Tp>
    struct hash<sia::checked_shared_ptr<_Tp>>
        : public __hash_base<size_t, shared_ptr<_Tp>>
    {
        size_t operator()(const sia::checked_shared_ptr<_Tp> &__s) const noexcept
        {
            return std::hash<typename sia::checked_shared_ptr<_Tp>::element_type *>()(__s.get());
        }
    };

    template <typename T, typename U>
    inline ::sia::checked_shared_ptr<T> static_pointer_cast(const ::sia::checked_shared_ptr<U> &__r) noexcept
    {
        using _Sp = ::sia::checked_shared_ptr<T>;
        return _Sp(__r, static_cast<typename _Sp::element_type *>(const_cast<typename ::sia::checked_shared_ptr<U>::element_type *>(__r.get())));
    }

    template <typename T, typename U>
    inline ::sia::checked_shared_ptr<T> dynamic_pointer_cast(const ::sia::checked_shared_ptr<U> &__r) noexcept
    {
        using _Sp = ::sia::checked_shared_ptr<T>;
        return _Sp(__r, dynamic_cast<typename _Sp::element_type *>(const_cast<typename ::sia::checked_shared_ptr<U>::element_type *>(__r.get())));
    }

    template <typename T, typename U>
    inline ::sia::checked_shared_ptr<T> const_pointer_cast(const ::sia::checked_shared_ptr<U> &__r) noexcept
    {
        using _Sp = ::sia::checked_shared_ptr<T>;
        return _Sp(__r, static_cast<typename _Sp::element_type *>(const_cast<typename ::sia::checked_shared_ptr<U>::element_type *>(__r.get())));
    }
}