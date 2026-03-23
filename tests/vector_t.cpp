#include "mystd/vector.h"
#include <iostream>

int main() {
    mystd::vector<int> v(2);
    v[0] = 3;
    std::cout << v.size() << std::endl;
    v.push_back(3);
    std::cout << v[0] << v[1] << v[2] << std::endl;
    std::cout << v.size() << std::endl;
    return 0;
}
