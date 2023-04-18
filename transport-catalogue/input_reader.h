#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <utility>
#include "transport_catalogue.h"

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
Query ParseQuery(std::string);
std::vector<std::string_view> SplitIntoWords(std::string_view);
std::vector<std::pair<long unsigned int, std::string>> ParseStopDistances(std::string_view info_);
void CreateBase(TransportCatalogue&);

