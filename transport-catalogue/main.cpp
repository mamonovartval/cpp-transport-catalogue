#include "serialization.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

#include "graph.h"
#include "router.h"
#include "transport_router.h"

#include <fstream>
#include <iostream>
#include <string_view>
#include <filesystem>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

		// create Setting object
		renderer::Settings settings;
		reader::JsonReader jr;
		const auto& [queryAdd, queryTowardStop, queryReq, routeSettings, nameBase] = jr.ReadData(std::cin, settings);
		// declare transport catalogue object
		tc::TransportCatalogue catalogue;
		// threading queries to add stop
		for (auto& query : queryAdd) {
			if (query.typeOfQuery == "Stop"s) {
				catalogue.AddStopToBase(query.nameStop, query.latitude, query.longitude);
			}
		}
		// threading queries to add stop distances
		for (auto& query : queryTowardStop) {
			for (auto& [from, vec_to] : query) {
				// get ptr to head stop
				const domain::Stop* ptr_from = catalogue.SearchStop(from);
				//get ptr for stop from head
				// and set distance in transport catalogue
				for (const auto& data : vec_to) {
					// set distance
					const domain::Stop* ptr_to = catalogue.SearchStop(data.to);
					catalogue.SetDistanceBetweenStops(std::make_pair(ptr_from, ptr_to), data.distance);
				}
			}
		} // end thereading queries
		// threading queries to add route
		for (auto& query : queryAdd) {
			if (query.typeOfQuery == "Bus"s) {
				catalogue.AddRouteToBase(query.nameBus, query.routeStops, query.isRing);
			}
		}

		// fill transport graph
		graph::TransportGraph tr(catalogue, routeSettings.velocity, routeSettings.waitTime);

		// init router by graph
		graph::TransportRouter rdb(tr);

		const std::filesystem::path path = nameBase;

		serialization::SerilalizeData(path,
			serialization::CreateTC(catalogue),
			serialization::CreateRenderer(settings),
			serialization::CreateRouter(rdb, routeSettings.waitTime, routeSettings.velocity));

    } else if (mode == "process_requests"sv) {

		// declare Setting object
		renderer::Settings settings;
		reader::JsonReader jr;
		const auto& [queryAdd, queryTowardStop, queryReq, routeSettings, nameBase] = jr.ReadData(std::cin, settings);
		// assign name for path to database
		const std::filesystem::path path = nameBase;

		std::optional<transport_catalogue_serialize::TCFull> tc_full = serialization::Deserelization(path);

		// make deserilization for trasport catalog
		std::optional<transport_catalogue_serialize::TC> deserializedTC = serialization::DeserelizationTC(tc_full);
		// make deserilization for renderer settings
		std::optional<render_settings_serialize::RenderSet> deserializedRenderer = serialization::DeserelizationRenderer(tc_full);
		// make deserelization for router
		std::optional<router_serialize::Router> deserializedRouter = serialization::DeserelizationRouter(tc_full);
		// declare transport catalogue object
		tc::TransportCatalogue catalogue_db;
		// make initialization transport catalog object by means db from file
		serialization::InitiliaziationTransportCatalogue(deserializedTC, catalogue_db);

		// getting render settings by means db from file
		serialization::InitializationRenderSettings(deserializedRenderer, settings);

		// declare transport graph
		graph::TransportGraph tr_db;
		// make initialization graph by means db from file
		serialization::InitializationRouter(deserializedRouter, tr_db);

		// define init data for renderer
		const auto& stops = catalogue_db.GetSortedStops();
		// initiliasitaion map by points all of them stops
		renderer::MapRenderer render(stops.begin(), stops.end(), settings.width,
			settings.height, settings.padding);
		// save settings in map renderer
		render.SaveSettings(settings);

		// init routerdata base
		graph::TransportRouter rdb(tr_db);

		// init request handler
		handler::RequestHandler reqHandler(catalogue_db, render, rdb);

		// threading queries to get
		jr.GetData(std::cout, reqHandler, queryReq);

    } else {
        PrintUsage();
        return 1;
    }
}