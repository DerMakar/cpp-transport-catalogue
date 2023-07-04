#pragma once

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <graph.pb.h>
#include <transport_router.pb.h>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "ranges.h"
#include "graph.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <variant>
#include <iterator>

void SaveBusStop(const transport_base_processing::TransportCatalogue& base, transport_base_serialize::TransportCatalogue& file);

void SaveColor(transport_base_serialize::Color* color, const svg::Color& setting_color);

void LoadColor(const transport_base_serialize::Color& base_color, svg::Color& result_color);

void SaveRenderSet(const json::JsonBaseProcessing& json_base, transport_base_serialize::TransportCatalogue& file);

void SaveRouter(const transport_base_processing::RequestHandler& request_handler, transport_base_serialize::TransportCatalogue& file);

void MakeBase(std::istream& input_json);

transport_base_processing::TransportCatalogue DiserializeTransportCatalogue(const transport_base_serialize::TransportCatalogue& base);
transport_base_processing::MapRenderer DiserializeMapRenderer(const transport_base_serialize::TransportCatalogue& base);

void ProcessRequest(std::istream& input_json);
