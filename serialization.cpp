#include <fstream>

#include "serialization.h"

namespace transcat {

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    //   Catalog Serializer
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////

    CatalogueSerializer::CatalogueSerializer(TransportCatalogue &db, const renderer::RenderSettings &render_settings,
                                             const RoutingSettings &routing_settings)
            : db_(db), render_settings_(render_settings), routing_settings_(routing_settings) {
    }

    void CatalogueSerializer::SerializeTo(const std::filesystem::path &path) {
        SerializeDb();
        SerializeRenderSettings();
        SerializeRoutingSettings();
        std::ofstream out_file(path, std::ios::binary);
        proto_db_.SerializeToOstream(&out_file);
        out_file.close();
    }

    void CatalogueSerializer::SerializeDb() {
        // выгрузим stops_
        for (const Stop &stop: db_.stops_) {
            proto_db_.mutable_stops()->Add(stop.ToProto());
        }
        // выгрузим buses_
        for (const Bus &bus: db_.buses_) {
            proto_db_.mutable_buses()->Add(bus.ToProto());
        }
        // выгрузим distances_
        for (const auto &[from_to, distance]: db_.distances_) {
            pb3::Distance proto_distance;
            *proto_distance.mutable_from() = from_to.from->ToProto();
            *proto_distance.mutable_to() = from_to.to->ToProto();
            proto_distance.set_distance(distance);
            proto_db_.mutable_distances()->Add(std::move(proto_distance));
        }
    }

    void CatalogueSerializer::SerializeRenderSettings() {
        pb3::RenderSettings *proto_settings = proto_db_.mutable_render_settings();
        proto_settings->set_width(render_settings_.width);
        proto_settings->set_height(render_settings_.height);
        proto_settings->set_padding(render_settings_.padding);
        proto_settings->set_line_width(render_settings_.line_width);
        proto_settings->set_stop_radius(render_settings_.stop_radius);
        proto_settings->set_bus_label_font_size(render_settings_.bus_label_font_size);
        proto_settings->set_bus_label_offset_x(render_settings_.bus_label_offset[0]);
        proto_settings->set_bus_label_offset_y(render_settings_.bus_label_offset[1]);
        proto_settings->set_stop_label_font_size(render_settings_.stop_label_font_size);
        proto_settings->set_stop_label_offset_x(render_settings_.stop_label_offset[0]);
        proto_settings->set_stop_label_offset_y(render_settings_.stop_label_offset[1]);
        *proto_settings->mutable_underlayer_color() = ColorToProto(render_settings_.underlayer_color);
        proto_settings->set_underlayer_width(render_settings_.underlayer_width);
        for (const svg::Color &color: render_settings_.color_palette) {
            proto_settings->mutable_color_palette()->Add(ColorToProto(color));
        }
    }

    void CatalogueSerializer::SerializeRoutingSettings() {
        pb3::RoutingSettings *proto_settings = proto_db_.mutable_routing_settings();
        proto_settings->set_bus_wait_time(routing_settings_.bus_wait_time);
        proto_settings->set_bus_velocity(routing_settings_.bus_velocity);
    }

