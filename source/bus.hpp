#pragma once

#include <cassert>
#include <cstdlib>
#include <map>
#include <optional>
#include <spdlog/spdlog.h>
#include <vector>

struct BusHarness;

struct Bus {
    std::vector<std::tuple<void *, BusHarness *>> harnesses;
    size_t                                        level = 0;
    const char                                   *name  = "unknown";

    Bus(const char *name) : name(name) {
    }

    void detach_all();
};

struct BusHarness {
    std::optional<Bus *> bus                                = std::nullopt;
    const char          *owner_name                         = "unknown";
    bool (*bus_level_updated)(void *self, size_t bus_level) = nullptr;
    size_t lower_threshold                                  = std::numeric_limits<size_t>::min(); // inclusive
    size_t upper_threshold                                  = std::numeric_limits<size_t>::max(); // inclusive

    BusHarness(const char *owner_name, bool (*bus_level_updated)(void *self, size_t bus_level) = nullptr,
               size_t      lower_threshold = std::numeric_limits<size_t>::min(),
               size_t      upper_threshold = std::numeric_limits<size_t>::max())
        : owner_name(owner_name), bus_level_updated(bus_level_updated), lower_threshold(lower_threshold),
          upper_threshold(upper_threshold) {
    }

    // void set_callback() {
    //     assert(!this->bus_level_updated);
    //     this->bus_level_updated = bus_level_updated;
    // }

    void attach(void *self, Bus &bus) {
        assert(!this->bus.has_value());
        this->bus = &bus;
        bus.harnesses.push_back(std::make_tuple(self, this));
        spdlog::debug("Attaching {} to {} (total {} harnesses), has callback: {}", this->owner_name, bus.name,
                      bus.harnesses.size(), (this->bus_level_updated != nullptr));
    }

    size_t get() {
        assert(this->bus.has_value());
        return this->bus.value()->level;
    }

    void put(size_t level) {
        assert(this->bus.has_value());
        auto *bus  = this->bus.value();
        bus->level = level;
        // spdlog::trace("Bus {} put @ {} : {:02} ({:02X})", bus->name, this->owner_name, level, level);
        // bool was_received_total = false;
        for (size_t i = 0; i < bus->harnesses.size(); i += 1) {
            auto [self, harness] = bus->harnesses[i];
            // spdlog::debug("Trying bus_level_updated in {}, has callback: {}, ({:X},{:X})", harness->owner_name,
            //               (harness->bus_level_updated != nullptr, harness->lower_threshold,
            //               harness->upper_threshold));
            if (harness != this && harness->bus_level_updated) {
                if ((level >= harness->lower_threshold) && (level <= harness->upper_threshold)) {
                    harness->bus_level_updated(self, level);
                    // auto was_received_this = harness->bus_level_updated(self, level);
                    // if (!was_received_total && was_received_this) {
                    //     was_received_total = true;
                    // }
                }
            }
        }
        // if (!was_received_total) {
        //     spdlog::trace("Bus {} put @ {} : {:02} ({:02X}) HAD NO RECEIVER", bus->name, this->owner_name, level,
        //                   level);
        // }
    }
};
