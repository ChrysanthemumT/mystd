#include "cassert"
#include "mystd/robin_map.h"

int main() {
    mystd::rmap<int, int> mp;
    mp.insert(1, 2);
    assert(mp.find(1).second);
    assert((mp.find(1).first)->second == 2);
    return 0;
};
