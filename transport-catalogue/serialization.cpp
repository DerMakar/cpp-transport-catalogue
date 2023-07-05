#include "serialization.h"

void SaveBusStop(const transport_base_processing::TransportCatalogue& base, transport_base_serialize::TransportCatalogue& file) {
	using transport_base_processing::Bus;
	using transport_base_processing::Stop;

	for (const Bus& bus : base.GetBuses()) {
		transport_base_serialize::Bus new_bus;
		new_bus.set_name(bus.name);
		new_bus.set_is_circle(bus.is_circle);
		for (const Stop* stop : bus.route) {
			new_bus.add_route(stop->id);
		}
		*file.add_buses() = new_bus;
	}

	for (const Stop& stop : base.GetStops()) {
		transport_base_serialize::Stop new_stop;
		new_stop.set_name(stop.name);
		new_stop.set_lat(stop.coordinates.lat);
		new_stop.set_lng(stop.coordinates.lng);
		new_stop.set_id(stop.id);
		*file.add_stops() = new_stop;
	}

	for (const auto& [stop_pair, distance] : base.GetDistanceCollection()) {
		transport_base_serialize::Distance new_distance;
		new_distance.set_id_start(stop_pair.first->id);
		new_distance.set_id_finish(stop_pair.second->id);
		new_distance.set_distance(distance);
		*file.add_stop_to_distance() = new_distance;
	}
}

void SaveColor(transport_base_serialize::Color* color, const svg::Color& setting_color) {
	using namespace std::literals;
	if (std::holds_alternative<std::string>(setting_color)) {
		*color->mutable_color() = std::get<std::string>(setting_color);
	}
	else if ((std::holds_alternative<svg::Rgb>(setting_color))) {
		*color->mutable_color() = "rgb"s;
		const svg::Rgb& color_ = std::get<svg::Rgb>(setting_color);
		color->set_red(static_cast<int>(color_.red));
		color->set_green(static_cast<int>(color_.green));
		color->set_blue(static_cast<int>(color_.blue));
		color->set_opacity(1.0);
	}
	else if ((std::holds_alternative<svg::Rgba>(setting_color))) {
		*color->mutable_color() = "rgba"s;
		const svg::Rgba& color_ = std::get<svg::Rgba>(setting_color);
		color->set_red(static_cast<uint32_t>(color_.red));
		color->set_green(static_cast<uint32_t>(color_.green));
		color->set_blue(static_cast<uint32_t>(color_.blue));
		color->set_opacity(color_.opacity);
	}
	else {
		*color->mutable_color() = "none"s;
	}
}

void LoadColor(const transport_base_serialize::Color& base_color, svg::Color& result_color) {
	using namespace std::literals;
	std::string color_type = base_color.color();
	if (color_type == "rgb"s) {
		svg::Rgb color_;
		color_.red = static_cast<uint8_t>(base_color.red());
		color_.green = static_cast<uint8_t>(base_color.green());
		color_.blue = static_cast<uint8_t>(base_color.blue());
		result_color = color_;
	}
	else if (color_type == "rgba"s) {
		svg::Rgba color_;
		color_.red = base_color.red();
		color_.green = base_color.green();
		color_.blue = base_color.blue();
		color_.opacity = base_color.opacity();
		result_color = color_;
	}
	else {
		result_color = color_type;
	}
}

void SaveRenderSet(const transport_base_processing::RenderSettings& render_setting, transport_base_serialize::TransportCatalogue& file) {
	auto render_set_ptr = file.mutable_map_renderer()->mutable_renderer_data_();
	render_set_ptr->set_width(render_setting.width);
	render_set_ptr->set_height(render_setting.height);
	render_set_ptr->set_padding(render_setting.padding);
	render_set_ptr->set_line_width(render_setting.line_width);
	render_set_ptr->set_stop_radius(render_setting.stop_radius);
	render_set_ptr->set_bus_label_font_size(render_setting.bus_label_font_size);
	render_set_ptr->set_bus_label_font_size(render_setting.bus_label_font_size);
	render_set_ptr->mutable_bus_label_offset()->set_x(render_setting.bus_label_offset.x);
	render_set_ptr->mutable_bus_label_offset()->set_y(render_setting.bus_label_offset.y);
	render_set_ptr->set_stop_label_font_size(render_setting.stop_label_font_size);
	render_set_ptr->mutable_stop_label_offset()->set_x(render_setting.stop_label_offset.x);
	render_set_ptr->mutable_stop_label_offset()->set_y(render_setting.stop_label_offset.y);
	SaveColor(render_set_ptr->mutable_underlayer_color(), render_setting.underlayer_color);
	render_set_ptr->set_underlayer_width(render_setting.underlayer_width);
	for (int i = 0; i < render_setting.color_palette.size(); ++i) {
		SaveColor(render_set_ptr->add_color_palette(), render_setting.color_palette[i]);
	}
}

