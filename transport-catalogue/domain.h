#pragma once
#include "geo.h"
#include <vector>
#include <string>

namespace transport_base_processing {
	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> route;
		bool is_circle = false;
	};

	struct BusInfo {
		int stops_on_route;
		int unique_stops;
		long unsigned int route_length;
		double curvature;
	};


} // namespace transport_base_processing