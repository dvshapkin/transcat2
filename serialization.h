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
                            const graph::DirectedWeightedGraph<double> &graph);

        void SerializeTo(const std::filesystem::path &path);

    private:
        void SerializeDb();

        void SerializeGraph();

        void SerializeRenderSettings();

        void SerializeRoutingSettings();

        static pb3::Color ColorToProto(const svg::Color &color);

    private:
        const TransportCatalogue &db_;
        const graph::DirectedWeightedGraph<double> &graph_;
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

        void DeserializeFrom(const std::filesystem::path &path);

    private:
        void DeserializeDb() const;

        void DeserializeGraph();

        void DeserializeRenderSettings();

        void DeserializeRoutingSettings();

        static svg::Color ColorFromProto(const pb3::Color &proto_color);

    private:
        TransportCatalogue &db_;
        graph::DirectedWeightedGraph<double> graph_;
        renderer::RenderSettings render_settings_;
        RoutingSettings routing_settings_;
        pb3::TransportCatalogue proto_db_;
    };

}
