#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <deque>
#include <vector>
#include <set>
#include <optional>
#include <filesystem>

#include "domain.h"
#include "graph.h"
#include "router.h"

namespace transcat {

    using distance_t = int;

    class TransportCatalogue {
    public:
        struct BusPtrComparator {
            bool operator()(const Bus * lhs, const Bus * rhs) const {
                return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(), rhs->name.begin(), rhs->name.end());
            }
        };

        void AddStop(const Stop &stop);

        void AddBus(const Bus &bus);

        [[nodiscard]] StopPtr GetStop(const std::string_view &name) const noexcept;

        [[nodiscard]] const Bus *GetBus(const std::string_view &name) const noexcept;

        [[nodiscard]] const std::set<const Bus *, BusPtrComparator> * GetBusesForStop(StopPtr p_stop) const noexcept;

        void SetDistance(StopPair from_to, distance_t distance);

        distance_t GetDistance(StopPair from_to) const noexcept;

        std::vector<StopPtr> GetAllStops() const noexcept;

        std::vector<const Bus *> GetAllBuses() const noexcept;

        bool IsStopInRoutes(StopPtr p_stop) const noexcept;

        size_t EvaluateVertexCount() const noexcept;

        void Serialize(pb3::TransportCatalogue &catalogue) const;

        void Deserialize(pb3::TransportCatalogue &catalogue);

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::map<std::string_view, StopPtr> stops_by_name_;
        std::map<std::string_view, const Bus *> buses_by_name_;
        std::unordered_map<StopPtr, std::set<const Bus *, BusPtrComparator>> buses_for_stop_;
        std::unordered_map<StopPair, distance_t, StopPairHasher> distances_;
    };

    namespace geo {

        double ComputeRouteGeoLength(const Bus *p_bus);

        distance_t ComputeRouteLength(const Bus *p_bus, const TransportCatalogue &catalogue);

    } //namespace transcat::geo

} //namespace transcat