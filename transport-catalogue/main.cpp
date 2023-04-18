#include "input_reader.h"
#include "geo.h"
#include "stat_reader.h"
#include <string>
#include <iostream>
#include <cassert>


using namespace std::literals;

int main() {
	TransportCatalogue base;
	
		CreateBase(base);
	
		AskBase(base);
	
	return 0;
}