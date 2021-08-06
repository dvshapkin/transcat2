#include <fstream>

#include <map_renderer.pb.h>
#include "serialization.h"

trancat_proto::Color ColorToProto(const svg::Color &color) {
    trancat_proto::Color proto_underlayer_color;
    if (std::holds_alternative<std::string>(color)) {
        proto_underlayer_color.set_str_color(std::get<std::string>(color));
    } else if (std::holds_alternative<svg::Rgba>(color)) {
        trancat_proto::Rgba proto_rgba;
        auto rgba = std::get<svg::Rgba>(color);
        proto_rgba.set_r(rgba.red);
        proto_rgba.set_g(rgba.green);
        proto_rgba.set_b(rgba.blue);
        proto_rgba.set_a(rgba.opacity);
        *proto_underlayer_color.mutable_rgba() = std::move(proto_rgba);
    } else {
        trancat_proto::Rgb proto_rgb;
        auto rgb = std::get<svg::Rgb>(color);
        proto_rgb.set_r(rgb.red);
        proto_rgb.set_g(rgb.green);
        proto_rgb.set_b(rgb.blue);
        *proto_underlayer_color.mutable_rgb() = std::move(proto_rgb);
    }
    return proto_underlayer_color;
}

svg::Color ColorFromProto(const trancat_proto::Color &proto_color) {
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

void SerializeRenderSettings(std::ofstream &out, const renderer::RenderSettings& settings) {
    trancat_proto::RenderSettings proto_settings;
    proto_settings.set_width(settings.width);
    proto_settings.set_height(settings.height);
    proto_settings.set_padding(settings.padding);
    proto_settings.set_line_width(settings.line_width);
    proto_settings.set_stop_radius(settings.stop_radius);
    proto_settings.set_bus_label_font_size(settings.bus_label_font_size);
    proto_settings.set_bus_label_offset_x(settings.bus_label_offset[0]);
    proto_settings.set_bus_label_offset_y(settings.bus_label_offset[1]);
    proto_settings.set_stop_label_font_size(settings.stop_label_font_size);
    proto_settings.set_stop_label_offset_x(settings.stop_label_offset[0]);
    proto_settings.set_stop_label_offset_y(settings.stop_label_offset[1]);
    *proto_settings.mutable_underlayer_color() = std::move(ColorToProto(settings.underlayer_color));
    proto_settings.set_underlayer_width(settings.underlayer_width);
    for (const svg::Color& color: settings.color_palette) {
        proto_settings.mutable_color_palette()->Add(std::move(ColorToProto(color)));
    }
    proto_settings.SerializeToOstream(&out);
}

renderer::RenderSettings DeserializeRenderSettings(std::ifstream &in) {
    trancat_proto::RenderSettings proto_settings;
    proto_settings.ParseFromIstream(&in);
    renderer::RenderSettings settings;
    settings.width = proto_settings.width();
    settings.height = proto_settings.height();
    settings.padding = proto_settings.padding();
    settings.line_width = proto_settings.line_width();
    settings.stop_radius = proto_settings.stop_radius();
    settings.bus_label_font_size = static_cast<int>(proto_settings.bus_label_font_size());
    settings.bus_label_offset[0] = proto_settings.bus_label_offset_x();
    settings.bus_label_offset[1] = proto_settings.bus_label_offset_y();
    settings.stop_label_font_size = static_cast<int>(proto_settings.stop_label_font_size());
    settings.stop_label_offset[0] = proto_settings.stop_label_offset_x();
    settings.stop_label_offset[1] = proto_settings.stop_label_offset_y();
    settings.underlayer_color = ColorFromProto(proto_settings.underlayer_color());
    settings.underlayer_width = proto_settings.underlayer_width();
    for (const trancat_proto::Color& proto_color: proto_settings.color_palette()) {
        settings.color_palette.push_back(ColorFromProto(proto_color));
    }
    return settings;
}
