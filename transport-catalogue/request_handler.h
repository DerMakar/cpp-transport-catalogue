#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

namespace transport_base_processing {
    class RequestHandler {
    public:
        RequestHandler() = default;

        //RequestHandler(transport_base_processing::TransportCatalogue& base) : db_(base) {
        //    
        //}


        // MapRenderer понадобится в следующей части итогового проекта
        RequestHandler(const transport_base_processing::TransportCatalogue& db, const transport_base_processing::MapRenderer& renderer, const graph::Router<double>& db_router) : db_(db), renderer_(renderer), db_router_(db_router){

        }

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<transport_base_processing::BusInfo> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через
        const std::set<std::string>* GetBusesByStop(const std::string_view& stop_name) const;

        // Возвращает оптимальный маршрут от остановки from до остановки to
        std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

        const transport_base_processing::TransportCatalogue& GetBase() const;

        const transport_base_processing::MapRenderer& GetRenderSet() const;

        // Этот метод будет нужен в следующей части итогового проекта
        svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const transport_base_processing::TransportCatalogue& db_;
        const transport_base_processing::MapRenderer& renderer_;
        const graph::Router<double>& db_router_;
    };
} // namespace transport_base_processing