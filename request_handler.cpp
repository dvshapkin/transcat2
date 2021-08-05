#include "request_handler.h"

namespace transcat {

    RequestHandler::RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer,
                                   RoutingSettings settings, size_t vertex_count)
            : db_(db), renderer_(renderer), settings_(settings), route_graph_(vertex_count),
              stops_to_vrtx_(vertex_count) {

        vrtx_to_stops_.resize(vertex_count);

        // индексация остановок
        graph::VertexId vrtx = 0;
        for (const auto p_stop: db_.GetAllStops()) {
            stops_to_vrtx_.emplace(p_stop, vrtx);
            vrtx_to_stops_[vrtx] = p_stop;
            ++vrtx;
        }

        const double normal_velocity = GetNormalBusVelocity();  // переводим скорость из км/ч -> м/мин

        // заполнение графа маршрутов
        for (const Bus *p_bus: db_.GetAllBuses()) {

            for (auto from_it = p_bus->route.begin(); from_it != p_bus->route.end(); ++from_it) {
                auto current_it = from_it;
                double weight = 0;
                int span_count = 0;
                for (auto to_it = next(from_it, 1); to_it != p_bus->route.end(); ++to_it, ++current_it) {
                    if (from_it != to_it) {
                        weight += db_.GetDistance({*current_it, *to_it}) / normal_velocity;
                        route_graph_.AddEdge({
                                                     GetVertexForStop(*from_it),
                                                     GetVertexForStop(*to_it),
                                                     weight + settings_.bus_wait_time,
                                                     p_bus,
                                                     ++span_count
                                             });
                    }
                    //if (*to_it == p_bus->end_stop && !p_bus->is_roundtrip) {
                    if (!p_bus->is_roundtrip && to_it == next(p_bus->route.begin(), p_bus->route.size()/2)) {
                        break;
                    }
                }
            }
        }
    }

    std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view &bus_name) const {
        const Bus *p_bus = db_.GetBus(bus_name);
        if (p_bus) {
            distance_t length = geo::ComputeRouteLength(p_bus, db_);
            double geo_length = geo::ComputeRouteGeoLength(p_bus);
            BusStat stat{
                    length / geo_length,
                    length,
                    static_cast<int>(p_bus->route.size()),
                    static_cast<int>(p_bus->unique_stops)
            };
            return stat;
        }
        return std::nullopt;
    }

    const std::set<const Bus *, TransportCatalogue::BusPtrComparator> *
    RequestHandler::GetBusesByStop(const std::string_view &stop_name) const {
        return db_.GetBusesForStop(db_.GetStop(stop_name));
    }

    svg::Document RequestHandler::RenderMap() const {
        std::vector<StopPtr> routed_stops;
        for (StopPtr p_stop: db_.GetAllStops()) {
            if (db_.IsStopInRoutes(p_stop)) {
                routed_stops.push_back(p_stop);
            }
        }
        std::vector<const Bus *> routes = db_.GetAllBuses();

        return renderer_.Render(routed_stops, routes);
    }

    bool RequestHandler::IsStopExists(const std::string_view &stop_name) const {
        return db_.GetStop(stop_name) != nullptr;
    }

    bool RequestHandler::IsStopInRoutes(const std::string_view &stop_name) const {
        return db_.IsStopInRoutes(db_.GetStop(stop_name));
    }

    distance_t RequestHandler::GetDistance(StopPair from_to) const noexcept {
        return db_.GetDistance(from_to);
    }

    RoutingSettings RequestHandler::GetRoutingSettings() const noexcept {
        return settings_;
    }

    double RequestHandler::GetBusVelocity() const noexcept {
        return settings_.bus_velocity;
    }

    double RequestHandler::GetNormalBusVelocity() const noexcept {
        return (settings_.bus_velocity * 1'000) / 60;
    }

    const graph::DirectedWeightedGraph<double> &RequestHandler::GetRouteGraph() const {
        return route_graph_;
    }

    graph::VertexId RequestHandler::GetVertexForStop(StopPtr p_stop) const {
        return stops_to_vrtx_.at(p_stop);
    }

    StopPtr RequestHandler::GetStopForVertex(graph::VertexId vertex_id) const {
        return vrtx_to_stops_.at(vertex_id);
    }

} // namespace transcat
