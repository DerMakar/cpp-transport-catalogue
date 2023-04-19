#include "transport_catalogue.h"
#include "stat_reader.h"
#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include "input_reader.h"
#include "geo.h"

using namespace std::literals;

void TransportCatalogue::AddStop(std::string data) {     
        stops.push_back(std::move(ParseStopInfo(data)));
        stopname_to_stop[std::string_view(stops.back().name)] = &stops.back();
           
}

void TransportCatalogue::AddDistance(std::string_view start_stop, std::string_view info) {
    auto distances = DistanceInfoInVector(info);
    if(!distances.empty()){
        for (const auto& dist_info : distances) {
            Stop* destination = stopname_to_stop.at(dist_info.second);
            Stop* start = stopname_to_stop.at(start_stop);
            std::pair<Stop*, Stop*> data{ start, destination };
            stop_to_distance[data] = dist_info.first;
        }
    }
}


void TransportCatalogue::AddBus(std::string_view data) { 
    auto bus_info = ParseBusInfo(data);
    for (auto& stop : bus_info.second) {
        if (stopname_to_stop.count(stop) != 0) {
            bus_info.first.route.push_back(stopname_to_stop.at(stop));
            
        }
        else {
                throw std::invalid_argument("You're trying add unvalid Stop to route"s);
        }
    }
    if (data.find('-') != std::string::npos) {
        std::vector<Stop*> tmp = bus_info.first.route;
        bus_info.first.route.resize(bus_info.first.route.size() * 2 - 1);
        std::copy_backward(tmp.rbegin() + 1, tmp.rend(), bus_info.first.route.end());
    }
    buses.push_back(bus_info.first);
    busname_to_bus[std::string_view(buses.back().name)] = &buses.back();
    FullStopByBuses(buses.back());
}

void TransportCatalogue::FullStopByBuses(const Bus& bus) {
    for (const Stop* stop : bus.route) {
        stopname_to_bus[stop->name].insert(bus.name);
    }
}

const Stop* TransportCatalogue::FindStop(std::string_view stop) const {
    if (stopname_to_stop.count(stop) == 0) {
        return nullptr;
    }
    return &(*stopname_to_stop.at(stop));
}

const Bus* TransportCatalogue::FindBus(std::string_view bus) const {
    if (busname_to_bus.count(bus) == 0) {
        return nullptr;
    }
    return busname_to_bus.at(bus);
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
    return stops;
}

const TransportCatalogue::DistanceInfo& TransportCatalogue::GetDistanceCollection() const {
    return stops_to_distance;
}

void TransportCatalogue::CountDistances(std::string_view stop) {
    Stop* start_of_dist = stopname_to_stop.at(stop);
    for (const auto& stop_in_coll : stopname_to_stop) {
        Stop* end_of_dist = stop_in_coll.second;
        
    if (stops_to_distance.count({ start_of_dist, end_of_dist }) != 0) {
        continue;
    }
    stops_to_distance[{ start_of_dist, end_of_dist }] = ComputeDistance({ start_of_dist->lat_ , start_of_dist->long_ }, { end_of_dist->lat_ ,end_of_dist->long_ });
    if (stops_to_distance.count({ end_of_dist, start_of_dist }) != 0) {
        continue;
    }
    stops_to_distance[{ end_of_dist, start_of_dist}] = ComputeDistance({ end_of_dist->lat_ ,end_of_dist->long_ }, { start_of_dist->lat_ , start_of_dist->long_ });
    }
    
}

BusInfo TransportCatalogue::GetBusInfo(std::string_view bus) const { 
    BusInfo bus_info{ 0, 1, 0, 0.0 };
    bus_info.stops_on_route = busname_to_bus.at(bus)->route.size();
    std::vector<Stop*> tmp;
    tmp.reserve(busname_to_bus.at(bus)->route.size());
    Stop* left = busname_to_bus.at(bus)->route[0];
    tmp.push_back(left);
    for (int i = 1; i < busname_to_bus.at(bus)->route.size(); ++i) {
        Stop* right = busname_to_bus.at(bus)->route[i];
        bus_info.curvature += ComputeDistance({ left->lat_, left->long_ }, { right->lat_, right->long_ });
        if (stop_to_distance.count({ left, right }) != 0) {
            bus_info.route_lenght += stop_to_distance.at({ left, right });
        }
        else {
            bus_info.route_lenght += stop_to_distance.at({ right, left });
        }
        left = right;
        if (std::count(tmp.begin(), tmp.end(), right) == 0) {
            ++bus_info.unique_stops;
            tmp.push_back(std::move(right));
        }
        
    }
    bus_info.curvature = bus_info.route_lenght / bus_info.curvature;
    return bus_info;
}

const std::set<std::string>* TransportCatalogue::GetStopInfo(std::string_view stop) const {
    if (stopname_to_bus.count(stop) == 0) {
        return nullptr;
    }
    return &stopname_to_bus.at(stop);
}

std::ostream& operator<<(std::ostream& out, const BusInfo& info){
    out << info.stops_on_route << " stops on route, "s;
    out << info.unique_stops << " unique stops, "s;
    out << std::setprecision(6) << info.route_lenght * 1.0 << " route length, "s;
    out << std::setprecision(6) << info.curvature << " curvature"s;
    return out;

}
