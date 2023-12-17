#include "checked_shared_ptr.hpp"
#include <gtest/gtest.h>

struct BasicClass
{
    constexpr BasicClass() = default;
};
struct MidLevelClass
{
};

struct Person
{
    std::uint64_t m_id{};
    std::string m_name{};

    friend std::ostream &operator<<(std::ostream &os, Person &person); // NOLINT(readability-identifier-length)

    protected:
    virtual ~Person() = default;
};

std::ostream &operator<<(std::ostream &os, Person &person) // NOLINT(readability-identifier-length)
{
    os << "Person=[(ID= " << person.m_id << ")(Name=" << person.m_name << ")]";
    return os;
}

struct Developer final : Person
{
    std::uint8_t m_num_of_tasks{};

    friend std::ostream &operator<<(std::ostream &os, Developer &developer); // NOLINT(readability-identifier-length)
};

std::ostream &operator<<(std::ostream &os, Developer &developer) // NOLINT(readability-identifier-length)
{
    os << "Developer=[" << static_cast<Person &>(developer) << "(Tasks=" << static_cast<std::uint32_t>(developer.m_num_of_tasks)
       << ")]";
    return os;
}

struct Manager final : Person
{
    std::string m_current_task_name{};

    friend std::ostream &operator<<(std::ostream &os, Manager &manager); // NOLINT(readability-identifier-length)
};

std::ostream &operator<<(std::ostream &os, Manager &manager) // NOLINT(readability-identifier-length)
{
    os << "Manager=[" << static_cast<Person &>(manager) << "(TaskName=" << manager.m_current_task_name << ")]";
    return os;
}

struct Base
{
    ~Base()
    {
        // std::cout << "~Base()\n";
    }
};

struct Derived : Base
{
    ~Derived()
    {
        // std::cout << "~Derived()\n";
    }
};

struct PlainObject
{
    constexpr PlainObject() = default;

    std::int32_t m_id{};
    std::string m_name{};
};

inline bool operator==(const PlainObject& lhs, const PlainObject& rhs)
{
    return (lhs.m_id == rhs.m_id) && (lhs.m_name == rhs.m_name);
}

TEST(CheckedSharedPtr, Mixed)
{
    sia::checked_shared_ptr<Person> developer = std::make_shared<Developer>();
    sia::checked_shared_ptr<Person> manager = std::make_shared<Manager>();

    developer->m_id = 20;
    developer->m_name = "sia";

    manager->m_id = 40;
    manager->m_name = "john";

    auto developer_ptr = std::static_pointer_cast<Developer>(developer);
    auto manager_ptr = std::static_pointer_cast<Manager>(manager);

    developer_ptr->m_num_of_tasks = 201;
    manager_ptr->m_current_task_name = "digit handling";

    std::cout << *developer_ptr << '\n';
    std::cout << *manager_ptr << '\n';

    auto valid_cast = std::dynamic_pointer_cast<Manager>(manager_ptr);
    auto invalid_cast = std::dynamic_pointer_cast<Manager>(developer_ptr);

    EXPECT_FALSE(valid_cast == nullptr);
    EXPECT_TRUE(invalid_cast == nullptr);
}

TEST(CheckedSharedPtr, EnableSharedFromThis)
{
    struct EnabledObject : public std::enable_shared_from_this<EnabledObject>
    {
    };
    struct Object
    {
    };

    sia::checked_shared_ptr<EnabledObject> ptr1 = std::make_shared<EnabledObject>();
    sia::checked_shared_ptr<Object> ptr2 = std::make_shared<Object>();

    std::cout << ptr1.use_count() << '\n';

    {
        auto ptr3 = ptr1.shared_from_this();
        EXPECT_TRUE(ptr3.get() == ptr1.get());
    }

    // Ptr2 cannot have shared_from_this() method ever.
    //
    static_assert(!std::is_base_of_v<std::enable_shared_from_this<Object>, decltype(*ptr2.get())>);
}

