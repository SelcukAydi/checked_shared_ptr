#include <memory>
#include <type_traits>

namespace sia
{
    template <typename T>
    class checked_shared_ptr final
    {
        template <typename U>
        friend class checked_shared_ptr;

        std::shared_ptr<T> m_ptr{nullptr};

        template <typename... Args>
        using Constructible = std::enable_if_t<std::is_constructible_v<std::shared_ptr<T>, Args...>>;

    public:
        using element_type = typename std::shared_ptr<T>::element_type;

        // Conversion constructor.
        //
        checked_shared_ptr(const std::shared_ptr<T> &ptr) : m_ptr(ptr) {}

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
    };
}