#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

#include "graph.h"
#include "router.h"
#include "transport_router.h"

#include <string>
#include <vector>
#include <fstream>
#include <utility>

#include <Windows.h>

using namespace std::literals;

int main() {
	//SetConsoleCP(1251);
	//SetConsoleOutputCP(1251);
	// read data from input
  	std::fstream input("InputJSON.txt");
	// create Setting object
	renderer::Settings settings;
	reader::JsonReader jr;
	//const auto&[queryAdd, queryTowardStop, queryReq, routeSettings] = jr.ReadData(std::cin, settings);
	const auto&[queryAdd, queryTowardStop, queryReq, routeSettings] = jr.ReadData(input, settings);
	
	tc::TransportCatalogue catalogue;
	// threading queries to add stop
	for (auto& query : queryAdd) {
		if (query.typeOfQuery == "Stop"s) {
			catalogue.AddStopToBase(query.nameStop, query.latitude, query.longitude);
		}
	}
	// threading queries to add stop distances
	for (auto& query : queryTowardStop) {
		for (auto&[from, vec_to] : query) {
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
	// define init data for renderer
	const auto& stops = catalogue.GetStops();
	// initiliasitaion map by points all of them stops
	renderer::MapRenderer render(stops.begin(), stops.end(), settings.width,
		settings.height, settings.padding);
	// save settings in map renderer
	render.SaveSettings(settings);

	// fill transport graph
	graph::TransportGraph tr(catalogue, routeSettings.velocity, routeSettings.waitTime);
	// init routerdata base
	graph::TransportRouter rdb(tr);
	
	// init request handler
	handler::RequestHandler reqHandler(catalogue, render, rdb);
		
	// threading queries to get
	jr.GetData(std::cout, reqHandler, queryReq);

	return 0;
}