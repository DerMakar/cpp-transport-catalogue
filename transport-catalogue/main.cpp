#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

/*
В этом и последующих заданиях нужно разделить программу, которая отвечает за наполнение транспортного справочника и обработку запросов к нему, на две:
Программа make_base: создание базы транспортного справочника по запросам base_requests и её сериализация в файл.
Программа process_requests: десериализация базы из файла и использование её для ответов на запросы stat_requests.
 В этой задаче требуется сериализация лишь той части базы, которая нужна для ответа на запросы Bus и Stop.
 Программа make_base
На вход программе make_base через стандартный поток ввода подаётся JSON со следующими ключами: base_requests, routing_settings, 
render_settings, serialization_settings - настройки сериализации. На текущий момент это словарь с единственным ключом file, 
которому соответствует строка — название файла. Именно в этот файл нужно сохранить сериализованную базу.
Задача программы make_base — построить базу и сериализовать её в файл с указанным именем. Выводить что-либо в стандартный поток вывода не требуется.

Программа process_requests
На вход программе process_requests подаётся файл с сериализованной базой (результат работы make_base), 
а также — через стандартный поток ввода — JSON со следующими ключами:
stat_requests: запросы Bus и Stop к готовой базе.
serialization_settings: настройки сериализации в формате, аналогичном этой же секции на входе make_base. А именно, в ключе file
указывается название файла, из которого нужно считать сериализованную базу.
Программа process_requests должна вывести JSON с ответами на запросы — в том же формате, что и в предыдущих частях.
*/

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

