#include <exception>

struct my_excep : std::exception {
    const char *
    what() const noexcept; // throw() is old code that means the same thing
};
