
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
/*
В stdout программа должна вывести SVG-изображение карты. Обрабатывать содержимое ключа stat_requests и выводить JSON с ответами на запросы к транспортному справочнику не нужно.
*/
int main() {
	using namespace std;
    using namespace svg;
    
    transport_base_processing::TransportCatalogue base;
	json::Document test = json::Load(std::cin);
	json::JsonBaseProcessing input_json (test);
	input_json.CreateBase(base);
	renderer::MapRenderer map_render;
	map_render.SetRendSet(input_json.GetRenderSet());
	RequestHandler requests(base, map_render);
	requests.JasonStatRequest(input_json.GetStatRequest(), std::cout);
    return 0;
}

