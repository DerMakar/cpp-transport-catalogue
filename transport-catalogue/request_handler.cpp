#include "request_handler.h"


namespace transport_base_processing {
    using namespace std::literals;
    const transport_base_processing::TransportCatalogue& RequestHandler::GetBase() const {
        return db_;
    }

    const transport_base_processing::MapRenderer& RequestHandler::GetRenderSet() const {
        return renderer_;
    }

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<transport_base_processing::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        return db_.GetBusInfo(bus_name);
    }

    // Возвращает маршруты, проходящие через
    const std::set<std::string>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        return db_.GetStopInfo(stop_name);
    }

    std::optional<graph::Router<double>::RouteInfo> RequestHandler::BuildRoute(std::string_view from, std::string_view to) const {
        size_t vertex_from = db_.FindStop(from)->id;
        size_t vertex_to = db_.FindStop(to)->id;
        if (vertex_from != vertex_to) {
            return db_router_.BuildRoute(vertex_from, vertex_to);
        }
        else {
            return graph::Router<double>::RouteInfo();
        }
    }

    svg::Document RequestHandler::RenderMap() const {
        using transport_base_processing::Bus;
        using transport_base_processing::Stop;
        svg::Document result;
        std::vector<svg::Text> stop_names;// private: std::deque<std::unique_ptr<Object>> objects_;
        SphereProjector point_corrector(db_.GetCoordCollect().begin(), db_.GetCoordCollect().end(), renderer_.GetRendSet().width, renderer_.GetRendSet().height, renderer_.GetRendSet().padding);
        std::deque<Bus> all_buses = db_.GetBuses();
        stop_names.reserve(all_buses.size() * 2);
        sort(all_buses.begin(), all_buses.end(), [](const Bus& lhs, const Bus& rhs) {return lhs.name < rhs.name; });
        std::map<std::string_view, std::vector<svg::Point>> route_coords;
        std::map<std::string_view, svg::Point> stops_on_routes;
        for (const auto& bus : all_buses) {
            if (bus.route.empty()) {
                route_coords[bus.name] = {};
            }
            else {
                for (const Stop* stop : bus.route) {
                    route_coords[bus.name].reserve(bus.route.size());
                    route_coords[bus.name].push_back(point_corrector(stop->coordinates));
                    stops_on_routes[stop->name] = point_corrector(stop->coordinates);
                }

            }

        }
        for (auto& polyline : renderer_.CreateBusLine(route_coords)) {
            result.Add(std::move(polyline));
        }
        for (auto& text : renderer_.CreateRouteNames(route_coords, db_)) {
            result.Add(std::move(text));
        }
        for (auto& stop : renderer_.CreateStops(stops_on_routes)) {
            result.Add(std::move(stop));
        }
        for (auto& name : renderer_.CreateStopsNames(stops_on_routes)) {
            result.Add(std::move(name));
        }


        return result;

    }
} // namespace transport_base_processing 