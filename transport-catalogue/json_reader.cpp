#include "json_reader.h"


namespace json {
    using namespace std::literals;
    using transport_base_processing::Stop;
    using transport_base_processing::Bus;

    void JsonBaseProcessing::CreateBase(transport_base_processing::TransportCatalogue& base) {
        const Dict* data = &document_.GetRoot().AsMap();
        
        const Dict* render_settings = &(*data).at("render_settings"s).AsMap();
        ParseRenderSettings(render_settings);
        
        const Array* base_request = &(*data).at("base_requests"s).AsArray();
        std::vector<Stop> stops_to_add = std::move(ParseStopRequests(base_request));

        for (Stop& stop : stops_to_add) {
            base.AddStop(stop);
        }

        for (Stop& stop : stops_to_add) {
           base.AddDistance(std::move(ParseStopDistInfo(stop.name)));
        }
       
        std::vector<Bus> bases_to_add = std::move(ParseBusRequests(base_request, base));
                
       
        for (Bus& bus : bases_to_add) {
           base.AddBus(std::move(bus));
        }

        

    }

    const transport_base_processing::RenderSettings& JsonBaseProcessing::GetRenderSet() const {
        return render_settings_;
    }

    Document JsonBaseProcessing::GetStatRequest(const transport_base_processing::RequestHandler handler) const {
        const Dict* data = &document_.GetRoot().AsMap();
        const Array* stat_request = &(*data).at("stat_requests"s).AsArray();
        Builder document;
        document.StartArray();
            for (const auto& request : *stat_request) {
                const Dict* request_ptr = std::get_if<Dict>(&request.GetValue());
                std::string_view req_type = (*request_ptr).at("type"s).AsString();
                document.StartDict().Key("request_id"s).Value((*request_ptr).at("id"s).AsInt());
                if (req_type == "Stop"sv) {
                    if (handler.GetBase().FindStop((*request_ptr).at("name"s).AsString()) == nullptr) {
                        document.Key("error_message"s).Value("not found"s);
                    }
                    else {
                        const auto* set_of_buses = handler.GetBase().GetStopInfo((*request_ptr).at("name"s).AsString());
                        document.Key("buses"s).StartArray();
                        if (set_of_buses != nullptr) {
                            for (const auto bus : *set_of_buses) {
                                document.Value(bus);
                            }
                            

                        }
                        document.EndArray();
                    }
                    
                }
                else if (req_type == "Bus"sv) {
                    std::optional<transport_base_processing::BusInfo> bus_info = handler.GetBase().GetBusInfo((*request_ptr).at("name"s).AsString());
                    if (!bus_info) {
                        document.Key("error_message"s).Value("not found"s);
                    }
                    else {
                        document.Key("curvature"s).Value(bus_info.value().curvature)
                        .Key("route_length"s).Value(double(bus_info.value().route_length))
                        .Key("stop_count"s).Value(bus_info.value().stops_on_route)
                        .Key("unique_stop_count"s).Value(bus_info.value().unique_stops);

                    }
                    
                }
                else if (req_type == "Map"sv) {
                    std::stringstream strm;
                    handler.RenderMap().Render(strm);
                    document.Key("map"s).Value(strm.str());
                    
                }
                document.EndDict();
            }
            return Document{ document.EndArray().Build() };
    }

    std::vector<Stop> JsonBaseProcessing::ParseStopRequests(const Array* data) {
        std::vector<Stop> stops_to_add;
        for (const auto& request_in_collection : *data) {
            const Dict* request = &request_in_collection.AsMap();
            if ((*request).at("type"s).AsString() != "Stop"s) {
                continue;
            }
            else {
                Stop stop;
                stop.name = (*request).at("name"s).AsString();
                stop.coordinates.lat = (*request).at("latitude"s).AsDouble();
                stop.coordinates.lng = (*request).at("longitude"s).AsDouble();
                if ((*request).count("road_distances"s) != 0) {
                    const Dict* stops_distance_info = &(*request).at("road_distances"s).AsMap();
                    for (const auto& [stop_name, dist] : *stops_distance_info) {
                        stop_to_stop_distances[stop.name][stop_name] = dist.AsInt();
                    }
                }
                stops_to_add.push_back(std::move(stop));
            }
        }
        return stops_to_add;
    }
    
