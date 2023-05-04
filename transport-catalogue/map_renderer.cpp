#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 * 
 * Вершины каждой ломаной — это координаты соответствующих остановок. Выведите их в порядке следования от первой до первой остановки по кольцевому маршруту. 
 Для проецирования координат остановок на SVG-карту используйте класс SphereProjector. Количество вершин должно быть равно величине stop_count из ответа на запрос Bus. Если маршрут некольцевой, 
 то есть "is_roundtrip": false, каждый отрезок между соседними остановками должен быть нарисован дважды: сначала в прямом, а потом в обратном направлении.
 */


// определение методов корректировки координат

namespace renderer {
    const RenderSettings& MapRenderer::GetRendSet() const {
        return renderer_data_;
    }
    
    
    std::vector<svg::Polyline> MapRenderer::CreateBusLine(const std::map<std::string_view, std::vector<svg::Point>>& bus_route_points) const {
        using namespace svg;
        using namespace std::literals;
        std::vector<svg::Polyline> result;
        Polyline polyline;
        int color_index = 0;
        int max_color_id = renderer_data_.color_palette.size() - 1;
        for (const auto& [bus, route] : bus_route_points) {
            if (route.empty()) {
                continue;
            }
            else {
                for (const auto& point : route) {
                    polyline.AddPoint(point);
                }
                polyline.SetStrokeColor(renderer_data_.color_palette[color_index]).SetFillColor("none"s).SetStrokeWidth(renderer_data_.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            result.push_back(std::move(polyline));
            ++color_index;
            if (color_index > max_color_id) color_index = 0;
            }
        }
        return result;
    }
    

}// namespace renderer

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
