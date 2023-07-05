#pragma once

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <graph.pb.h>
#include <transport_router.pb.h>
#include "request_handler.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <variant>
#include <iterator>

void SaveBusStop(const transport_base_processing::TransportCatalogue& base, transport_base_serialize::TransportCatalogue& file);

void SaveColor(transport_base_serialize::Color* color, const svg::Color& setting_color);

void LoadColor(const transport_base_serialize::Color& base_color, svg::Color& result_color);

void SaveRenderSet(const transport_base_processing::RenderSettings& render_setting, transport_base_serialize::TransportCatalogue& file);

void SaveRouter(const transport_base_processing::RequestHandler& request_handler, transport_base_serialize::TransportCatalogue& file);

void MakeBase(const transport_base_processing::RequestHandler& request_handler
		, transport_base_serialize::TransportCatalogue& file);

void SerializeBase(const transport_base_serialize::TransportCatalogue& file, std::ofstream& out_file);

transport_base_processing::TransportCatalogue DiserializeTransportCatalogue(const transport_base_serialize::TransportCatalogue& base);

transport_base_processing::MapRenderer DiserializeMapRenderer(const transport_base_serialize::TransportCatalogue& base);

graph::Router<double> DiserializeRouter(const graph::DirectedWeightedGraph<double>& db_graph, const transport_base_serialize::TransportCatalogue& base);

transport_base_processing::TransportGraph DiserializeTransportGraph(const transport_base_serialize::TransportCatalogue& base);
