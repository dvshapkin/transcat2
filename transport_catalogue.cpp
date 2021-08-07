#include <cassert>
#include <algorithm>
#include <fstream>

#include "transport_catalogue.h"
#include "geo.h"

namespace transcat {

    void TransportCatalogue::AddStop(const Stop &stop) {
        if (stops_by_name_.count(stop.name) == 0) {
            stops_.emplace_back(stop);
            Stop *p_stop = &stops_.back();
            stops_by_name_[p_stop->name] = p_stop;

            buses_for_stop_[p_stop] = {};
        }
    }

    void TransportCatalogue::AddBus(const Bus &bus) {
        if (buses_by_name_.count(bus.name) == 0) {
            buses_.emplace_back(bus);
            const Bus *p_bus = &buses_.back();
            buses_by_name_[p_bus->name] = p_bus;

            for (StopPtr p_stop: bus.route) {
                buses_for_stop_[p_stop].insert(p_bus);
            }
        }
    }

    StopPtr TransportCatalogue::GetStop(const std::string_view &name) const noexcept {
        return stops_by_name_.count(name) ? stops_by_name_.at(name) : nullptr;
    }

    const Bus *TransportCatalogue::GetBus(const std::string_view &name) const noexcept {
        return buses_by_name_.count(name) ? buses_by_name_.at(name) : nullptr;
    }

    const std::set<const Bus *, TransportCatalogue::BusPtrComparator> *
    TransportCatalogue::GetBusesForStop(StopPtr p_stop) const noexcept {
        if (p_stop && buses_for_stop_.count(p_stop)) {
            return &buses_for_stop_.at(p_stop);
        }
        return nullptr;
    }

    void TransportCatalogue::SetDistance(StopPair from_to, distance_t distance) {
        distances_.emplace(from_to, distance);
    }

    distance_t TransportCatalogue::GetDistance(StopPair from_to) const noexcept {
        if (distances_.count(from_to)) {
            return distances_.at(from_to);
        } else if (distances_.count(from_to.Reverse())) {
            return distances_.at(from_to.Reverse());
        }
        return 0;
        //return -1;
    }

    std::vector<StopPtr> TransportCatalogue::GetAllStops() const noexcept {
        std::vector<StopPtr> result;
        result.reserve(stops_by_name_.size());
        for (const auto[name, p_stop]: stops_by_name_) {
            result.push_back(p_stop);
        }
        return result;
    }

    std::vector<const Bus *> TransportCatalogue::GetAllBuses() const noexcept {
        std::vector<const Bus *> result;
        result.reserve(buses_by_name_.size());
        for (const auto[name, p_bus]: buses_by_name_) {
            result.push_back(p_bus);
        }
        return result;
    }

    bool TransportCatalogue::IsStopInRoutes(StopPtr p_stop) const noexcept {
        return buses_for_stop_.count(p_stop) && !buses_for_stop_.at(p_stop).empty();
    }

    size_t TransportCatalogue::EvaluateVertexCount() const noexcept {
        return stops_.size();
    }

    namespace geo {

        double ComputeRouteGeoLength(const Bus *p_bus) {
            const Route &route = p_bus->route;
            double length = 0;
            for (auto it = route.cbegin(); it != route.cend(); ++it) {
                auto next_it = std::next(it, 1);
                if (next_it != route.cend()) {
                    length += ComputeDistance({(*it)->latitude, (*it)->longitude},
                                              {(*next_it)->latitude, (*next_it)->longitude});
                }
            }
            return length;
        }

        distance_t ComputeRouteLength(const Bus *p_bus, const TransportCatalogue &catalogue) {
            const Route &route = p_bus->route;
            distance_t length = 0;
            for (auto it = route.cbegin(); it != route.cend(); ++it) {
                auto next_it = std::next(it, 1);
                if (next_it != route.cend()) {
                    length += catalogue.GetDistance({*it, *next_it});
                }
            }
            return length;
        }

    } //namespace transcat::geo

} //namespace transcat
