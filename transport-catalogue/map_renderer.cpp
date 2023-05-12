#include "map_renderer.h"


namespace transport_base_processing {
    const RenderSettings& MapRenderer::GetRendSet() const {
        return renderer_data_;
    }
    
    void MapRenderer::SetRendSet(const RenderSettings& renderer_data) {
        renderer_data_ = renderer_data;
    }
    
    std::vector<svg::Polyline> MapRenderer::CreateBusLine(const std::map<std::string_view, std::vector<svg::Point>>& bus_route_points) const {
        using namespace svg;
        using namespace std::literals;
        std::vector<svg::Polyline> result;
        result.reserve(bus_route_points.size());
        size_t color_index = 0;
        size_t max_color_id = renderer_data_.color_palette.size() - 1;
        for (const auto& [bus, route] : bus_route_points) {
            if (route.empty()) {
                continue;
            }
            else {
                Polyline polyline;
                for (const auto& point : route) {
                    polyline.AddPoint(point);
                }
                
        polyline.SetStrokeColor(renderer_data_.color_palette[color_index]).
            SetStrokeColor(renderer_data_.color_palette[color_index]).
            SetStrokeColor(renderer_data_.color_palette[color_index]).
            SetFillColor("none"s).
            SetStrokeWidth(renderer_data_.line_width).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            result.push_back(std::move(polyline));
            ++color_index;
            if (color_index > max_color_id) color_index = 0;
            }
        }
        return result;
    }

    std::vector<svg::Text> MapRenderer::CreateRouteNames(const std::map<std::string_view, std::vector<svg::Point>>& bus_route_points, const transport_base_processing::TransportCatalogue& db) const {
        using namespace std::literals;
        using namespace svg;
        std::vector<Text> result;
        result.reserve(bus_route_points.size() * 2);
        size_t color_index = 0;
        size_t max_color_id = renderer_data_.color_palette.size() - 1;
        for (const auto& [bus, route] : bus_route_points) {
            if (route.empty()) {
                continue;
            }
            else {
                Text text;
                Text background;
                text.SetPosition(route[0]).
                    SetOffset(renderer_data_.bus_label_offset).
                    SetFontSize(renderer_data_.bus_label_font_size).
                    SetFontFamily("Verdana"s).
                    SetFontWeight("bold"s).SetData(static_cast<std::string>(bus)).SetFillColor(renderer_data_.color_palette[color_index]);
                background = text;
                result.push_back(background.SetFillColor(renderer_data_.underlayer_color).
                    SetStrokeWidth(renderer_data_.underlayer_width).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                    SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeColor(renderer_data_.underlayer_color));
                result.push_back(text);
                if(db.FindBus(bus) -> is_circle == false && *route.begin() != *next(route.begin(), +route.size() / 2)) {
                    background.SetPosition(route[route.size() / 2]);
                    result.push_back(background);
                    text.SetPosition(route[route.size() / 2]);
                    result.push_back(text);
                }
                ++color_index;
                if (color_index > max_color_id) color_index = 0;
            }
           
        }
        return result;
    }
    
    std::vector<svg::Circle> MapRenderer::CreateStops(const std::map<std::string_view, svg::Point>& stops_on_routes) const {
        using namespace svg;
        using namespace std::literals;
        std::vector<svg::Circle> result;
        result.reserve(stops_on_routes.size());
        for (const auto& [stop, point] : stops_on_routes) {
            Circle circle;
            circle.SetCenter(point).SetRadius(renderer_data_.stop_radius).SetFillColor("white"s);
            result.push_back(circle);
        }
        return result;
    }

    std::vector<svg::Text> MapRenderer::CreateStopsNames(const std::map<std::string_view, svg::Point>& stops_on_routes) const {
        using namespace svg;
        using namespace std::literals;
        std::vector<svg::Text> result;
        result.reserve(stops_on_routes.size());
        for (const auto& [stop, point] : stops_on_routes) {
            Text text;
            Text background;
            background.SetFillColor("black"s).
                SetPosition(point).
                SetOffset(renderer_data_.stop_label_offset).
                SetFontSize(renderer_data_.stop_label_font_size).
                SetFontFamily("Verdana"s).SetData(static_cast<std::string>(stop));
            text = background;
            background.SetFillColor(renderer_data_.underlayer_color).
                SetStrokeColor(renderer_data_.underlayer_color).
                SetStrokeWidth(renderer_data_.underlayer_width).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            result.push_back(background);
            result.push_back(text);
        }
        return result;
    }
    

}// namespace transport_base_processing

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}


svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}
