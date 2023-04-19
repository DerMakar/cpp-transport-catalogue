#pragma once

#include <unordered_map>
#include <deque>
#include <vector>
#include <string>
#include <set>


struct Stop {
	std::string name;
	// у long не убрать суффикс _, иначе это будет тип, а не идентификатор.
	// тогда для однообразности и у lat оставим
	double lat_ = 0; // широта
	double long_ = 0; // долгота
};

struct Bus {
	std::string name;
	std::vector<Stop*> route;
};

struct BusInfo {
	int stops_on_route;
	int unique_stops;
	long unsigned int route_lenght;
	double curvature;
};


// запихнуть в namespace detail
struct StopToDistanceHasher {
	size_t operator()(const std::pair<Stop*, Stop*>& info) const {
		auto h1 = hasher(info.first->name);
		auto h2 = hasher(info.second->name);
		return 37 * h1 + h2;
	}

	std::hash<std::string> hasher;
};

class TransportCatalogue {
	using DistanceInfo = std::unordered_map<std::pair<Stop*, Stop*>, double, StopToDistanceHasher>;
public:
	void AddStop(std::string stopname);
	void AddDistance(std::string_view start_stop, std::string_view dist_info);
	void AddBus(std::string_view busname);
	void FullStopByBuses(const Bus& bus);
	const Stop* FindStop(std::string_view stopname) const;
	const Bus* FindBus(std::string_view busname) const;
	BusInfo GetBusInfo(std::string_view busname) const;
	const std::deque<Bus>& GetBuses () const;
	const std::deque<Stop>& GetStops() const;
	const DistanceInfo& GetDistanceCollection() const;
	const std::set<std::string>* GetStopInfo(std::string_view stopname) const;
	
	
private:
	std::deque<Stop> stops;
	std::deque<Bus> buses;
	std::unordered_map<std::string_view, Stop*> stopname_to_stop; 
	std::unordered_map<std::string_view, Bus*> busname_to_bus;	
	std::unordered_map<std::string_view, std::set<std::string>> stopname_to_bus;
	std::unordered_map<std::pair<Stop*, Stop*>, long unsigned int, StopToDistanceHasher> stop_to_distance;
	DistanceInfo stops_to_distance;
	void CountDistances(std::string_view);
};

std::ostream& operator<<(std::ostream& out, const BusInfo& info);