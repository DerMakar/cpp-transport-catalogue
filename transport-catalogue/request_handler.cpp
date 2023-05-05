#include "request_handler.h"

#include <sstream>

    using namespace std::literals;
    void RequestHandler::JasonStatRequest(const json::Array* stat_request, std::ostream& out) {
        using namespace json;
        Array document;
        document.reserve(stat_request->size());
        for (const auto& request : *stat_request) {
            const Dict* request_ptr = std::get_if<Dict>(&request.GetValue());
            std::string_view req_type = (*request_ptr).at("type"s).AsString();
            Dict result{ {"request_id"s, Node((*request_ptr).at("id"s).AsInt())} };
            if (req_type == "Stop"sv) {
                if (db_.FindStop((*request_ptr).at("name"s).AsString()) == nullptr) {
                    result["error_message"s] = Node("not found"s);
                }
                else {
                    const auto* set_of_buses = db_.GetStopInfo((*request_ptr).at("name"s).AsString());
                    Array buses;
                        if (set_of_buses != nullptr) {
                           for (const auto& bus : *set_of_buses) {
                                    buses.push_back(Node(bus));
                                }
                            
                        }
                        result["buses"s] = Node(buses);
                }
                document.push_back(Node(std::move(result)));
            }
            else if (req_type == "Bus"sv) {
                std::optional<transport_base_processing::BusInfo> bus_info = db_.GetBusInfo((*request_ptr).at("name"s).AsString());
                if (!bus_info) {
                    result["error_message"s] = Node("not found"s);
                }
                else {
                    result["curvature"s] = Node(bus_info.value().curvature);
                    result["route_length"s] = Node(bus_info.value().route_length);
                    result["stop_count"s] = Node(bus_info.value().stops_on_route);
                    result["unique_stop_count"s] = Node(bus_info.value().unique_stops);

                }
                document.push_back(Node(std::move(result)));
            }
            else if (req_type == "Map"sv){
            std::stringstream strm;
                RenderMap().Render(strm);
                result["map"s] = Node(strm.str());
                document.push_back(Node(std::move(result)));
            }
        }
        Print(Document(Node(document)), out);

    }

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<transport_base_processing::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        return db_.GetBusInfo(bus_name);
    }

    // Возвращает маршруты, проходящие через
    const std::set<std::string>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        return db_.GetStopInfo(stop_name);
    }

    svg::Document RequestHandler::RenderMap() const {
        using transport_base_processing::Bus;
        using transport_base_processing::Stop;
        svg::Document result;
        std::vector<svg::Text> stop_names;// private: std::deque<std::unique_ptr<Object>> objects_;
        SphereProjector point_corrector (db_.GetCoordCollect().begin(), db_.GetCoordCollect().end(), renderer_.GetRendSet().width, renderer_.GetRendSet().height, renderer_.GetRendSet().padding);
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
        for (auto& text : renderer_.CreateRouteNames(route_coords)) {
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
   