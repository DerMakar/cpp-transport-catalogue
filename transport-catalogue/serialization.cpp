#include "serialization.h"

void MakeBase(std::istream& input_json) {
	using transport_base_processing::Bus;
	using transport_base_processing::Stop;

	transport_base_processing::TransportCatalogue base;
	json::Document test = json::Load(input_json);
	json::JsonBaseProcessing json_base(test);
	json_base.CreateBase(base);
	const std::filesystem::path path = json_base.GetFileName();
	std::ofstream out_file(path, std::ios::binary);
	/*
	message TransportCatalogue {
	repeated Stop stops = 1;
	repeated Bus buses = 2;
	repeated Distance stop_to_distance = 3;
}
	*/
	transport_base_serialize::TransportCatalogue file;
	/*
	message Bus {
	string name = 1;
	repeated int32 route = 2;
	bool is_circle = 3;
}
	*/
	for (const Bus& bus : base.GetBuses()) {
		transport_base_serialize::Bus new_bus;
		new_bus.set_name(bus.name);
		new_bus.set_is_circle(bus.is_circle);
		for (const Stop* stop : bus.route) {
			new_bus.add_route(stop->id);
		}
		*file.add_buses() = new_bus;
	}
	/*
	message Stop{
	 string name = 1;
	 double lat = 2;
	 double lng = 3;	
	 int32 id = 4;
}
	*/
	for (const Stop& stop : base.GetStops()) {
		transport_base_serialize::Stop new_stop;
		new_stop.set_name(stop.name);
		new_stop.set_lat(stop.coordinates.lat);
		new_stop.set_lng(stop.coordinates.lng);
		new_stop.set_id(stop.id);
		*file.add_stops() = new_stop;
	}

	/*
	message Distance{
	int32 id_start = 1;
	int32 id_finish = 2;
	uint64 distance = 3;
	}
	*/
	for (const auto& [stop_pair, distance] : base.GetDistanceCollection()) {
		transport_base_serialize::Distance new_distance;
		new_distance.set_id_start(stop_pair.first->id);
		new_distance.set_id_finish(stop_pair.second->id);
		new_distance.set_distance(distance);
		*file.add_stop_to_distance() = new_distance;
	}
	file.SerializeToOstream(&out_file);
}


std::optional<transport_base_processing::TransportCatalogue> DiserializeBase(const std::filesystem::path& path) {
	using transport_base_processing::Stop;
	using transport_base_processing::Bus;
	transport_base_processing::TransportCatalogue result;
	std::ifstream in_file(path, std::ios::binary);
	transport_base_serialize::TransportCatalogue base;
	if (!base.ParseFromIstream(&in_file)) {
		return std::nullopt;
	}
	for (int i = 0; i < base.stops_size(); ++i) {
		Stop new_stop;
		new_stop.name = base.mutable_stops(i)->name();
		new_stop.coordinates.lat = base.mutable_stops(i)->lat();
		new_stop.coordinates.lng = base.mutable_stops(i)->lng();
		new_stop.id = base.mutable_stops(i)->id();
		result.AddStop(new_stop);
	}

	for (int i = 0; i < base.stop_to_distance_size(); ++i) {
		int start_id = base.mutable_stop_to_distance(i)->id_start();
		int finish_id = base.mutable_stop_to_distance(i)->id_finish();
		const Stop* start_ptr = &result.GetStops()[start_id];
		const Stop* finish_ptr = &result.GetStops()[finish_id];
		long unsigned int distance = base.mutable_stop_to_distance(i)->distance();
		result.AddDistanceToMap(start_ptr, finish_ptr, distance);
	}

	for (int i = 0; i < base.buses_size(); ++i) {
		Bus new_bus;
		new_bus.name = base.mutable_buses(i)->name();
		new_bus.is_circle = base.mutable_buses(i)->is_circle();
		new_bus.route.resize(base.mutable_buses(i)->route_size());
		for (int m = 0; m < new_bus.route.size(); ++m) {
			const Stop* stop = &result.GetStops()[base.mutable_buses(i)->route(m)];
			new_bus.route[m] = const_cast<Stop*>(stop);
		}
		result.AddBus(new_bus);
	}
	return std::move(result);
}

void ProcessRequest(std::istream& input_json) {
	json::Document test = json::Load(input_json);
	json::JsonBaseProcessing json_base(test);
	json_base.ParseSerializationSet();
	const std::filesystem::path path = json_base.GetFileName();
	std::optional<transport_base_processing::TransportCatalogue> transport_catalogue = DiserializeBase(path);
	if (!transport_catalogue) {
		return;
	}
	transport_base_processing::MapRenderer map_render;
	transport_base_processing::RequestHandler requests(transport_catalogue.value(), map_render);
	
	Print(json_base.GetStatRequest(requests), std::cout);
}
