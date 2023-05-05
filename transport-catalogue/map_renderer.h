#pragma once
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>
#include <utility>

namespace renderer {
    struct RenderSettings {
        double width = 0.0; // ширина холста от 0 до 100000
        double height = 0.0; // высота холста от 0 до 100000
        double padding = 0.0; // отступ от краев от 0 до  min(width, height) / 2
        double line_width = 0.0; // ширина линии от 0 до 100000
        double stop_radius = 0.0; // радиус кружка Stop от 0 до 100000
        unsigned long int bus_label_font_size = 0; //  размер текста, которым написаны названия автобусных маршрутов
        std::pair<double, double> bus_label_offset = { 0.0, 0.0 }; // Задаёт значения свойств dx и dy SVG - элемента <text>  от –100000 до 100000
        unsigned long int stop_label_font_size = 0; // размер текста, которым отображаются названия остановок от 0 до 100000
        std::pair<double, double> stop_label_offset = { 0.0, 0.0 }; // смещение названия остановки относительно её координат от –100000 до 100000
        svg::Color underlayer_color;
        double underlayer_width = 0.0; // толщина подложки под названиями остановок и маршрутов адаёт значение атрибута stroke - width элемента <text> от 0 до 100000 
        std::vector<svg::Color>  color_palette; // цветовая палитра
    };
    
    class MapRenderer {
    public:
        
        MapRenderer() = default;

        void SetRendSet(const RenderSettings& renderer_data);

        std::vector<svg::Polyline> CreateBusLine(const std::map<std::string_view, std::vector<svg::Point>>& bus_route_points) const;
              
        // void Draw(svg::ObjectContainer& container) const override;

        const RenderSettings& GetRendSet() const;

        
    private:
        RenderSettings renderer_data_;
};
} // namespace render

template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
    for (auto it = begin; it != end; ++it) {
        (*it)->Draw(target);
    }
}

template <typename Container>
void DrawPicture(const Container& container, svg::ObjectContainer& target) {
    using namespace std;
    DrawPicture(begin(container), end(container), target);
}



bool IsZero(double value);

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const std::pair min_max = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = min_max.first->lng;
        const double max_lon = min_max.second->lng;

        // Находим точки с минимальной и максимальной широтой
        const std::pair bottom_top = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_top.first->lat;
        max_lat_ = bottom_top.second->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};
