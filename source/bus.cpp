#include "bus.hpp"

void Bus::detach_all() {
    for (size_t i = 0; i < harnesses.size(); i += 1) {
        auto [self, harness] = harnesses[i];
        harness->bus.reset();
    }

    harnesses.clear();

    level = 0;
}
