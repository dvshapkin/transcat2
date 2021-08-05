#pragma once

#include <cstdint>
#include <limits>

#include "svg.h"
#include "geo.h"
#include "domain.h"


namespace transcat::renderer {

    struct RenderSettings {
        double width = 0;
        double height = 0;

        double padding = 0;

        double line_width = 0;
        double stop_radius = 0;

        int bus_label_font_size = 0;
        double bus_label_offset[2] = {0, 0};

        int stop_label_font_size = 0;
        double stop_label_offset[2] = {0, 0};

        svg::Color underlayer_color = svg::NoneColor;
        double underlayer_width = 0;

        std::vector<svg::Color> color_palette;
    };

    class MapRenderer {
    public:
        explicit MapRenderer() = default;

        void UseSettings(RenderSettings settings);

        [[nodiscard]] svg::Document Render(const std::vector<StopPtr> &stops, const std::vector<const Bus *> &buses) const;

    private:
        void InitMap(const std::vector<StopPtr> &stops) const;

        void RenderRoutes(const std::vector<const Bus *> &buses, svg::Document &doc,
                          const std::vector<svg::Color> &palette) const;

        void RenderRoutesName(const std::vector<const Bus *> &buses, svg::Document &doc,
                              const std::vector<svg::Color> &palette) const;

        void RenderStops(const std::vector<StopPtr> &stops, svg::Document &doc) const;

        void RenderStopsName(const std::vector<StopPtr> &stops, svg::Document &doc) const;

        double GetZoomCoef() const noexcept;

        svg::Point GetPoint(double lon, double lat) const noexcept;

        svg::Text
        GetBusLabel(const Bus *p_bus, StopPtr p_stop, bool is_background , svg::Color fill_color = svg::NoneColor) const noexcept;

        svg::Text
        GetStopLabel(StopPtr p_stop, bool is_background = false) const noexcept;

        RenderSettings settings_;
        mutable double min_lat_ = std::numeric_limits<double>::max();
        mutable double min_lon_ = std::numeric_limits<double>::max();
        mutable double max_lat_ = std::numeric_limits<double>::min();
        mutable double max_lon_ = std::numeric_limits<double>::min();
        mutable double zoom_coef_ = 0;
    };

} // namespace transcat::renderer