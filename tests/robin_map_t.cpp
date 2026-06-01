#include "cassert"
#include "mystd/robin_map.h"

int main() {
    mystd::rmap<int, int> mp;
    mp.insert(1, 2);
    assert(mp.find(1).second);
    assert((mp.find(1).first)->second == 2);
    assert(!(mp.find(2).second));
    mp[2] = 5;
    assert(mp.find(2).second);
    assert((mp.find(2).first)->second == 5);
    return 0;
};
