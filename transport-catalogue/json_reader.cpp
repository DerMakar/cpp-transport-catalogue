#include "json_reader.h"

namespace json {
    using namespace std::literals;
    using transport_base_processing::Stop;
    using transport_base_processing::Bus;

    void JsonBaseProcessing::CreateBase(transport_base_processing::TransportCatalogue& base) {
        const Dict* data = std::get_if<Dict>(&document_.GetRoot().GetValue());
        const Array* base_request = std::get_if<Array>(&(*data).at("base_requests"s).GetValue());
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

    const Array* JsonBaseProcessing::GetStatRequest() const {
        const Dict* data = std::get_if<Dict>(&document_.GetRoot().GetValue());
        const Array* stat_request = std::get_if<Array>(&(*data).at("stat_requests"s).GetValue());
        return stat_request;
    }

    std::vector<Stop> JsonBaseProcessing::ParseStopRequests(const Array* data) {
        std::vector<Stop> stops_to_add;
        for (const auto& request_in_collection : *data) {
            const Dict* request = std::get_if<Dict>(&request_in_collection.GetValue());
            if (std::get<std::string>((*request).at("type"s).GetValue()) != "Stop"s) {
                continue;
            }
            else {
                Stop stop;
                stop.name = std::get<std::string>((*request).at("name"s).GetValue());
                stop.coordinates.lat = std::get<double>((*request).at("latitude"s).GetValue());
                stop.coordinates.lng = std::get<double>((*request).at("longitude"s).GetValue());
                if ((*request).count("road_distances"s) != 0) {
                    const Dict* stops_distance_info = std::get_if<Dict>(&(*request).at("road_distances"s).GetValue());
                    for (const auto& [stop_name, dist] : *stops_distance_info) {
                        stop_to_stop_distances[stop.name][stop_name] = std::get<int>(dist.GetValue());
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
            const Dict* request = std::get_if<Dict>(&request_in_collection.GetValue());
            if (std::get<std::string>((*request).at("type"s).GetValue()) != "Bus"s) {
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
        result.reserve(stop_to_stop_distances.at(stop).size() + 1);
        result.push_back({ 0, stop });
        for (const auto& [stop_name, dist] : stop_to_stop_distances.at(stop)) {
            result.push_back(std::move(std::make_pair(dist, stop_name)));
            }
        return result;
    }
    

   
}// namespace json