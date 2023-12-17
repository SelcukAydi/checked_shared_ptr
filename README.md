#Checked Shared Pointer

## Features

### Mimicking std::shared_ptr
*checked_shared_ptr* mimicks the std::shared_ptr. The library provides all the C++17 features of std::shared_ptr by the same APIs and methods. For example, std::make_shared can be used to create a new checked_shared_ptr instance. Moreover, a checked_shared_ptr can be created through std::shared_ptr instances while the vice verse is forbidden. Internally, a checked_shared_ptr manages a std::shared_ptr instance not a raw pointer. Thus, it is very easy to access to the real std::shared_ptr instance by the provided APIs.

### Exception Throwing
checked_shared_ptr throws an exception when the pointer is dereferenced and tries to access an invalid address, nullptr in this case. Thus, for each pointer access internally an if-check is performed may affect the overall performance. However, this trade-off is not benchmarked yet and it is developer's responsibility to see the impact. On the other hand, this exception handling provides a very clean pattern to get rid of segfaults, which is the nightmare of almost all C++ developers. Rather than getting a signal from OS we prevent it and handle the exception softly.

### Very Customizable
The source code itself is pretty readable and it is very easy to adapt to any requirement. For example, if you want to get a backtrace on an exception you can rewrite your exception entities.

### Installation
checked_shared_ptr is a header-only library with a single file. You do not need to compile it at all. Just include the corresponding file somewhere that your project knows, mostly the include-path, and have fun!

### UTs
To be able to enable the UTs you need to set a flag 'ENABLE_CSP_TEST' to 'ON'. Moreover, the only dependency is gtest, which should be installed in your system already.
