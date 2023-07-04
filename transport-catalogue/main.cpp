#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        MakeBase(std::cin);

    }
    else if (mode == "process_requests"sv) {

        ProcessRequest(std::cin);

    }
    else {
        PrintUsage();
        return 1;
    }
}