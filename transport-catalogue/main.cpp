
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
	requests.RenderMap().Render(cout);
    return 0;
}

/*
Ожидаемый вывод
<?xml version="1.0" encoding="UTF-8" ?>
<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
  <polyline points="99.2283,329.5 50,232.18 99.2283,329.5" fill="none" stroke="green" stroke-width="14" stroke-linecap="round" stroke-linejoin="round"/>
  <polyline points="550,190.051 279.22,50 333.61,269.08 550,190.051" fill="none" stroke="rgb(255,160,0)" stroke-width="14" stroke-linecap="round" stroke-linejoin="round"/>
</svg>
*/