    pb3::Color CatalogueSerializer::ColorToProto(const svg::Color &color) {
        pb3::Color proto_color;
        if (std::holds_alternative<std::string>(color)) {
            proto_color.set_str_color(std::get<std::string>(color));
        } else if (std::holds_alternative<svg::Rgba>(color)) {
            pb3::Rgba proto_rgba;
            auto rgba = std::get<svg::Rgba>(color);
            proto_rgba.set_r(rgba.red);
            proto_rgba.set_g(rgba.green);
            proto_rgba.set_b(rgba.blue);
            proto_rgba.set_a(rgba.opacity);
            *proto_color.mutable_rgba() = std::move(proto_rgba);
        } else if (std::holds_alternative<svg::Rgb>(color)) {
            pb3::Rgb proto_rgb;
            auto rgb = std::get<svg::Rgb>(color);
            proto_rgb.set_r(rgb.red);
            proto_rgb.set_g(rgb.green);
            proto_rgb.set_b(rgb.blue);
            *proto_color.mutable_rgb() = std::move(proto_rgb);
        } else {
            proto_color.set_str_color(std::get<std::string>(svg::NoneColor));
        }
        return proto_color;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    //   Catalog Deserializer
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////

    CatalogueDeserializer::CatalogueDeserializer(TransportCatalogue &db)
            : db_(db) {
    }

    renderer::RenderSettings CatalogueDeserializer::GetRenderSettings() const {
        return render_settings_;
    }

    RoutingSettings CatalogueDeserializer::GetRoutingSettings() const {
        return routing_settings_;
    }

    void CatalogueDeserializer::DeserializeFrom(const std::filesystem::path &path) {
        std::ifstream in_file(path, std::ios::binary);
        proto_db_.ParseFromIstream(&in_file);
        DeserializeDb();
        DeserializeRenderSettings();
        DeserializeRoutingSettings();
        in_file.close();
    }

    void CatalogueDeserializer::DeserializeDb() const {
        // заполним stops_ и stops_by_name_
        for (const auto &proto_stop: proto_db_.stops()) {
            auto &ref_stop = db_.stops_.emplace_back(Stop::FromProto(proto_stop));
            db_.stops_by_name_[ref_stop.name] = &ref_stop;
        }
        // заполним buses_ и buses_by_name_
        for (const auto &proto_bus: proto_db_.buses()) {
            auto &ref_bus = db_.buses_.emplace_back(Bus::FromProto(proto_bus, db_.stops_by_name_));
            db_.buses_by_name_[ref_bus.name] = &ref_bus;
            for (StopPtr p_stop: ref_bus.route) {
                db_.buses_for_stop_[p_stop].insert(&ref_bus);
            }
        }
        // заполним distances_
        for (const auto &proto_distance: proto_db_.distances()) {
            StopPair from_to{
                    db_.stops_by_name_.at(proto_distance.from().name()),
                    db_.stops_by_name_.at(proto_distance.to().name())
            };
            db_.distances_[from_to] = static_cast<distance_t>(proto_distance.distance());
        }
    }

    void CatalogueDeserializer::DeserializeRenderSettings() {
        const pb3::RenderSettings &proto_settings = proto_db_.render_settings();
        render_settings_.width = proto_settings.width();
        render_settings_.height = proto_settings.height();
        render_settings_.padding = proto_settings.padding();
        render_settings_.line_width = proto_settings.line_width();
        render_settings_.stop_radius = proto_settings.stop_radius();
        render_settings_.bus_label_font_size = static_cast<int>(proto_settings.bus_label_font_size());
        render_settings_.bus_label_offset[0] = proto_settings.bus_label_offset_x();
        render_settings_.bus_label_offset[1] = proto_settings.bus_label_offset_y();
        render_settings_.stop_label_font_size = static_cast<int>(proto_settings.stop_label_font_size());
        render_settings_.stop_label_offset[0] = proto_settings.stop_label_offset_x();
        render_settings_.stop_label_offset[1] = proto_settings.stop_label_offset_y();
        render_settings_.underlayer_color = ColorFromProto(proto_settings.underlayer_color());
        render_settings_.underlayer_width = proto_settings.underlayer_width();
        for (const pb3::Color &proto_color: proto_settings.color_palette()) {
            render_settings_.color_palette.push_back(ColorFromProto(proto_color));
        }
    }

    void CatalogueDeserializer::DeserializeRoutingSettings() {
        const pb3::RoutingSettings &proto_settings = proto_db_.routing_settings();
        routing_settings_.bus_wait_time = proto_settings.bus_wait_time();
        routing_settings_.bus_velocity = proto_settings.bus_velocity();
    }

    svg::Color CatalogueDeserializer::ColorFromProto(const pb3::Color &proto_color) {
        if (proto_color.has_rgba()) {
            svg::Rgba rgba;
            rgba.red = proto_color.rgba().r();
            rgba.green = proto_color.rgba().g();
            rgba.blue = proto_color.rgba().b();
            rgba.opacity = proto_color.rgba().a();
            return rgba;
        } else if (proto_color.has_rgb()) {
            svg::Rgb rgb;
            rgb.red = proto_color.rgb().r();
            rgb.green = proto_color.rgb().g();
            rgb.blue = proto_color.rgb().b();
            return rgb;
        } else {
            return proto_color.str_color();
        }
    }
}