void SaveRouter(const transport_base_processing::RequestHandler& request_handler, transport_base_serialize::TransportCatalogue& file) {
	// добавление TransportRouter: TransportGraph, repeated Router, RoutingSet - в file
	transport_base_serialize::TransportRouter* ser_transport_router = file.mutable_transport_router();
	//
	// ссериализация TransportGraph: граф, вектор пересадок
	transport_base_serialize::TransportGraph* ser_transport_graf = ser_transport_router->mutable_transport_catalogue_graph();
	const transport_base_processing::TransportGraph& db_graf = request_handler.GetTransportGraph();
	// сериализация графа: вектор ребер, вектор вершин
	transport_base_serialize::DirectedWeightedGraph* ser_graf = ser_transport_graf->mutable_graph();
	// сериализация вектора вершин
	transport_base_serialize::IncidenceList incidence_lists;
	size_t db_graph_size = db_graf.GetGraph().GetVertexCount();
	for (size_t i = 0; i < db_graph_size; ++i) {
		ranges::Range db_incidence_list = db_graf.GetGraph().GetIncidentEdges(i);
		size_t length = std::distance(db_incidence_list.begin(), db_incidence_list.end());
		transport_base_serialize::IncidenceList ser_incidence_list;
		for (size_t m = 0; m < length; ++m) {
			ser_incidence_list.add_incidence_list(*(db_incidence_list.begin() + m));
		}
		*ser_graf->add_incidence_lists() = ser_incidence_list;
	}
	// сериализация вектора ребер
	size_t db_edges_cout = db_graf.GetGraph().GetEdgeCount();
	for (size_t i = 0; i < db_edges_cout; ++i) {
		const graph::Edge db_edge = db_graf.GetGraph().GetEdge(i);
		transport_base_serialize::Edge ser_edge;
		ser_edge.set_from(db_edge.from);
		ser_edge.set_to(db_edge.to);
		ser_edge.set_weight(db_edge.weight);
		*ser_graf->add_edges() = ser_edge;
	}
	
	// сериализация вектора пересадок
	size_t rout_info_size = db_graf.GetEdgeInfo().size();
	for (int i = 0; i < rout_info_size; ++i) {
		transport_base_serialize::RouteInfo ser_edge_info;
		ser_edge_info.set_bus_name(std::string(db_graf.GetEdgeInfo()[i].bus_name));
		ser_edge_info.set_spans(db_graf.GetEdgeInfo()[i].spans);
		*ser_transport_graf->add_route_infos() = ser_edge_info;
	}
	//
	//сериализация repeated Router: repeated RouteInternalData routes_internal_data_ 
	size_t db_internal_data_size = db_graf.GetGraph().GetVertexCount();
	for (size_t i = 0; i < db_internal_data_size; ++i) {
		transport_base_serialize::Router* router = ser_transport_router->add_transport_catalogue_router();
		for (size_t m = 0; m < db_internal_data_size; ++m) {
			transport_base_serialize::RouteInternalData ser_inter_data;
			if (request_handler.GetRouter().GetRouteInternalData(i, m).has_value()) {
				ser_inter_data.mutable_route_internal_data_opt()->set_weight(request_handler.GetRouter().GetRouteInternalData(i, m).value().weight);
				if (request_handler.GetRouter().GetRouteInternalData(i, m).value().prev_edge.has_value()) {
					ser_inter_data.mutable_route_internal_data_opt()->mutable_prev_edge()->
						set_prev_edge(request_handler.GetRouter().GetRouteInternalData(i, m).value().prev_edge.value());
				}
			}
			*router->add_routes_internal_data_() = ser_inter_data;
		}
	}
	
	//
	// сериализация RoutingSet: int32 bus_wait_time, double bus_velocity
	auto [wait_time, velocity] = request_handler.GetBase().GetWaitVelocityInfo();
	ser_transport_router->mutable_rouiting_set()->set_bus_wait_time(wait_time);
	const int meters_in_km = 1000;
	const int sec_in_min = 60;
	velocity = velocity * sec_in_min / meters_in_km;
	ser_transport_router->mutable_rouiting_set()->set_bus_velocity(velocity);
}


