#pragma once

#include <unordered_map>
#include <deque>
#include <set>
#include <optional>

#include "domain.h"

namespace transport_base_processing {
		
	namespace detail {
		struct StopToDistanceHasher {
			size_t operator()(const std::pair<Stop*, Stop*>& info) const {
				auto h1 = hasher(info.first->name);
				auto h2 = hasher(info.second->name);
				return 37 * h1 + h2;
			}

			std::hash<std::string> hasher;
		};
	}
	

	class TransportCatalogue {
		
	public:
		using DistanceInfo = std::unordered_map<std::pair<Stop*, Stop*>, double, detail::StopToDistanceHasher>;
		using StopDistancesInfo = std::vector<std::pair<long unsigned int, std::string>>;
		
		void AddStop(Stop stop);
		void AddDistance(const StopDistancesInfo& info);
		void AddBus(Bus bus);
		void FullStopByBuses(const Bus& bus);
		const Stop* FindStop(std::string_view stopname) const;
		const Bus* FindBus(std::string_view busname) const;
		std::optional<BusInfo> GetBusInfo (std::string_view busname) const;
		const std::unordered_map<std::string_view, Stop*>& GetStopsMap ();
		const std::deque<Bus>& GetBuses() const;
		const std::deque<Stop>& GetStops() const;
		const std::vector<geo::Coordinates>& GetCoordCollect() const;
		const DistanceInfo& GetDistanceCollection() const;
		const std::set<std::string>* GetStopInfo(std::string_view stopname) const;
		void CountDistances(std::string_view);

	private:
		std::deque<Stop> stops;
		std::deque<Bus> buses;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop;
		std::unordered_map<std::string_view, Bus*> busname_to_bus;
		std::unordered_map<std::string_view, std::set<std::string>> stopname_to_bus;
		std::unordered_map<std::pair<Stop*, Stop*>, long unsigned int, detail::StopToDistanceHasher> stop_to_distance;
		DistanceInfo stops_to_distance;
		std::vector<geo::Coordinates> coordinates_collection;
		
	};

	std::ostream& operator<<(std::ostream& out, const BusInfo& info);
}