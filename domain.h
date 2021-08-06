#pragma once

#include <string>
#include <list>
#include <optional>
#include <map>

#include <transport_catalogue.pb.h>

namespace transcat {

    struct Stop {
        std::string name;
        double latitude = 0;
        double longitude = 0;

        pb3::Stop ToProto() const {
            pb3::Stop proto_stop;
            proto_stop.set_name(name);
            proto_stop.set_latitude(latitude);
            proto_stop.set_longitude(longitude);
            return proto_stop;
        }

        static Stop FromProto(const pb3::Stop &proto_stop) {
            return {
                    proto_stop.name(),
                    proto_stop.latitude(),
                    proto_stop.longitude()
            };
        }
    };

    using StopPtr = const Stop *;
    using Route = std::list<StopPtr>;

    struct Bus {
        std::string name;
        Route route;
        size_t unique_stops = 0;
        bool is_roundtrip = false;
        StopPtr start_stop = nullptr;
        StopPtr end_stop = nullptr;

        pb3::Bus ToProto() const {
            pb3::Bus proto_bus;
            proto_bus.set_name(name);
            proto_bus.set_unique_stops(unique_stops);
            proto_bus.set_is_roundtrip(is_roundtrip);
            for (StopPtr stop: route) {
                proto_bus.mutable_route()->Add(stop->ToProto());
            }
            *proto_bus.mutable_start_stop() = start_stop->ToProto();
            *proto_bus.mutable_end_stop() = end_stop->ToProto();
            return proto_bus;
        }

        static Bus
        FromProto(const pb3::Bus &proto_bus, const std::map<std::string_view, StopPtr> &stops_by_name) {
            Route route;
            for (const pb3::Stop &proto_stop: proto_bus.route()) {
                route.emplace_back(stops_by_name.at(proto_stop.name()));
            }
            return {
                    proto_bus.name(),
                    std::move(route),
                    proto_bus.unique_stops(),
                    proto_bus.is_roundtrip(),
                    stops_by_name.at(proto_bus.start_stop().name()),
                    stops_by_name.at(proto_bus.end_stop().name())
            };
        }
    };

    struct StopPair {
        StopPtr from = nullptr;
        StopPtr to = nullptr;

        StopPair Reverse() const noexcept;

        friend bool operator==(const StopPair &lhs, const StopPair &rhs) {
            return lhs.from == rhs.from && lhs.to == rhs.to;
        }
    };

    struct StopPairHasher {
        size_t operator()(const StopPair &stop_pair) const;
    };

    struct RoutingSettings {
        int bus_wait_time = 0;
        double bus_velocity = 0;
    };

} // namespace transcat