void MakeBase(const transport_base_processing::RequestHandler& request_handler,
			transport_base_serialize::TransportCatalogue& file) {
	SaveBusStop(request_handler.GetBase(), file);
	SaveRenderSet(request_handler.GetRenderSet().GetRendSet(), file);
	SaveRouter(request_handler, file);
}

void SerializeBase(const transport_base_serialize::TransportCatalogue& file, std::ofstream& out_file) {
	file.SerializeToOstream(&out_file);
}

transport_base_processing::TransportCatalogue DiserializeTransportCatalogue(const transport_base_serialize::TransportCatalogue& base) {
	using transport_base_processing::Stop;
	using transport_base_processing::Bus;

	transport_base_processing::TransportCatalogue result;

	for (int i = 0; i < base.stops_size(); ++i) {
		Stop new_stop;
		new_stop.name = base.stops(i).name();
		new_stop.coordinates.lat = base.stops(i).lat();
		new_stop.coordinates.lng = base.stops(i).lng();
		new_stop.id = base.stops(i).id();
		result.AddStop(new_stop);
	}

	for (int i = 0; i < base.stop_to_distance_size(); ++i) {
		int start_id = base.stop_to_distance(i).id_start();
		int finish_id = base.stop_to_distance(i).id_finish();
		const Stop* start_ptr = &result.GetStops()[start_id];
		const Stop* finish_ptr = &result.GetStops()[finish_id];
		long unsigned int distance = base.stop_to_distance(i).distance();
		result.AddDistanceToMap(start_ptr, finish_ptr, distance);
	}

	for (int i = 0; i < base.buses_size(); ++i) {
		Bus new_bus;
		new_bus.name = base.buses(i).name();
		new_bus.is_circle = base.buses(i).is_circle();
		new_bus.route.resize(base.buses(i).route_size());
		for (int m = 0; m < new_bus.route.size(); ++m) {
			const Stop* stop = &result.GetStops()[base.buses(i).route(m)];
			new_bus.route[m] = const_cast<Stop*>(stop);
		}
		result.AddBus(new_bus);
	}

	result.SetBusWaitTime(base.transport_router().rouiting_set().bus_wait_time());
	result.SetBusVelocity(base.transport_router().rouiting_set().bus_velocity());
	return result;
}


transport_base_processing::MapRenderer DiserializeMapRenderer(const transport_base_serialize::TransportCatalogue& base) {
	transport_base_processing::RenderSettings result;
	result.width = base.map_renderer().renderer_data_().width();
	result.height = base.map_renderer().renderer_data_().height();
	result.padding = base.map_renderer().renderer_data_().padding();
	result.line_width = base.map_renderer().renderer_data_().line_width();
	result.stop_radius = base.map_renderer().renderer_data_().stop_radius();
	result.bus_label_font_size = base.map_renderer().renderer_data_().bus_label_font_size();
	result.bus_label_offset.x = base.map_renderer().renderer_data_().bus_label_offset().x();
	result.bus_label_offset.y = base.map_renderer().renderer_data_().bus_label_offset().y();
	result.stop_label_font_size = base.map_renderer().renderer_data_().stop_label_font_size();
	result.stop_label_offset.x = base.map_renderer().renderer_data_().stop_label_offset().x();
	result.stop_label_offset.y = base.map_renderer().renderer_data_().stop_label_offset().y();
	LoadColor(base.map_renderer().renderer_data_().underlayer_color(), result.underlayer_color);
	result.underlayer_width = base.map_renderer().renderer_data_().underlayer_width();
	result.color_palette.resize(base.map_renderer().renderer_data_().color_palette().size());
	for (int i = 0; i < base.map_renderer().renderer_data_().color_palette().size(); ++i) {
		LoadColor(base.map_renderer().renderer_data_().color_palette(i), result.color_palette[i]);
	}
	return transport_base_processing::MapRenderer(std::move(result));
}

