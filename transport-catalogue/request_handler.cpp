#include "request_handler.h"


    using namespace std::literals;
    void RequestHandler::JasonStatRequest(const json::Array* stat_request, std::ostream& out) {
        using namespace json;
        Array document;
        document.reserve(stat_request->size());
        for (const auto& request : *stat_request) {
            const Dict* request_ptr = std::get_if<Dict>(&request.GetValue());
            std::string_view req_type = (*request_ptr).at("type"s).AsString();
            if (req_type == "Stop"sv) {
                Dict result{ {"request_id"s, Node((*request_ptr).at("id"s).AsInt())} };
                const auto* set_of_buses = db_.GetStopInfo((*request_ptr).at("name"s).AsString());
                if (set_of_buses == nullptr) {
                    result["error_message"s] = Node("not found"s);
                }
                else {
                    Array buses;
                    for (const auto& bus : *set_of_buses) {
                        buses.push_back(Node(bus));
                    }
                    result["buses"s] = Node(buses);
                }
                document.push_back(Node(std::move(result)));
            }
            else if (req_type == "Bus"sv) {
                Dict result{ {"request_id"s, Node((*request_ptr).at("id"s).AsInt())} };
                std::optional<transport_base_processing::BusInfo> bus_info = db_.GetBusInfo((*request_ptr).at("name"s).AsString());
                if (!bus_info) {
                    result["error_message"s] = Node("not found"s);
                }
                else {
                    result["curvature"s] = Node(bus_info.value().curvature);
                    result["route_length"s] = Node(bus_info.value().route_lenght);
                    result["stop_count"s] = Node(bus_info.value().stops_on_route);
                    result["unique_stop_count"s] = Node(bus_info.value().unique_stops);

                }
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
