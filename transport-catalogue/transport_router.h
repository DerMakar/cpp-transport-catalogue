#pragma once

#include "transport_catalogue.h"
#include "router.h"

namespace transport_base_processing {
	class TransportGraph {
	public:
		TransportGraph(const TransportCatalogue& base) {
			SetGraf(base);
		}

		graph::Router<double> GetRouter() const;

		const graph::DirectedWeightedGraph<double>& GetGraph() const;

		const std::vector<RouteInfo>& GetEdgeInfo() const;
		
	private:
		graph::DirectedWeightedGraph<double> transport_catalogue_graph;
		std::vector<RouteInfo> edge_info;

		const graph::DirectedWeightedGraph<double>& SetGraf(const TransportCatalogue& base);
		
	};

	class TransportRouter {
	public:
		TransportRouter(const TransportGraph& trasport_graph) : transport_catalogue_router(graph::Router<double>(trasport_graph.GetGraph())){
			
		}

    private:
		graph::Router<double> transport_catalogue_router;
	};

	
}