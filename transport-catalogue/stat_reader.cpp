#include "stat_reader.h"
#include "transport_catalogue.h"
#include <iostream>

namespace transport_base_processing {
    using namespace std::literals;

    void AskBase(bus_stop_processing::TransportCatalogue& base) {
        int num_of_queries = transport_base_processing::ReadLineWithNumber();
        for (int i = 0; i != num_of_queries; ++i) {
            std::string query = transport_base_processing::ReadLine();
            auto pos = query.find(' ');
            std::string name = query.substr(pos + 1);
            if (query[0] == 'B') {
                if (base.FindBus(move(name)) == nullptr) {
                    std::cout << query << ": not found"s << std::endl;
                }
                else {
                    std::cout << query << ": "s << base.GetBusInfo(name) << std::endl;
                }
            }
            else if (query[0] == 'S') {
                if (base.FindStop(name) == nullptr) {
                    std::cout << query << ": not found"s << std::endl;
                }
                else if (base.GetStopInfo(name) == nullptr) {
                    std::cout << query << ": no buses" << std::endl;
                }
                else {
                    std::cout << query << ": buses"s;
                    for (const std::string& bus : *base.GetStopInfo(name)) {
                        std::cout << " "s << bus;
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
}