TEST(CheckedSharedPtr, ConversionCtor1)
{
    auto ptr = std::make_shared<Base>();
    sia::checked_shared_ptr<Base> c_ptr = ptr;
    sia::checked_shared_ptr<Base> c_ptr_derived = std::make_shared<Derived>();
    std::shared_ptr<Derived> ptr_derived = std::static_pointer_cast<Derived>(c_ptr_derived).managedSharedPointer();

    EXPECT_TRUE(ptr.get() == c_ptr.get());
    EXPECT_TRUE(ptr_derived.get() == c_ptr_derived.get());
}

TEST(CheckedSharedPtr, ConversionCtor2)
{
    std::shared_ptr<Derived> ptr = std::make_shared<Derived>();
    sia::checked_shared_ptr<Base> c_ptr{ptr};

    EXPECT_TRUE(ptr.get() == c_ptr.get());
    EXPECT_TRUE(ptr.use_count() == c_ptr.use_count());
}

TEST(CheckedSharedPtr, DefaultCtor)
{
    sia::checked_shared_ptr<Base> c_ptr{};
    EXPECT_TRUE(c_ptr.get() == nullptr);
}

TEST(CheckedSharedPtr, CtorWithNullptr)
{
    sia::checked_shared_ptr<Derived> c_ptr{nullptr};
    EXPECT_TRUE(c_ptr.get() == nullptr);
}

TEST(CheckedSharedPtr, CtorWithRawPtr)
{
    Base *ptr = new Base();

    sia::checked_shared_ptr<Base> c_ptr{ptr};
    EXPECT_TRUE(ptr == c_ptr.get());

    // No need to delete ptr cause it is managed by the checked_shared_ptr.
    //
}

TEST(CheckedSharedPtr, AlisingCtor1)
{
    sia::checked_shared_ptr<Derived> c_ptr_derived = std::make_shared<Derived>();
    sia::checked_shared_ptr<Base> c_ptr_base{c_ptr_derived, c_ptr_derived.get()};

    EXPECT_TRUE(c_ptr_base.get() == c_ptr_derived.get());
}

TEST(CheckedSharedPtr, AlisingCtor2)
{
    sia::checked_shared_ptr<Derived> c_ptr_derived = std::make_shared<Derived>();
    sia::checked_shared_ptr<Base> c_ptr_base{std::move(c_ptr_derived), c_ptr_derived.get()};

    // Rvalue overload of shared pointer is supported after C++17. As a result we do not have that support.
    // So the following will be true.
    //
    EXPECT_TRUE(c_ptr_base.get() == c_ptr_derived.get());
}

// TODO(selcuk): We may want to default the copy constructor cause the managed type
// may already be removed its copy constructor.
//

TEST(CheckedSharedPtr, DefaultCopyCtor)
{
    sia::checked_shared_ptr<Base> c_ptr_base = std::make_shared<Base>();
    sia::checked_shared_ptr<Base> c_ptr_base_other{c_ptr_base}; // NOLINT(performance-unnecessary-copy-initialization)

    EXPECT_TRUE(c_ptr_base.get() == c_ptr_base_other.get());
}

TEST(CheckedSharedPtr, ConversionCopyCtor)
{
    sia::checked_shared_ptr<Derived> c_ptr_base = std::make_shared<Derived>();
    sia::checked_shared_ptr<Base> c_ptr_base_other{c_ptr_base};

    EXPECT_TRUE(c_ptr_base.get() == c_ptr_base_other.get());
}

TEST(CheckedSharedPtr, DefaultMoveCtor)
{
    sia::checked_shared_ptr<Derived> c_ptr_derived = std::make_shared<Derived>();
    sia::checked_shared_ptr<Derived> c_ptr_derived_other{std::move(c_ptr_derived)};

    EXPECT_TRUE(c_ptr_derived.get() != c_ptr_derived_other.get());
    EXPECT_TRUE(c_ptr_derived.get() == nullptr);
}

