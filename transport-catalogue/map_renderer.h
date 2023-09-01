#pragma once
#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"


#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>
#include <utility>
#include <deque>

namespace transport_base_processing {
    struct RenderSettings {
        double width = 0.0; 
        double height = 0.0; 
        double padding = 0.0; 
        double line_width = 0.0; 
        double stop_radius = 0.0; 
        unsigned long int bus_label_font_size = 0; 
        svg::Point bus_label_offset = { 0.0, 0.0 }; 
        unsigned long int stop_label_font_size = 0; 
        svg::Point stop_label_offset = { 0.0, 0.0 }; 
        svg::Color underlayer_color;
        double underlayer_width = 0.0; 
        std::vector<svg::Color>  color_palette; 
    };
    
    class MapRenderer {
    public:
        
        MapRenderer() = default;

        MapRenderer(const RenderSettings& renderer_data) {
            SetRendSet(renderer_data);
        }

        MapRenderer(RenderSettings&& renderer_data) {
            SetRendSet(std::move(renderer_data));
        }

        void SetRendSet(const RenderSettings& renderer_data);

        std::vector<svg::Polyline> CreateBusLine(const std::map<std::string_view, std::vector<svg::Point>>& bus_route_points) const;

        std::vector<svg::Text> CreateRouteNames(const std::map<std::string_view, std::vector<svg::Point>>& bus_route_points, const transport_base_processing::TransportCatalogue& db) const;

        std::vector<svg::Circle> CreateStops(const std::map<std::string_view, svg::Point>& stops_on_routes) const;

        std::vector<svg::Text> CreateStopsNames(const std::map<std::string_view, svg::Point>& stops_on_routes) const;
                      
        const RenderSettings& GetRendSet() const;
                
    private:
        RenderSettings renderer_data_;
};
} // namespace transport_base_processing

bool IsZero(double value);

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        if (points_begin == points_end) {
            return;
        }

        const std::pair min_max = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = min_max.first->lng;
        const double max_lon = min_max.second->lng;

        const std::pair bottom_top = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_top.first->lat;
        max_lat_ = bottom_top.second->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};