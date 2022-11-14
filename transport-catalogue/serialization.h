#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <map_renderer.pb.h>
#include <graph.pb.h>
#include <transport_router.pb.h>

#include <filesystem>
#include <optional>
#include <fstream>

namespace serialization {

	using RouteSettings = std::tuple<int, double>;
	
	transport_catalogue_serialize::TC CreateTC(const tc::TransportCatalogue& catalogue);

	render_settings_serialize::RenderSet CreateRenderer(const renderer::Settings& settings);

	router_serialize::Router CreateRouter(const graph::TransportRouter& rdb,
		const int waitTime, const double velocity);

	void SerilalizeData(const std::filesystem::path& path,
		const transport_catalogue_serialize::TC& obj_catalogue, 
		const render_settings_serialize::RenderSet& obj_rendSet,
		const router_serialize::Router& obj_router);

    std::optional<transport_catalogue_serialize::TCFull> Deserelization (const std::filesystem::path& path);

    std::optional<transport_catalogue_serialize::TC> DeserelizationTC(std::optional<transport_catalogue_serialize::TCFull> &tc_full);
    std::optional<render_settings_serialize::RenderSet> DeserelizationRenderer(std::optional<transport_catalogue_serialize::TCFull> &tc_full);
    std::optional<router_serialize::Router> DeserelizationRouter(std::optional<transport_catalogue_serialize::TCFull> &tc_full);

	void InitiliaziationTransportCatalogue(const std::optional<transport_catalogue_serialize::TC>& db,
		tc::TransportCatalogue& catalogue);
	void InitializationRenderSettings(const std::optional<render_settings_serialize::RenderSet>& sets_db,
		renderer::Settings& settings);
	void InitializationRouter(const std::optional<router_serialize::Router>& router_db,
		graph::TransportGraph& tr);

	void SetColorSetting(const svg_serialize::Color& c_data, renderer::Settings& settings);
	
}