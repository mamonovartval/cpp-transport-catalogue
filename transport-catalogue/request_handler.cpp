#include "request_handler.h"
#include "map_renderer.h"

#include <algorithm>
#include <map>


namespace handler
{
	/************Constructor*************/
	RequestHandler::RequestHandler(const tc::TransportCatalogue & db,
		const renderer::MapRenderer & renderer,
		const graph::TransportRouter& tr)	:db_(db), renderer_(renderer), rdb_(tr) {}
	/*************************************/

	std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view & bus_name) const
	{
		const domain::Bus* bus = db_.SearchRoute(bus_name);

		if (bus == nullptr) {
			return std::nullopt;
		}

		return std::optional<BusStat>({ bus->numStops, bus->numUniqueStops,
			bus->lengthRoute, bus->curvature });
	}

	const std::unordered_set<BusPtr>* RequestHandler::GetBusesByStop(const std::string_view & stop_name) const
	{
		return db_.GetStopToBuses(stop_name);
	}
	
	svg::Document RequestHandler::RenderMap() const
	{
		return renderer_.GetMap(db_.GetSortedBuses());
	}

	const graph::TransportRouter& RequestHandler::GetRouter() const
	{
		return rdb_;
	}
}