#include <sstream>
#include <algorithm>

#include "map_renderer.h"

namespace transcat::renderer {

    void MapRenderer::UseSettings(RenderSettings settings) {
        settings_ = std::move(settings);
    }

    const RenderSettings& MapRenderer::GetSettings() const {
        return settings_;
    }

    svg::Document MapRenderer::Render(const std::vector<StopPtr> &stops, const std::vector<const Bus *> &buses) const {

        // Расчет размеров карты и коэффициента масштабирования
        InitMap(stops);

        svg::Document doc;

        auto palette = settings_.color_palette;

        // отрисовка линий маршрутов
        RenderRoutes(buses, doc, palette);

        // отрисовка названий маршрутов
        RenderRoutesName(buses, doc, palette);

        // отрисовка остановок
        RenderStops(stops, doc);

        // отрисовка названий остановок
        RenderStopsName(stops, doc);

        return doc;
    }

    void MapRenderer::RenderStopsName(const std::vector<StopPtr> &stops, svg::Document &doc) const {
        for (StopPtr p_stop: stops) {
            // подложка
            svg::Text route_name_bg = GetStopLabel(p_stop, true);
            doc.Add(route_name_bg);
            // надпись
            svg::Text route_name = GetStopLabel(p_stop);
            doc.Add(route_name);
        }
    }

    void MapRenderer::RenderStops(const std::vector<StopPtr> &stops, svg::Document &doc) const {
        for (StopPtr p_stop: stops) {
            svg::Circle stop_circle;
            stop_circle.SetCenter(GetPoint(p_stop->longitude, p_stop->latitude));
            stop_circle.SetRadius(settings_.stop_radius);
            stop_circle.SetFillColor("white");
            doc.Add(stop_circle);
        }
    }

    void MapRenderer::RenderRoutesName(const std::vector<const Bus *> &buses, svg::Document &doc,
                                       const std::vector<svg::Color> &palette) const {
        int bus_counter = -1;
        for (const Bus *p_bus: buses) {
            const auto &route = p_bus->route;
            if (route.empty()) continue;

            auto fill_color = palette[++bus_counter % palette.size()];

            // подложка
            svg::Text route_name_bg = GetBusLabel(p_bus, p_bus->start_stop, true);
            doc.Add(route_name_bg);
            // надпись
            svg::Text route_name = GetBusLabel(p_bus, p_bus->start_stop, false, fill_color);
            doc.Add(route_name);

            if (!p_bus->is_roundtrip && p_bus->start_stop != p_bus->end_stop) {
                // подложка
                svg::Text route_name_bg = GetBusLabel(p_bus, p_bus->end_stop, true);
                doc.Add(route_name_bg);

                // надпись
                svg::Text route_name = GetBusLabel(p_bus, p_bus->end_stop, false, fill_color);
                doc.Add(route_name);
            }
        }
    }

    void MapRenderer::InitMap(const std::vector<StopPtr> &stops) const {
        min_lat_ = std::numeric_limits<double>::max();
        min_lon_ = std::numeric_limits<double>::max();
        max_lat_ = std::numeric_limits<double>::min();
        max_lon_ = std::numeric_limits<double>::min();
        for (StopPtr p_stop: stops) {
            if (p_stop->latitude < min_lat_) min_lat_ = p_stop->latitude;
            if (p_stop->longitude < min_lon_) min_lon_ = p_stop->longitude;
            if (p_stop->latitude > max_lat_) max_lat_ = p_stop->latitude;
            if (p_stop->longitude > max_lon_) max_lon_ = p_stop->longitude;
        }
        zoom_coef_ = GetZoomCoef();
    }

    void MapRenderer::RenderRoutes(const std::vector<const Bus *> &buses, svg::Document &doc,
                                   const std::vector<svg::Color> &palette) const {
        int bus_counter = -1;
        for (const Bus *p_bus: buses) {
            if (p_bus->route.empty()) continue;

            auto fill_color = palette[++bus_counter % palette.size()];

            svg::Polyline route;
            route.SetStrokeWidth(settings_.line_width);
            route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            route.SetFillColor(svg::NoneColor);
            route.SetStrokeColor(fill_color);
            for (StopPtr p_stop: p_bus->route) {
                route.AddPoint(GetPoint(p_stop->longitude, p_stop->latitude));
            }
            doc.Add(route);
        }
    }

    svg::Text
    MapRenderer::GetBusLabel(const Bus *p_bus, StopPtr p_stop, bool is_background, svg::Color fill_color) const noexcept {
        svg::Text label;
        label.SetData(p_bus->name);
        label.SetPosition(GetPoint(p_stop->longitude, p_stop->latitude));
        label.SetOffset(svg::Point(settings_.bus_label_offset[0], settings_.bus_label_offset[1]));
        label.SetFontSize(settings_.bus_label_font_size);
        label.SetFontFamily("Verdana");
        label.SetFontWeight("bold");
        if (!is_background) {
            label.SetFillColor(fill_color);
        } else {
            label.SetFillColor(settings_.underlayer_color);
            label.SetStrokeColor(settings_.underlayer_color);
            label.SetStrokeWidth(settings_.underlayer_width);
            label.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            label.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        }
        return label;
    }

    svg::Text
    MapRenderer::GetStopLabel(StopPtr p_stop, bool is_background) const noexcept {
        svg::Text label;
        label.SetData(p_stop->name);
        label.SetPosition(GetPoint(p_stop->longitude, p_stop->latitude));
        label.SetOffset(svg::Point(settings_.stop_label_offset[0], settings_.stop_label_offset[1]));
        label.SetFontSize(settings_.stop_label_font_size);
        label.SetFontFamily("Verdana");
        if (!is_background) {
            label.SetFillColor("black");
        } else {
            label.SetFillColor(settings_.underlayer_color);
            label.SetStrokeColor(settings_.underlayer_color);
            label.SetStrokeWidth(settings_.underlayer_width);
            label.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            label.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        }
        return label;
    }

    bool IsZero(double value) {
        return std::abs(value) < std::numeric_limits<double>::epsilon();
    }

    double MapRenderer::GetZoomCoef() const noexcept {
        std::optional<double> width_zoom_coef;
        if (!IsZero(max_lon_ - min_lon_)) {
            width_zoom_coef = (settings_.width - 2 * settings_.padding) / (max_lon_ - min_lon_);
        }
        std::optional<double> height_zoom_coef;
        if (!IsZero(max_lat_ - min_lat_)) {
            height_zoom_coef = (settings_.height - 2 * settings_.padding) / (max_lat_ - min_lat_);
        }
        if (width_zoom_coef && height_zoom_coef) {
            return std::min(*width_zoom_coef, *height_zoom_coef);
        } else if (width_zoom_coef) {
            return *width_zoom_coef;
        } else if (height_zoom_coef) {
            return *height_zoom_coef;
        }
        return 0;
    }

    svg::Point
    MapRenderer::GetPoint(double lon, double lat) const noexcept {
        return svg::Point(
                (lon - min_lon_) * zoom_coef_ + settings_.padding,
                (max_lat_ - lat) * zoom_coef_ + settings_.padding
        );
    }

} // namespace transcat::renderer