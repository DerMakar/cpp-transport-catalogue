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
	Bus ParseBusInfo(TransportCatalogue& base, std::string_view data);
	TransportCatalogue::StopDistancesInfo DistanceInfoInVector(std::string_view info);
	TransportCatalogue::StopDistancesInfo ParseStopDistances(std::string_view info);
	void CreateBase(TransportCatalogue& base);
}
