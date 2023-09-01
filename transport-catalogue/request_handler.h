#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace transport_base_processing {
    class RequestHandler {
    public:
        RequestHandler() = default;

        RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer) : db_(db), renderer_(renderer), db_graf(std::move(TransportGraph(db))){
            db_router.SetRoutesInternalData(std::move(db_graf.GetRouter().GetInternalData()));
        }

        RequestHandler(const TransportCatalogue& db
            , const MapRenderer& renderer
            , const TransportGraph& graf
            , graph::Router<double> router)
            : db_(db)
            , renderer_(renderer)
            , db_graf(graf)
            , db_router (std::move(router)){
            
        }

        std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;

        const std::set<std::string>* GetBusesByStop(const std::string_view& stop_name) const;

        std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;

        const transport_base_processing::TransportCatalogue& GetBase() const;

        const transport_base_processing::MapRenderer& GetRenderSet() const;

        const TransportGraph& GetTransportGraph() const;

        const graph::Router<double>& GetRouter() const;

        svg::Document RenderMap() const;

    private:
        const TransportCatalogue& db_;
        const MapRenderer& renderer_;
        TransportGraph db_graf;
        graph::Router<double> db_router = graph::Router<double>(db_graf.GetGraph(), false);
    };
} // namespace transport_base_processing