#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <utility>
#include "transport_catalogue.h"

namespace transport_base_processing {
	enum class QueryType {
		Stop,
		Bus,
	};

	struct Query {
		QueryType type;
		std::string data;
	};


	std::string ReadLine();
	int ReadLineWithNumber();
	Query ParseQuery(std::string_view query);
	std::vector<std::string_view> SplitIntoWords(std::string_view str);
	std::pair<bus_stop_processing::Bus, std::vector<std::string_view>> ParseBusInfo(std::string_view data);
	bus_stop_processing::Stop ParseStopInfo(std::string& data);
	std::vector<std::pair<long unsigned int, std::string>> DistanceInfoInVector(std::string_view info);
	std::vector<std::pair<long unsigned int, std::string>> ParseStopDistances(std::string_view info);
	void CreateBase(bus_stop_processing::TransportCatalogue&);
}
