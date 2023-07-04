#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace transport_base_processing {
    class RequestHandler {
    public:
        RequestHandler() = default;

        //RequestHandler(transport_base_processing::TransportCatalogue& base) : db_(base) {
        //    
        //}


        // MapRenderer понадобится в следующей части итогового проекта
        RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer) : db_(db), renderer_(renderer), db_graf(std::move(TransportGraph(db))){

        }

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через
        const std::set<std::string>* GetBusesByStop(const std::string_view& stop_name) const;

        // Возвращает оптимальный маршрут от остановки from до остановки to
        std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

        const transport_base_processing::TransportCatalogue& GetBase() const;

        const transport_base_processing::MapRenderer& GetRenderSet() const;

        const TransportGraph& GetTransportGraph() const;

        const graph::Router<double>& GetRouter() const;

        // Этот метод будет нужен в следующей части итогового проекта
        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue& db_;
        const MapRenderer& renderer_;
        TransportGraph db_graf;
        graph::Router<double> db_router = db_graf.GetRouter();
    };
} // namespace transport_base_processing