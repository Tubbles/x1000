#pragma once

#include <cassert>
#include <cstdlib>
#include <limits>
#include <map>
#include <optional>
#include <spdlog/spdlog.h>
#include <vector>

struct BusHarness;

struct Bus {
    std::vector<std::tuple<void *, BusHarness *>> harnesses;
    size_t                                        level = 0;
    std::string                                   name  = "unknown";

    Bus(std::string name) : name(name) {
    }

    void detach_all();
};

struct BusHarness {
    Bus        *bus                                         = nullptr;
    std::string owner_name                                  = "unknown";
    bool (*bus_level_updated)(void *self, size_t bus_level) = nullptr;

    BusHarness(std::string owner_name, bool (*bus_level_updated)(void *self, size_t bus_level) = nullptr)
        : owner_name(owner_name), bus_level_updated(bus_level_updated) {
    }

    // void set_callback() {
    //     assert(!this->bus_level_updated);
    //     this->bus_level_updated = bus_level_updated;
    // }

    void attach(void *self, Bus &bus) {
        assert(!this->bus);
        this->bus = &bus;
        bus.harnesses.push_back(std::make_tuple(self, this));
        spdlog::debug("Attaching {} to {} (total {} harnesses), has callback: {}", this->owner_name, bus.name,
                      bus.harnesses.size(), (this->bus_level_updated != nullptr));
    }

    size_t get() {
        return this->bus->level;
    }

    void put(size_t level) {
        auto *bus  = this->bus;
        bus->level = level;
        // spdlog::trace("Bus {} put @ {} : {:02} ({:02X})", bus->name, this->owner_name, level, level);
        for (size_t i = 0; i < bus->harnesses.size(); i += 1) {
            auto &[self, harness] = bus->harnesses[i];
            // spdlog::debug("Trying bus_level_updated in {}, has callback: {}, ({:X},{:X})", harness->owner_name,
            //               (harness->bus_level_updated != nullptr, harness->lower_threshold,
            //               harness->upper_threshold));
            if (harness != this && harness->bus_level_updated) {
                harness->bus_level_updated(self, level);
            }
        }
    }
};
