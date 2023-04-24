#include <iostream>

#include "input_reader.h"

namespace transport_base_processing {
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

    Query ParseQuery(std::string_view query) {
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

    Bus ParseBusInfo(TransportCatalogue& base, std::string_view data) {
        Bus result;
        auto start_of_busname = data.find_first_not_of(" ");
        auto end_of_busname = data.find(':');
        result.name = data.substr(start_of_busname, end_of_busname);
        auto stops = SplitIntoWords(data.substr(end_of_busname + 1));
        if (data.find('-') != std::string::npos) result.is_circle = true;
        for (auto& stop : stops) {
            if (base.GetStopsMap().count(stop) != 0) {
                result.route.push_back(base.GetStopsMap().at(stop));

            }
            else {
                throw std::invalid_argument("You're trying add unvalid Stop to route"s);
            }
        }
        if (result.is_circle) {
            std::vector<Stop*> tmp = result.route;
            result.route.resize(result.route.size() * 2 - 1);
            std::copy_backward(tmp.rbegin() + 1, tmp.rend(), result.route.end());
        }
        return result;
    }

    Stop ParseStopInfo(std::string& data) {
        Stop result;
        auto start_of_stopname = data.find_first_not_of(" ");
        auto end_of_stopname = data.find(':');
        result.name = data.substr(start_of_stopname, end_of_stopname - start_of_stopname);
        auto start_of_latitude = data.find_first_of("-0123456789", end_of_stopname);
        auto end_of_latitude = data.find_first_of(",", end_of_stopname);
        std::string latitude = data.substr(start_of_latitude, end_of_latitude - start_of_latitude);
        result.coordinates.lat = std::stod(move(latitude));
        auto end_of_longitude = data.find_first_not_of("-0123456789.", end_of_latitude + 2);
        std::string longitude = data.substr(end_of_latitude + 2, end_of_longitude - end_of_latitude - 2);
        result.coordinates.lng = std::stod(move(longitude));
        return result;
    }

    TransportCatalogue::StopDistancesInfo DistanceInfoInVector(std::string_view info) {
        if(info.find(',', info.find(',') + 1) == std::string::npos){
            return {};
        }
        TransportCatalogue::StopDistancesInfo  result;
        result.reserve(std::count(info.begin(), info.end(), ','));
        auto start_of_stopname = info.find_first_not_of(" ");
        auto end_of_stopname = info.find(':');
        std::string name = static_cast<std::string> (info.substr(start_of_stopname, end_of_stopname - start_of_stopname));
        result.push_back({ 0, name });
        info.remove_prefix(info.find(',', info.find(',') + 1) + 1);
        while (!info.empty()) {
            if (info[0] == ',') info.remove_prefix(1);
            info.remove_prefix(std::min(info.size(), info.find_first_not_of(" ")));
            int64_t m_point = info.find('m');
            long unsigned int dist = std::stol(static_cast<std::string>(info.substr(0, m_point)));
            info.remove_prefix(info.find("to", m_point));
            info.remove_prefix(info.find_first_of(" ") + 1);
            std::string stop = static_cast<std::string>(info.substr(0, info.find(',')));
            result.push_back({ dist, stop });
            auto next = info.find(',');
            info.remove_prefix((std::min(info.size(), next)));
        }
        return result;
    }


    void CreateBase(TransportCatalogue& base) {
        int64_t num_of_queries = ReadLineWithNumber();
        std::vector<Query> data(num_of_queries);

        for (int64_t i = 0; i < num_of_queries; ++i) {
            data[i] = ParseQuery(ReadLine());
        }


        for (Query& query : data) {
            if (query.type == QueryType::Stop) {
                base.AddStop(std::move(ParseStopInfo(query.data)));
            }
        }

        for (Query& query : data) {
            if (query.type == QueryType::Stop) {
                base.AddDistance(move(DistanceInfoInVector(query.data)));
            }
        }

        for (Query& query : data) {
            if (query.type == QueryType::Bus) {
                base.AddBus(std::move(ParseBusInfo(base, query.data)));
            }
        }


    }
}