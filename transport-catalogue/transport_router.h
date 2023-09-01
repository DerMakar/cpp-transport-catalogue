#pragma once

#include "transport_catalogue.h"
#include "router.h"

namespace transport_base_processing {
	class TransportGraph {
	public:
		TransportGraph(const TransportCatalogue& base) {
			SetGraf(base);
		}

		TransportGraph(graph::DirectedWeightedGraph<double>&& graph, std::vector<RouteInfo>&& info) : transport_catalogue_graph(std::move(graph)), edge_info(std::move(info)) {
		}

		graph::Router<double> GetRouter() const;

		const graph::DirectedWeightedGraph<double>& GetGraph() const;

		const std::vector<RouteInfo>& GetEdgeInfo() const;
		
	private:
		graph::DirectedWeightedGraph<double> transport_catalogue_graph;
		std::vector<RouteInfo> edge_info;
		const graph::DirectedWeightedGraph<double>& SetGraf(const TransportCatalogue& base);
	};
}