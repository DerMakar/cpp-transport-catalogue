#include <iostream>

#include "input_reader.h"
#include "log_duration.h"



using namespace std::literals;

std::string ReadLine() {
    std::string s;
    getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    std::cin >> result;
    ReadLine();
    return result;
}

Query ParseQuery(std::string query) {
    Query result;
    if (query[0] == 'S') {
        result.type = QueryType::Stop;
    }
    else if (query[0] == 'B') {
        result.type = QueryType::Bus;
    }
    else {
        throw std::invalid_argument("Invalid query"s);
    }
    auto start_of_data = query.find_first_of(" ", 1);
    result.data = query.substr(start_of_data + 1);
    return result;
}

std::vector<std::string_view> SplitIntoWords(std::string_view str) {
    std::vector<std::string_view> result;
    char type = '-';
    if (str.find('>') != std::string::npos) type = '>';
    while (!str.empty()) {
        str.remove_prefix(std::min(str.size(), str.find_first_not_of(" ")));
        int64_t stop = str.find(type);
        result.push_back(str.substr(0, stop - 1));
        str.remove_prefix(std::min(str.size(), str.find_first_not_of(type, stop)));


    }
    return result;
}

std::vector<std::pair<long unsigned int, std::string>> ParseStopDistances(std::string_view info_) {
    std::vector<std::pair<long unsigned int, std::string>>  result;
    while (!info_.empty()) {
        if (info_[0] == ',') info_.remove_prefix(1);
        info_.remove_prefix(std::min(info_.size(), info_.find_first_not_of(" ")));
        int64_t stop = info_.find('m');
        long unsigned int dist_ = std::stol(static_cast<std::string>(info_.substr(0, stop)));
        info_.remove_prefix(info_.find("to", stop));
        info_.remove_prefix(info_.find_first_of(" ")+1);
        std::string stop_ = static_cast<std::string>(info_.substr(0, info_.find(',')));
        result.push_back({ dist_, stop_ });
        auto next = info_.find(',');
        info_.remove_prefix((std::min(info_.size(), next)));
    }
    return result;
}


void CreateBase(TransportCatalogue& base) { 
    int64_t num_of_queries = ReadLineWithNumber();
    std::vector<Query> data (num_of_queries);
    
        for (int64_t i = 0; i < num_of_queries; ++i) {
            data[i] = ParseQuery(ReadLine());
        }
    
    
        for (Query& query_ : data) {
            if (query_.type == QueryType::Stop) {
                base.AddStop(query_.data);
            }
        }
   
        for (Query& query_ : data) {
            if (query_.type == QueryType::Stop) {
                auto start_of_stopname = query_.data.find_first_not_of(" ");
                auto end_of_stopname = query_.data.find(':');
                auto name = query_.data.substr(start_of_stopname, end_of_stopname - start_of_stopname);
                base.AddDistance(base.FindStop(name), query_.data);
            }
        }
    
        for (Query& query_ : data) {
            if (query_.type == QueryType::Bus) {
                base.AddBus(move(query_.data));
            }
        }
    
    
}

