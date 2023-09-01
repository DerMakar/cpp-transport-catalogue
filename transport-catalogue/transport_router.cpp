#include "transport_router.h"


namespace transport_base_processing {
   
    const graph::DirectedWeightedGraph<double>& TransportGraph::SetGraf(const TransportCatalogue& base) {
        transport_catalogue_graph = graph::DirectedWeightedGraph<double>(base.GetStops().size());
        graph::EdgeId cur_edge_id = 0;
        for (const auto& bus : base.GetBuses()) {
            auto cur_stop = bus.route.begin();
            while (cur_stop < bus.route.end() - 1) {
                bool IsFirst = true;
                size_t from = (*cur_stop)->id;
                int span_counter = 0;
                for (auto it = cur_stop + 1; it < bus.route.end(); ++it) {
                    size_t to = (*it)->id;
                    double distance = 0.0;
                    if (base.GetDistanceCollection().count(std::make_pair(*(prev(it)), *it)) != 0) {
                        distance = base.GetDistanceCollection().at(std::make_pair(*(prev(it)), *it));
                    }
                    else {
                        distance = base.GetDistanceCollection().at(std::make_pair(*it, *(prev(it))));
                    }
                    graph::Edge<double> edge = { from, to, distance / base.GetWaitVelocityInfo().second};
                    if (IsFirst) {
                        edge.weight += base.GetWaitVelocityInfo().first;
                        IsFirst = false;
                    }
                    else {
                        edge.weight += transport_catalogue_graph.GetEdge(cur_edge_id).weight;
                    }
                    cur_edge_id = transport_catalogue_graph.AddEdge(edge);
                    edge_info.push_back({ bus.name, ++span_counter });
                }
                ++cur_stop;
            }
        }
        return transport_catalogue_graph;
    }

    graph::Router<double> TransportGraph::GetRouter() const {
        return std::move(graph::Router<double>(transport_catalogue_graph, true));
    }

    const graph::DirectedWeightedGraph<double>& TransportGraph::GetGraph() const {
        return transport_catalogue_graph;
    }

    const std::vector<RouteInfo>& TransportGraph::GetEdgeInfo() const {
        return edge_info;
    }
} //namespace transport_base_processing