#include "mystd/allocator.h"
#include <cassert>
#include <iostream>

struct Order {
    int id;
    double price;
};

int main() {
    mystd::PoolAllocator<Order, 4> pool;

    Order *a = pool.construct(1, 99.5);
    Order *b = pool.construct(2, 100.0);
    Order *c = pool.construct(3, 101.0);
    Order *d = pool.construct(4, 102.0);

    Order *e = pool.construct(5, 103.0);
    assert(e != nullptr);
    std::cout << "new block allocated ok\n";

    pool.destroy(a);
    Order *f = pool.construct(6, 104.0);
    assert(f == a); // should reuse a's slot
    std::cout << "reuse ok\n";

    pool.destroy(b);
    pool.destroy(c);
    pool.destroy(d);
    pool.destroy(e);
    pool.destroy(f);
    std::cout << "all destroyed ok\n";
}
