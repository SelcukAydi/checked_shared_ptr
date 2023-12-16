#include <gtest/gtest.h>
#include "checked_shared_ptr.hpp"

struct Person
{
    std::uint64_t m_id{};
    std::string m_name{};

    friend std::ostream& operator<<(std::ostream& os, Person& person);

protected:
    virtual ~Person() = default;
};

std::ostream& operator<<(std::ostream& os, Person& person)
{
    os << "Person=[(ID= " << person.m_id << ")(Name=" << person.m_name << ")]";
    return os;
}

struct Developer final : Person
{
    std::uint8_t m_num_of_tasks{};

    friend std::ostream& operator<<(std::ostream& os, Developer& developer);
};

std::ostream& operator<<(std::ostream& os, Developer& developer)
{
    os << "Developer=[" << static_cast<Person&>(developer) << "(Tasks=" << (std::uint32_t)developer.m_num_of_tasks << ")]";
    return os;
}

struct Manager final : Person
{
    std::string m_current_task_name{};

    friend std::ostream& operator<<(std::ostream& os, Manager& manager);
};

std::ostream& operator<<(std::ostream& os, Manager& manager)
{
    os << "Manager=[" << static_cast<Person&>(manager) << "(TaskName=" << manager.m_current_task_name << ")]";
    return os;
}

struct Base
{
    ~Base()
    {
        std::cout << "~Base()\n";
    }
};

struct Derived : Base
{
    ~Derived()
    {
        std::cout << "~Derived()\n";
    }
};

TEST(Basic, test1)
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

    Base base = std::move(Derived{});
    base = Derived{};

    static_assert(std::is_constructible_v<Base, Derived>, "");

    EXPECT_THROW(invalid_cast->m_current_task_name, sia::CheckedNullPtrException);
}