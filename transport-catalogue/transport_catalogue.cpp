#include "transport_catalogue.h"
#include "stat_reader.h"
#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include "input_reader.h"
#include "geo.h"
#include "log_duration.h"



using namespace std::literals;


void TransportCatalogue::AddStop(std::string data) {     
        Stop result;
        auto start_of_stopname = data.find_first_not_of(" ");
        auto end_of_stopname = data.find(':');
        result.name = data.substr(start_of_stopname, end_of_stopname - start_of_stopname);
        auto start_of_latitude = data.find_first_of("-0123456789", end_of_stopname);
        auto end_of_latitude = data.find_first_of(",", end_of_stopname);
        std::string latitude = data.substr(start_of_latitude, end_of_latitude - start_of_latitude);
        result.lat_ = std::stod(move(latitude));
        auto end_of_longitude = data.find_first_not_of("-0123456789.", end_of_latitude + 2);
        std::string longitude = data.substr(end_of_latitude + 2, end_of_longitude - end_of_latitude - 2);
        result.long_ = std::stod(move(longitude));
        stops.push_back(std::move(result));
        stopname_to_stop[std::string_view(stops.back().name)] = &stops.back();
           
}

void TransportCatalogue::AddDistance(Stop* stop_, std::string_view info_) {
    info_.remove_prefix(info_.find(',') + 1);
    if (info_.find(',') != std::string::npos) {
        info_.remove_prefix(info_.find(',') + 1);
        std::vector<std::pair<long unsigned int, std::string>> distances = ParseStopDistances(info_);
    for (const auto& dist_info : distances) {
        Stop* destination = stopname_to_stop.at(dist_info.second);
        std::pair<Stop*, Stop*> data{ stop_, destination };
        stop_to_distance[data] = dist_info.first;
    }
    }
}


void TransportCatalogue::AddBus(std::string_view data) { 
    Bus result;
    auto start_of_busname = data.find_first_not_of(" ");
    auto end_of_busname = data.find(':');
    result.name = data.substr(start_of_busname, end_of_busname);
    auto stops_ = SplitIntoWords(data.substr(end_of_busname + 1));
    for (auto& stop_ : stops_) {
        if (stopname_to_stop.count(stop_) != 0) {
            result.route.push_back(stopname_to_stop.at(stop_));
            
        }
        else {
                throw std::invalid_argument("You're trying add unvalid Stop to route"s);
        }
    }
    if (data.find('-') != std::string::npos) {
        std::vector<Stop*> tmp = result.route;
        result.route.resize(result.route.size() * 2 - 1);
        std::copy_backward(tmp.rbegin() + 1, tmp.rend(), result.route.end());
    }
    buses.push_back(result);
    busname_to_bus[std::string_view(buses.back().name)] = &buses.back();
    for (const Stop* stop_ : buses.back().route) {
        stopname_to_bus[stop_->name].insert(buses.back().name);
    }
}

Stop* TransportCatalogue::FindStop(std::string_view stop) const { 
    if (stopname_to_stop.count(stop) == 0) {
        return nullptr;
    }
    return stopname_to_stop.at(stop);
}

Bus* TransportCatalogue::FindBus(std::string_view bus) const { 
    if (busname_to_bus.count(bus) == 0) {
        return nullptr;
    }
    return busname_to_bus.at(bus);
}

std::deque<Bus> TransportCatalogue::GetBuses() const {
    return buses;
}

std::deque<Stop> TransportCatalogue::GetStops() const {
    return stops;
}

TransportCatalogue::DistanceInfo TransportCatalogue::GetDistanceCollection() const {
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

std::tuple<int, int, long unsigned int, double> TransportCatalogue::GetBusInfo(std::string_view bus) const { 
    int num_of_stops_total = busname_to_bus.at(bus)->route.size();
    int num_of_stops_uniq = 1;
    long unsigned int route_lenght = 0;
    double curvature = 0.0;
    std::vector<Stop*> tmp;
    tmp.reserve(busname_to_bus.at(bus)->route.size());
    Stop* left = busname_to_bus.at(bus)->route[0];
    tmp.push_back(left);
    for (int i = 1; i < busname_to_bus.at(bus)->route.size(); ++i) {
        Stop* right = busname_to_bus.at(bus)->route[i];
        curvature += ComputeDistance({ left->lat_, left->long_ }, { right->lat_, right->long_ });
        if (stop_to_distance.count({ left, right }) != 0) {
            route_lenght += stop_to_distance.at({ left, right });
        }
        else {
            route_lenght += stop_to_distance.at({ right, left });
        }
        left = right;
        if (std::count(tmp.begin(), tmp.end(), right) == 0) {
            ++num_of_stops_uniq;
            tmp.push_back(std::move(right));
        }
        
    }
    curvature = route_lenght / curvature;
    return { num_of_stops_total , num_of_stops_uniq , route_lenght,  curvature };
}

std::set<std::string> TransportCatalogue::GetStopInfo(std::string_view stop_) const {
    if (stopname_to_bus.count(stop_) == 0) {
        return {};
    }
    return stopname_to_bus.at(stop_);
}

std::ostream& operator<<(std::ostream& out, const std::tuple<int, int, long unsigned int, double>& info_){
    out << std::get<0>(info_) << " stops on route, "s;
    out << std::get<1>(info_) << " unique stops, "s;
    out << std::setprecision(6) << std::get<2>(info_) * 1.0 << " route length, "s;
    out << std::setprecision(6) << std::get<3>(info_) << " curvature"s;
    return out;

}











