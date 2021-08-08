#pragma once

#include <iostream>
#include <variant>

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"


namespace transcat::query {

    enum class StatRequestType {
        Bus,
        Stop,
        Map,
        Route
    };

    StatRequestType StatRequestTypeFromString(const std::string &type_name);

    struct StatRequest {
        int id = 0;
        StatRequestType type;
        std::variant<std::nullopt_t, std::string, StopPair> data;
    };

    struct SerializationSettings {
        std::string file;
    };

    class JsonReader {
    public:
        JsonReader(TransportCatalogue &db, renderer::MapRenderer &renderer);

        void ReadData(const json::Document &document);

        void WriteInfo(std::ostream &out, const std::vector<query::StatRequest> &requests,
                       graph::DirectedWeightedGraph<double> route_graph,
                       graph::Router<double>::RoutesInternalData routes_internal_data) const;

        [[nodiscard]] std::vector<StatRequest> ParseStatRequests(const json::Document &document) const;

        static SerializationSettings ParseSerializationSettings(const json::Document &document);

        [[nodiscard]] const RoutingSettings &GetRoutingSettings() const;

        void SetRoutingSettings(const RoutingSettings &settings);

    private:
        void ParseBaseRequests(const json::Document &document) const;

        void ParseRenderSettings(const json::Document &document) const;

        void UpdateDistances(const std::unordered_map<StopPtr, json::Dict> &distances) const;

        void UpdateRoutes(const std::vector<json::Dict> &bus_queries) const;

        [[nodiscard]] svg::Color ParseColor(const json::Node &color) const;

        void ParseRoutingSettings(const json::Document &document);

        void WriteBusInfo(const RequestHandler &handler, json::Array &responses, const StatRequest &request) const;

        void WriteStopInfo(const RequestHandler &handler, json::Array &responses, const StatRequest &request) const;

        void WriteMapInfo(const RequestHandler &handler, json::Array &responses, const StatRequest &request) const;

        void WriteRouteInfo(const RequestHandler &handler, const graph::Router<double> &router, json::Array &responses,
                            const StatRequest &request) const;

        struct BusItem {
            std::string name;
            int span_count = 0;
            double weight = 0;
        };

        [[nodiscard]] json::Dict MakeBusItem(const std::string &bus_name, int span_count, double time) const;

        [[nodiscard]] json::Dict MakeWaitItem(const std::string &stop_name, int wait_time) const;

        void MakeRouteItems(const RequestHandler &handler, const graph::Router<double>::RouteInfo &route_info,
                            json::Array &items) const;

    private:
        TransportCatalogue &db_;
        renderer::MapRenderer &renderer_;
        RoutingSettings routing_settings_;
    };

} // namespace transcat::query