transport_base_processing::TransportGraph DiserializeTransportGraph(const transport_base_serialize::TransportCatalogue& base) {
	//выгружаем граф
	auto& f_graph_ptr = base.transport_router().transport_catalogue_graph().graph();
	// выгружает вектор ребер
	std::vector<graph::Edge<double>> edges;
	edges.resize(f_graph_ptr.edges_size());
	for (size_t i = 0; i < edges.size(); ++i) {
		graph::Edge<double> edge;
		edge.from = f_graph_ptr.edges(i).from();
		edge.to = f_graph_ptr.edges(i).to();
		edge.weight = f_graph_ptr.edges(i).weight();
		edges[i] = std::move(edge);
	}
	// выгружаем вектор вершин
	std::vector<std::vector<size_t>> incidence_lists;
	incidence_lists.resize (f_graph_ptr.incidence_lists_size());
	for (int i = 0; i < incidence_lists.size(); ++i) {
		std::vector<size_t> incidence_list(f_graph_ptr.incidence_lists(i).incidence_list_size());
		for (int m = 0; m < incidence_list.size(); ++m) {
			incidence_list[m] = f_graph_ptr.incidence_lists(i).incidence_list(m);
		}
		incidence_lists[i] = std::move(incidence_list);
	}
	graph::DirectedWeightedGraph<double> result_graph(std::move(edges), std::move(incidence_lists));

	// выгружаем информацию по маршрутам
	std::vector<transport_base_processing::RouteInfo> edge_info;
	auto& f_rout_info_ptr = base.transport_router().transport_catalogue_graph();
	edge_info.resize(f_rout_info_ptr.route_infos_size());
	for (int i = 0; i < edge_info.size(); ++i) {
		transport_base_processing::RouteInfo info;
		info.bus_name = f_rout_info_ptr.route_infos(i).bus_name();
		info.spans = f_rout_info_ptr.route_infos(i).spans();
		edge_info[i] = std::move(info);
	}

	transport_base_processing::TransportGraph result (std::move(result_graph), std::move(edge_info));

	return result;
}


graph::Router<double> DiserializeRouter(const graph::DirectedWeightedGraph<double>& db_graph, const transport_base_serialize::TransportCatalogue& base) {
	std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>> routes_internal_data_;
	routes_internal_data_.resize(base.transport_router().transport_catalogue_router_size());
	for (int i = 0; i < routes_internal_data_.size(); ++i) {
		std::vector<std::optional<graph::Router<double>::RouteInternalData>> route_internal_data_(base.transport_router().transport_catalogue_router(i).routes_internal_data__size());
		for (int m = 0; m < route_internal_data_.size(); ++m) {
			std::optional<graph::Router<double>::RouteInternalData> data;
			if (base.transport_router().transport_catalogue_router(i).routes_internal_data_(m).has_route_internal_data_opt()) {
				double weight = base.transport_router().transport_catalogue_router(i).routes_internal_data_(m).route_internal_data_opt().weight();
				data = { weight,std::nullopt};
				if (base.transport_router().transport_catalogue_router(i).routes_internal_data_(m).route_internal_data_opt().has_prev_edge()) {
					graph::EdgeId prev_edge = base.transport_router().transport_catalogue_router(i).routes_internal_data_(m).route_internal_data_opt().prev_edge().prev_edge();
					data = { weight,prev_edge };
				}
				
			}
			route_internal_data_[m] = data;
		}
		routes_internal_data_[i] = route_internal_data_;
	}
	bool internal_build_comd = false;
	graph::Router<double> result (db_graph, internal_build_comd);
	result.SetRoutesInternalData(std::move(routes_internal_data_));
	return result;
}