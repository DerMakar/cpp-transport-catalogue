#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "json_builder.h"
#include <sstream>



namespace json {
	using StopDistancesInfo = transport_base_processing::TransportCatalogue::StopDistancesInfo;
	using JsonStopDistanceMap = std::unordered_map<std::string, std::unordered_map<std::string, long unsigned int>>;
	class JsonBaseProcessing {
		public:
		JsonBaseProcessing(Document document) : document_(std::move(document)) {
		}

		void CreateBase(transport_base_processing::TransportCatalogue& base);

		const transport_base_processing::RenderSettings& GetRenderSet() const;

		Document GetStatRequest(const transport_base_processing::RequestHandler handler) const;
	
	private:
		Document document_;
		JsonStopDistanceMap stop_to_stop_distances;
		transport_base_processing::RenderSettings render_settings_;

		svg::Color ParseColor(const Node& color_collection) const;
		void ParseRenderSettings(const Dict* data);
		std::vector<transport_base_processing::Stop> ParseStopRequests(const Array* data);
		std::vector< transport_base_processing::Bus> ParseBusRequests(const Array* data, transport_base_processing::TransportCatalogue& base);
		void ParseRoutingSettings(const Dict* data, transport_base_processing::TransportCatalogue& base);
		StopDistancesInfo ParseStopDistInfo(std::string& stop);
		
	};
}