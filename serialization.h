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
        CatalogueSerializer(TransportCatalogue &db, const renderer::RenderSettings &render_settings);

        void SerializeTo(const std::filesystem::path &path);

    private:
        void SerializeDb();

        void SerializeRenderSettings();

        static pb3::Color ColorToProto(const svg::Color &color);

    private:
        const TransportCatalogue &db_;
        const renderer::RenderSettings &render_settings_;
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

        void DeserializeFrom(const std::filesystem::path &path);

    private:
        void DeserializeDb() const;

        void DeserializeRenderSettings();

        static svg::Color ColorFromProto(const pb3::Color &proto_color);

    private:
        TransportCatalogue &db_;
        renderer::RenderSettings render_settings_;
        pb3::TransportCatalogue proto_db_;
    };

}