TEST(CheckedSharedPtr, ConversionMoveCtor)
{
    sia::checked_shared_ptr<Derived> c_ptr_derived = std::make_shared<Derived>();
    sia::checked_shared_ptr<Base> c_ptr_derived_other{std::move(c_ptr_derived)};

    EXPECT_TRUE(c_ptr_derived.get() != c_ptr_derived_other.get());
    EXPECT_TRUE(c_ptr_derived.get() == nullptr);
}

TEST(CheckedSharedPtr, DefaultCopyAssignment)
{
    sia::checked_shared_ptr<BasicClass> c_ptr_base = std::make_shared<BasicClass>();
    sia::checked_shared_ptr<BasicClass> c_ptr_base_other{};

    c_ptr_base_other = c_ptr_base;

    EXPECT_TRUE(c_ptr_base.get() == c_ptr_base_other.get());
}

TEST(CheckedSharedPtr, ConversionCopyAssignment)
{
    sia::checked_shared_ptr<Derived> c_ptr_base = std::make_shared<Derived>();
    sia::checked_shared_ptr<Base> c_ptr_base_other{};

    c_ptr_base_other = c_ptr_base;

    EXPECT_TRUE(c_ptr_base.get() == c_ptr_base_other.get());
}

TEST(CheckedSharedPtr, DefaultMoveAssignment)
{
    sia::checked_shared_ptr<Derived> c_ptr_derived = std::make_shared<Derived>();
    sia::checked_shared_ptr<Derived> c_ptr_derived_other{};

    c_ptr_derived_other = std::move(c_ptr_derived);

    // Rvalue overload of shared pointer is supported after C++17. As a result we do not have that support.
    // So the following will be true.
    //
    EXPECT_TRUE(c_ptr_derived.get() == c_ptr_derived_other.get());

    // Rvalue overload is not triggered because it is not supported by std::shared_ptr
    //
    EXPECT_TRUE(c_ptr_derived.get() != nullptr);
}

TEST(CheckedSharedPtr, ConversionMoveAssignment)
{
    sia::checked_shared_ptr<Derived> c_ptr_derived = std::make_shared<Derived>();
    sia::checked_shared_ptr<Base> c_ptr_derived_other{};

    c_ptr_derived_other = std::move(c_ptr_derived);

    // Rvalue overload of shared pointer is supported after C++17. As a result we do not have that support.
    // So the following will be true.
    //
    EXPECT_TRUE(c_ptr_derived.get() == c_ptr_derived_other.get());

    // Rvalue overload is not triggered because it is not supported by std::shared_ptr
    //
    EXPECT_TRUE(c_ptr_derived.get() != nullptr);
}

TEST(CheckedSharedPtr, EqualOperator)
{
    sia::checked_shared_ptr<PlainObject> c_ptr1 = std::make_shared<PlainObject>();
    sia::checked_shared_ptr<PlainObject> c_ptr2 = std::make_shared<PlainObject>();

    c_ptr1->m_id = 100;
    c_ptr1->m_name = "sia";

    c_ptr2->m_id = 100;
    c_ptr2->m_name = "sia";

    EXPECT_NE(c_ptr1, c_ptr2);
    EXPECT_EQ(*c_ptr1, *c_ptr2);
    c_ptr1 = nullptr;
    EXPECT_EQ(c_ptr1, nullptr);
    c_ptr2 = nullptr;
    EXPECT_EQ(c_ptr1, c_ptr2);
}

TEST(CheckedSharedPtr, NotEqualOperator)
{
    sia::checked_shared_ptr<PlainObject> c_ptr1 = std::make_shared<PlainObject>();
    sia::checked_shared_ptr<PlainObject> c_ptr2 = std::make_shared<PlainObject>();

    c_ptr1->m_id = 100;
    c_ptr1->m_name = "sia";

    c_ptr2->m_id = 100;
    c_ptr2->m_name = "sia";

    EXPECT_TRUE(c_ptr1 != c_ptr2);
    EXPECT_EQ(*c_ptr1, *c_ptr2);

    c_ptr1 = nullptr;
    EXPECT_FALSE(c_ptr1 != nullptr);
    c_ptr2 = nullptr;
    EXPECT_FALSE(c_ptr1 != c_ptr2);
}

