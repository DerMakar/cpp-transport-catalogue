#include "transport_catalogue.h"
#include "geo.h"

#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std::literals;

namespace transport_base_processing {

    void TransportCatalogue::AddStop(Stop stop) {
        stops.push_back(std::move(stop));
        stopname_to_stop[std::string_view(stops.back().name)] = &stops.back();
    }

    void TransportCatalogue::AddDistance(const StopDistancesInfo& info) {
        if (!info.empty()) {
            Stop* start = stopname_to_stop.at(info[0].second);
            for (size_t i = 1; i < info.size(); ++i) {
                Stop* destination = stopname_to_stop.at(info[i].second);
                std::pair<Stop*, Stop*> data{ start, destination };
                stop_to_distance[data] = info[i].first;
            }
        }
    }

    void TransportCatalogue::AddDistanceToMap(const Stop* start, const Stop* finish, long unsigned int distance) {
        std::pair<Stop*, Stop*> stops_pair = std::make_pair(const_cast<Stop*>(start), const_cast<Stop*>(finish));
        stop_to_distance[stops_pair] = distance;
    }


    void TransportCatalogue::AddBus(Bus bus) {
        buses.push_back(std::move(bus));
        busname_to_bus[std::string_view(buses.back().name)] = &buses.back();
        FullStopByBuses(buses.back());
    }

    void TransportCatalogue::FullStopByBuses(const Bus& bus) {
        for (const Stop* stop : bus.route) {
            if (stopname_to_bus.count(stop->name) == 0) {
                coordinates_collection.push_back(stop->coordinates);
            }
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

    const std::vector<geo::Coordinates>& TransportCatalogue::GetCoordCollect() const {
        return coordinates_collection;
    }

    const std::unordered_map<std::string_view, Stop*>& TransportCatalogue::GetStopsMap() {
        return stopname_to_stop;
    }

    const TransportCatalogue::RouteLength& TransportCatalogue::GetDistanceCollection() const {
        return stop_to_distance;
    }

    void TransportCatalogue::CountDistances(std::string_view stop) {
        Stop* start_of_dist = stopname_to_stop.at(stop);
        for (const auto& stop_in_coll : stopname_to_stop) {
            Stop* end_of_dist = stop_in_coll.second;
            if (stops_to_distance.count({ start_of_dist, end_of_dist }) != 0) {
                continue;
            }
            stops_to_distance[{ start_of_dist, end_of_dist }] = geo::ComputeDistance({ start_of_dist->coordinates.lat , start_of_dist->coordinates.lng }, { end_of_dist->coordinates.lat ,end_of_dist->coordinates.lng });
            if (stops_to_distance.count({ end_of_dist, start_of_dist }) != 0) {
                continue;
            }
            stops_to_distance[{ end_of_dist, start_of_dist}] = geo::ComputeDistance({ end_of_dist->coordinates.lat ,end_of_dist->coordinates.lng }, { start_of_dist->coordinates.lat , start_of_dist->coordinates.lng });
        }
    }

    std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view busname) const {
        if (busname_to_bus.count(busname) == 0) {
            return {};
        }
        BusInfo bus_info{ 0, 1, 0, 0.0 };
        bus_info.stops_on_route = busname_to_bus.at(busname)->route.size();
        std::vector<Stop*> tmp;
        tmp.reserve(busname_to_bus.at(busname)->route.size());
        Stop* left = busname_to_bus.at(busname)->route[0];
        tmp.push_back(left);
        for (size_t i = 1; i < busname_to_bus.at(busname)->route.size(); ++i) {
            Stop* right = busname_to_bus.at(busname)->route[i];
            bus_info.curvature += geo::ComputeDistance({ left->coordinates.lat, left->coordinates.lng }, { right->coordinates.lat, right->coordinates.lng });
            if (stop_to_distance.count({ left, right }) != 0) {
                bus_info.route_length += stop_to_distance.at({ left, right });
            }
            else {
                bus_info.route_length += stop_to_distance.at({ right, left });
            }
            left = right;
            if (std::count(tmp.begin(), tmp.end(), right) == 0) {
                ++bus_info.unique_stops;
                tmp.push_back(std::move(right));
            }
        }
        bus_info.curvature = bus_info.route_length / bus_info.curvature;
        return bus_info;
    }

    const std::set<std::string>* TransportCatalogue::GetStopInfo(std::string_view stop) const {
        if (stopname_to_bus.count(stop) == 0) {
            return nullptr;
        }
        return &stopname_to_bus.at(stop);
    }

    void TransportCatalogue::SetBusWaitTime(unsigned short int time) {
        bus_wait_time = time;
    }

    void TransportCatalogue::SetBusVelocity(double speed) {
        const int meters_in_km = 1000;
        const int sec_in_min = 60;
        bus_velocity = speed * meters_in_km / sec_in_min;
    }

    std::pair<unsigned short int, double> TransportCatalogue::GetWaitVelocityInfo() const {
        return { bus_wait_time, bus_velocity };
    }

    std::ostream& operator<<(std::ostream& out, const BusInfo& info) {
        out << info.stops_on_route << " stops on route, "s;
        out << info.unique_stops << " unique stops, "s;
        out << std::setprecision(9) << double(info.route_length) << " route length, "s;
        out << std::setprecision(9) << info.curvature << " curvature"s;
        return out;
    }
}