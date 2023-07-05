#include "serialization.h"
#include "json_reader.h"

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
        transport_base_processing::TransportCatalogue base;
        json::Document test = json::Load(std::cin);
        json::JsonBaseProcessing json_base(test);
        json_base.CreateBase(base);
        const transport_base_processing::RenderSettings& render_setting = json_base.GetRenderSet();
        const std::filesystem::path path = json_base.GetFileName();
        std::ofstream out_file(path, std::ios::binary);
        transport_base_serialize::TransportCatalogue file;
        transport_base_processing::MapRenderer map_render(render_setting);
        transport_base_processing::RequestHandler request_handler(base, map_render);
        MakeBase(request_handler, file);
        SerializeBase(file, out_file);

    }
    else if (mode == "process_requests"sv) {
        json::Document test = json::Load(std::cin);
        json::JsonBaseProcessing json_base(test);
        json_base.ParseSerializationSet();
        const std::filesystem::path path = json_base.GetFileName();
        std::ifstream in_file(path, std::ios::binary);
        transport_base_serialize::TransportCatalogue base;
        if (!base.ParseFromIstream(&in_file)) {
            return 1;
        }
        transport_base_processing::TransportCatalogue db_result = DiserializeTransportCatalogue(base);
        transport_base_processing::TransportGraph db_graph = DiserializeTransportGraph(base);
        transport_base_processing::MapRenderer mr_result = DiserializeMapRenderer(base);
        graph::Router<double> db_router = DiserializeRouter(db_graph.GetGraph(), base);
        transport_base_processing::RequestHandler request_handler(db_result, mr_result, db_graph, db_router);
              
        Print(json_base.GetStatRequest(request_handler), std::cout);

    }
    else {
        PrintUsage();
        return 1;
    }
}



/*
Old version
#include "geo.h"
#include "json_reader.h"
#include "request_handler.h"

#include <string>
#include <iostream>
#include <cassert>
#include <sstream>


using namespace std::literals;



json::Document LoadJSON(const std::string& s) {
	std::istringstream strm(s);
	return json::Load(strm);
}

int main() {
	using namespace std;
    using namespace svg;
    
    transport_base_processing::TransportCatalogue base;
	json::Document test = json::Load(std::cin);
	json::JsonBaseProcessing input_json (test);
	input_json.CreateBase(base);
	transport_base_processing::MapRenderer map_render;
	map_render.SetRendSet(input_json.GetRenderSet());
	transport_base_processing::RequestHandler requests(base, map_render);
	Print(input_json.GetStatRequest(requests), std::cout);
    return 0;
}
*/

