#include "mystd/allocator.h"
#include <cassert>

struct Test {
    int id;
    float num;
};

int main() {
    mystd::PoolAllocator<Test> pool;
    Test *a = pool.construct(1, 2.0);
    Test *b = pool.construct(2, 3.0);
    Test *c = pool.construct(3, 4.5);

    pool.destroy(a);
    Test *d = pool.construct(4, 3.3);
    assert(a == d);
}
