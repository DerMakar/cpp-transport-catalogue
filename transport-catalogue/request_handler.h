#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"


class RequestHandler {
public:
    RequestHandler() = default;

    //RequestHandler(transport_base_processing::TransportCatalogue& base) : db_(base) {
    //    
    //}

    void JasonStatRequest(const json::Array*, std::ostream& out);
    
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transport_base_processing::TransportCatalogue& db, const renderer::MapRenderer& renderer) : db_(db), renderer_(renderer) {

    }

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<transport_base_processing::BusInfo> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::set<std::string>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_base_processing::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