    std::vector<Bus> JsonBaseProcessing::ParseBusRequests(const Array* data, transport_base_processing::TransportCatalogue& base_) {
        std::vector<Bus> bases_to_add;
        for (const auto & request_in_collection : *data) {
            const Dict* request = &request_in_collection.AsMap();
            if ((*request).at("type"s).AsString() != "Bus"s) {
                continue;
            }
            else {
                Bus bus;
                bus.name = (*request).at("name"s).AsString();
                bus.is_circle = (*request).at("is_roundtrip"s).AsBool();
                const Array& stops = (*request).at("stops"s).AsArray();
                for (const auto& stop : stops) {
                    std::string_view sv_stop = stop.AsString();
                    if (base_.GetStopsMap().count(sv_stop) != 0) {
                        bus.route.push_back(base_.GetStopsMap().at(sv_stop));

                    }
                    else {
                        throw std::invalid_argument("You're trying add unvalid Stop to route"s);
                    }
                }
                if (!bus.is_circle) {
                    std::vector<Stop*> tmp = bus.route;
                    bus.route.resize(bus.route.size() * 2 - 1);
                    std::copy_backward(tmp.rbegin() + 1, tmp.rend(), bus.route.end());
                }
                bases_to_add.push_back(std::move(bus));
            }
        }
        return bases_to_add;
    }
    
    StopDistancesInfo JsonBaseProcessing::ParseStopDistInfo(std::string& stop) {
        StopDistancesInfo result; // std::vector<std::pair<long unsigned int, std::string>>
        if (stop_to_stop_distances.count(stop) == 0) {
            return {};
        }
        result.reserve(stop_to_stop_distances.at(stop).size() + 1);
        result.push_back({ 0, stop });
        for (const auto& [stop_name, dist] : stop_to_stop_distances.at(stop)) {
            result.push_back(std::move(std::make_pair(dist, stop_name)));
            }
        return result;
    }
    
    svg::Color JsonBaseProcessing::ParseColor(const Node& data) const {
        svg::Color color;
        if (!data.IsArray()) {
             color = data.AsString();
        }
        else {
            const Array& underlayer_color = data.AsArray();
            if (underlayer_color.size() == 3) {
                svg::Rgb rgb_color;
                rgb_color.red = underlayer_color[0].AsInt();
                rgb_color.green = underlayer_color[1].AsInt();
                rgb_color.blue = underlayer_color[2].AsInt();
                color = rgb_color;
            }
            else {
                svg::Rgba rgba_color;
                rgba_color.red = underlayer_color[0].AsInt();
                rgba_color.green = underlayer_color[1].AsInt();
                rgba_color.blue = underlayer_color[2].AsInt();
                rgba_color.opacity = underlayer_color[3].AsDouble();
                color = rgba_color;
            }
        }
        return color;
    }

    void JsonBaseProcessing::ParseRenderSettings(const Dict* data) {
        using namespace transport_base_processing;
        render_settings_.width = (*data).at("width"s).AsDouble();
        render_settings_.height = (*data).at("height"s).AsDouble();
        render_settings_.padding = (*data).at("padding"s).AsDouble();
        render_settings_.line_width = (*data).at("line_width"s).AsDouble();
        render_settings_.stop_radius = (*data).at("stop_radius"s).AsDouble();
        render_settings_.bus_label_font_size = static_cast<long unsigned int>((*data).at("bus_label_font_size"s).AsInt());
        render_settings_.bus_label_offset.x = (*data).at("bus_label_offset"s).AsArray()[0].AsDouble();
        render_settings_.bus_label_offset.y = (*data).at("bus_label_offset"s).AsArray()[1].AsDouble();
        render_settings_.stop_label_font_size = static_cast<long unsigned int>((*data).at("stop_label_font_size"s).AsInt());
        render_settings_.stop_label_offset.x = (*data).at("stop_label_offset"s).AsArray()[0].AsDouble();
        render_settings_.stop_label_offset.y = (*data).at("stop_label_offset"s).AsArray()[1].AsDouble();
        render_settings_.underlayer_color = ParseColor((*data).at("underlayer_color"s));
        render_settings_.underlayer_width = (*data).at("underlayer_width"s).AsDouble();
        for (Node color : (*data).at("color_palette"s).AsArray()) {
            render_settings_.color_palette.push_back(ParseColor(color));
        } 
    }

   
}// namespace json