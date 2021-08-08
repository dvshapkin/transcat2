#pragma once

#include <optional>
#include <vector>
#include <unordered_map>

#include "transport_catalogue.h"
#include "map_renderer.h"

#include "router.h"

namespace transcat {

    struct BusStat {
        double curvature = 0;
        distance_t route_length = 0;
        int stop_count = 0;
        int unique_stop_count = 0;
    };

    // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
    // с другими подсистемами приложения.
    // См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
    class RequestHandler {
    public:
        // MapRenderer понадобится в следующей части итогового проекта
        RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer, RoutingSettings settings,
                       size_t vertex_count);

        RequestHandler(const TransportCatalogue &db, const renderer::MapRenderer &renderer,
                                       RoutingSettings settings, size_t vertex_count,
                                       graph::DirectedWeightedGraph<double> route_graph);

        // Проверяет наличие остановки в БД
        bool IsStopExists(const std::string_view &stop_name) const;

        // Возвращает информацию о маршруте (запрос Bus)
        [[nodiscard]] std::optional<BusStat> GetBusStat(const std::string_view &bus_name) const;

        // Проверяет есть ли автобусы, проходящие через остановку
        bool IsStopInRoutes(const std::string_view &stop_name) const;

        // Возвращает маршруты, проходящие через
        [[nodiscard]] const std::set<const Bus *, TransportCatalogue::BusPtrComparator> *
        GetBusesByStop(const std::string_view &stop_name) const;

        // Этот метод будет нужен в следующей части итогового проекта
        [[nodiscard]] svg::Document RenderMap() const;

        [[nodiscard]] distance_t GetDistance(StopPair from_to) const noexcept;

        RoutingSettings GetRoutingSettings() const noexcept;

        [[nodiscard]] double GetBusVelocity() const noexcept;

        [[nodiscard]] double GetNormalBusVelocity() const noexcept;

        [[nodiscard]] const graph::DirectedWeightedGraph<double> &GetRouteGraph() const;

        graph::VertexId GetVertexForStop(StopPtr p_stop) const;

        StopPtr GetStopForVertex(graph::VertexId vertex_id) const;

        const Bus *GetBusByEdge(graph::EdgeId edge_id) const;

        void SetBusForEdge(graph::EdgeId edge_id, const Bus *p_bus) const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue &db_;
        const renderer::MapRenderer &renderer_;
        RoutingSettings settings_;
        graph::DirectedWeightedGraph<double> route_graph_;
        std::unordered_map<StopPtr, graph::VertexId> stops_to_vrtx_;    // собственные вершины остановки (wait): 1 <-> 1
        std::vector<StopPtr> vrtx_to_stops_;                            // вершины автобусов этой остановки: 1 <-> 1
    };

} // namespace transcat
