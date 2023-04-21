#include "input_reader.h"
#include "geo.h"
#include "stat_reader.h"
#include <string>
#include <iostream>
#include <cassert>


using namespace std::literals;

int main() {
	transport_base_processing::TransportCatalogue base;
	
	transport_base_processing::CreateBase(base);
	
	transport_base_processing::AskBase(base);
	
	return 0;
}