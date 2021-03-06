#pragma once

#include <filesystem>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include <transport_catalogue.pb.h>

namespace transcat {

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    //   Catalog Serializer
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////

    class CatalogueSerializer {
    public:
        CatalogueSerializer(const TransportCatalogue &db,
                            const renderer::RenderSettings &render_settings,
                            const RoutingSettings &routing_settings,
                            const graph::DirectedWeightedGraph<double> &graph,
                            const graph::Router<double>::RoutesInternalData &routes_internal_data);

        void SerializeTo(const std::filesystem::path &path);

    private:
        void SerializeDb();

        void SerializeGraph();

        void SerializeRoutesInternalData();

        void SerializeRenderSettings();

        void SerializeRoutingSettings();

        static pb3::Color ColorToProto(const svg::Color &color);

        static pb3::Stop StopToProto(const Stop *p_stop);

        static pb3::Bus BusToProto(const Bus *p_bus, const std::map<const Stop*, size_t> &stops_id);

    private:
        const TransportCatalogue &db_;
        const graph::DirectedWeightedGraph<double> &graph_;
        const graph::Router<double>::RoutesInternalData &routes_internal_data_;
        const renderer::RenderSettings &render_settings_;
        const RoutingSettings &routing_settings_;
        pb3::TransportCatalogue proto_db_;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    //   Catalog Deserializer
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////

    class CatalogueDeserializer {
    public:
        explicit CatalogueDeserializer(TransportCatalogue &db);

        renderer::RenderSettings GetRenderSettings() const;

        RoutingSettings GetRoutingSettings() const;

        graph::DirectedWeightedGraph<double> GetRouteGraph() const;

        graph::Router<double>::RoutesInternalData GetRoutesInternalData() const;

        void DeserializeFrom(const std::filesystem::path &path);

    private:
        void DeserializeDb() const;

        void DeserializeGraph();

        void DeserializeRoutesInternalData();

        void DeserializeRenderSettings();

        void DeserializeRoutingSettings();

        static svg::Color ColorFromProto(const pb3::Color &proto_color);

        static Stop StopFromProto(const pb3::Stop &proto_stop);

        static Bus BusFromProto(const pb3::Bus &proto_bus, const std::deque<Stop> &stops);

    private:
        TransportCatalogue &db_;
        graph::DirectedWeightedGraph<double> graph_;
        graph::Router<double>::RoutesInternalData routes_internal_data_;
        renderer::RenderSettings render_settings_;
        RoutingSettings routing_settings_;
        pb3::TransportCatalogue proto_db_;
    };

}
