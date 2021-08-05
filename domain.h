#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#include <string>
#include <list>
#include <optional>

namespace transcat {

    struct Stop {
        std::string name;
        double latitude = 0;
        double longitude = 0;
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