TEST(CheckedSharedPtr, LessOperator)
{
    sia::checked_shared_ptr<PlainObject> c_ptr1 = std::make_shared<PlainObject>();
    sia::checked_shared_ptr<PlainObject> c_ptr2 = std::make_shared<PlainObject>();

    c_ptr1->m_id = 100;
    c_ptr1->m_name = "sia";

    c_ptr2->m_id = 100;
    c_ptr2->m_name = "sia";

    EXPECT_FALSE(c_ptr1 < c_ptr2);
    c_ptr1 = nullptr;
    EXPECT_FALSE(c_ptr1 < nullptr);
    EXPECT_FALSE(nullptr < c_ptr2);
}

TEST(CheckedSharedPtr, GreaterOperator)
{
    sia::checked_shared_ptr<PlainObject> c_ptr1 = std::make_shared<PlainObject>();
    sia::checked_shared_ptr<PlainObject> c_ptr2 = std::make_shared<PlainObject>();

    c_ptr1->m_id = 100;
    c_ptr1->m_name = "sia";

    c_ptr2->m_id = 100;
    c_ptr2->m_name = "sia";

    EXPECT_TRUE(c_ptr2 > c_ptr1);
    c_ptr1 = nullptr;
    EXPECT_FALSE(c_ptr1 > nullptr);
    EXPECT_FALSE(nullptr > c_ptr2);
}

TEST(CheckedSharedPtr, LessOrEqualOperator)
{
    sia::checked_shared_ptr<PlainObject> c_ptr1 = std::make_shared<PlainObject>();
    sia::checked_shared_ptr<PlainObject> c_ptr2{c_ptr1}; // NOLINT(performance-unnecessary-copy-initialization)

    EXPECT_TRUE(c_ptr1 <= c_ptr2);
}

TEST(CheckedSharedPtr, GreaterOrEqualOperator)
{
    sia::checked_shared_ptr<PlainObject> c_ptr1 = std::make_shared<PlainObject>();
    sia::checked_shared_ptr<PlainObject> c_ptr2{c_ptr1}; // NOLINT(performance-unnecessary-copy-initialization)

    EXPECT_TRUE(c_ptr2 >= c_ptr1);
}

TEST(CheckedSharedPtr, Reset)
{
    sia::checked_shared_ptr<Base> c_ptr1 = std::make_shared<Base>();
    c_ptr1.reset();
    EXPECT_TRUE(c_ptr1 == nullptr);
    c_ptr1.reset(new Base());
    EXPECT_TRUE(c_ptr1 != nullptr);
}

TEST(CheckedSharedPtr, Swap)
{
    auto c_ptr1 = sia::make_checked_shared<PlainObject>();
    auto c_ptr2 = sia::make_checked_shared<PlainObject>();

    c_ptr1->m_id = 100;
    c_ptr1->m_name = "john";

    c_ptr2->m_id = 200;
    c_ptr2->m_name = "sia";

    sia::swap(c_ptr1, c_ptr2);

    EXPECT_TRUE(c_ptr1->m_id == 200);
    EXPECT_TRUE(c_ptr1->m_name == "sia");
    EXPECT_TRUE(c_ptr2->m_id == 100);
    EXPECT_TRUE(c_ptr2->m_name == "john");
}

TEST(CheckedSharedPtr, NullPtrAccess)
{
    auto c_ptr1 = sia::make_checked_shared<PlainObject>();
    c_ptr1 = nullptr;
    EXPECT_THROW(c_ptr1->m_name = "empty", sia::CheckedNullPtrException);
    EXPECT_THROW(*c_ptr1, sia::CheckedNullPtrException);
}