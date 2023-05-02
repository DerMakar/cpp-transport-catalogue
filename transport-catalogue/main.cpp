
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
	transport_base_processing::TransportCatalogue base;
		
	json::Document test = json::Load(std::cin);

	json::JsonBaseProcessing input_json (test);

	input_json.CreateBase(base);

	RequestHandler requests(base);

	requests.JasonStatRequest(input_json.GetStatRequest(), std::cout);
	
	return 0;
}
