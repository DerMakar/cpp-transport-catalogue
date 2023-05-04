#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

namespace json {
	using StopDistancesInfo = transport_base_processing::TransportCatalogue::StopDistancesInfo;
	using JsonStopDistanceMap = std::unordered_map<std::string, std::unordered_map<std::string, long unsigned int>>;
	class JsonBaseProcessing {
		public:
		JsonBaseProcessing(Document document) : document_(document) {
		}

		void CreateBase(transport_base_processing::TransportCatalogue& base);

		const renderer::RenderSettings& GetRenderSet() const; 

		const Array* GetStatRequest() const;
	
	private:
		Document document_;
		JsonStopDistanceMap stop_to_stop_distances;
		renderer::RenderSettings render_settings_;

		svg::Color ParseColor(const Node& color_collection) const;
		void ParseRenderSettings(const Dict* data);
		std::vector<transport_base_processing::Stop> ParseStopRequests(const Array* data);
		std::vector< transport_base_processing::Bus> ParseBusRequests(const Array* data, transport_base_processing::TransportCatalogue& base);
		StopDistancesInfo ParseStopDistInfo(std::string& stop);
		
	};
}