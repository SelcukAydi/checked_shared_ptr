#include <gtest/gtest.h>
#include "checked_shared_ptr.hpp"

struct Object
{

};

TEST(Basic, test1)
{
    sia::checked_shared_ptr<Object> ptr = std::make_shared<Object>();    
}