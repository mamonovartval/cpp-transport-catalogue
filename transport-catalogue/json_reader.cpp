#include "json_reader.h"
#include "json_builder.h"
#include "map_renderer.h"

#include <algorithm> 
#include <cctype>
#include <locale>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>
#include <string_view>
#include <unordered_map>
#include <optional>

namespace reader
{
	/*constants*/
	static const unsigned int numStops = 100;
	static const std::string baseReq{ "base_requests"s };
	static const std::string rendSet{ "render_settings"s };
	static const std::string statReq{ "stat_requests"s };
	static const std::string routeSet{ "routing_settings"s };
	static const std::string serialSet{ "serialization_settings"s };
	static const std::string type{ "type"s };
	static const std::string name{ "name"s };
	static const std::string stops{ "stops"s };
	static const std::string isRoundRoute{ "is_roundtrip"s };
	static const std::string lat{ "latitude"s };
	static const std::string lng{ "longitude"s };
	static const std::string dist{ "road_distances"s };
	static const std::string id{ "id"s };
	static const std::string velocity{ "bus_velocity"s };
	static const std::string time{ "bus_wait_time"s };
	static const std::string from{ "from"s };
	static const std::string to{ "to"s };

	/******************************Read***********************************/
	std::vector<std::string> JsonReader::GetStopsRoute(json::Node& input) {
		// getting list of stops
		std::vector<std::string> result;

		// getting name of stop
		if (input.AsDict().count(stops) && input.AsDict().at(stops).IsArray()) {
			for (auto nameStop : input.AsDict().at(stops).AsArray()) {
				// put data to result
				if (nameStop.IsString()) {
					result.push_back(nameStop.AsString());
				}
			}
		}
		else {
			return result;
		}
		return result;
	}

	std::vector<detail::Distance> JsonReader::GetStopsDistance(json::Node& input) {

		std::vector<detail::Distance> result;

		if (input.AsDict().count(dist) && input.AsDict().at(dist).IsDict()) {

			for (auto&[toStop, distance] : input.AsDict().at(dist).AsDict()) {
				detail::Distance data;
				data.to = toStop;

				if (distance.IsInt()) {
					data.distance = distance.AsInt();
				}

				result.push_back(data);
			}
		}
		else {
			return result;
		}

		return result;
	}

	ResponseAddQuery JsonReader::ReadAddQuery(const json::Node& input) {

		std::deque<detail::Query> queriesToAdd;
		std::vector<detail::Query> queryStops;
		std::vector<detail::Query> queryBuses;

		json::Array arr_data = input.AsArray();

		size_t reqNum = arr_data.size();
		// divide data to query for add to base into to query for stops and buses
		for (int i = 0; i < reqNum; ++i) {
			detail::Query result;
			// read line
			if (arr_data[i].IsDict() && arr_data[i].AsDict().empty()) {
				continue;
			}
			json::Node currReq = arr_data[i];
			// getting type of query
			if (currReq.AsDict().count(type) && currReq.AsDict().at(type).IsString()) {
				result.typeOfQuery = currReq.AsDict().at(type).AsString();
			}

			if (result.typeOfQuery == "Stop"s) {
				// read nameStop
				if (currReq.AsDict().count(name) && currReq.AsDict().at(name).IsString()) {
					result.nameStop = currReq.AsDict().at(name).AsString();
				}
				// read latitude
				if (currReq.AsDict().count(lat) && !(currReq.AsDict().at(lat).IsDouble())) {
					if (currReq.AsDict().at(lat).IsInt()) {
						result.latitude = currReq.AsDict().at(lat).IsInt();
					}
				}
				else {
					result.latitude = currReq.AsDict().at(lat).AsDouble();
				}
				//read longitude
				if (currReq.AsDict().count(lng) && !(currReq.AsDict().at(lng).IsDouble())) {
					if (currReq.AsDict().at(lng).IsInt()) {
						result.longitude = currReq.AsDict().at(lng).AsInt();
					}
				}
				else {
					result.longitude = currReq.AsDict().at(lng).AsDouble();
				}
				// put data to query result vector
				queryStops.push_back(result);
			}
			else if (result.typeOfQuery == "Bus"s) {
				//read nameBus
				if (currReq.AsDict().count(name) && currReq.AsDict().at(name).IsString()) {
					result.nameBus = currReq.AsDict().at(name).AsString();
				}
				// read route Stops
				result.routeStops = GetStopsRoute(currReq);
				// read route is ring
				if (currReq.AsDict().count(isRoundRoute) && currReq.AsDict().at(isRoundRoute).IsBool()) {
					result.isRing = currReq.AsDict().at(isRoundRoute).AsBool();
				}

				// put data to query result vector
				queryBuses.push_back(result);
			}
		}
		// collect common query: in first - stops, after buses
		for (auto& stop : queryStops) {
			queriesToAdd.push_back(stop);
		}
		for (auto& bus : queryBuses) {
			queriesToAdd.push_back(bus);
		}
		return queriesToAdd;
	}

	ResponseAddTowardStop JsonReader::ReadAddTowardStop(const json::Node & input)
	{
		std::deque<std::unordered_map<std::string, std::vector<detail::Distance>>> queryTowardStop;

		json::Array arr_data = input.AsArray();

		size_t reqNum = arr_data.size();
		// divide data to query for add to base into to query for stops and buses
		for (int i = 0; i < reqNum; ++i) {
			detail::Query result;
			// read line
			if (arr_data[i].IsDict() && arr_data[i].AsDict().empty()) {
				continue;
			}
			json::Node currReq = arr_data[i];
			// getting type of query
			if (currReq.AsDict().count(type) && currReq.AsDict().at(type).IsString()) {
				result.typeOfQuery = currReq.AsDict().at(type).AsString();
			}

			if (result.typeOfQuery == "Stop"s) {

				if (currReq.AsDict().count(name) && currReq.AsDict().at(name).IsString()) {
					result.nameStop = currReq.AsDict().at(name).AsString();
				}

				std::vector<detail::Distance> vecData = GetStopsDistance(currReq);

				if (!(vecData.empty())) {
					std::unordered_map<std::string, std::vector<detail::Distance>> data;
					data[result.nameStop] = vecData;
					queryTowardStop.push_back(data);
				}
			}
		}

		return queryTowardStop;
	}

	std::deque<detail::Query> JsonReader::ReadGetQuery(const json::Node& input) {

		std::deque<detail::Query> queriesToBase;

		json::Array arr_data = input.AsArray();

		size_t reqNum = arr_data.size();
		// collect data for query of request to base
		for (int i = 0; i < reqNum; ++i) {
			detail::Query result;

			if (arr_data[i].IsDict() && arr_data[i].AsDict().empty()) {
				continue;
			}
			json::Node currReq = arr_data[i];

			// getting type of query
			if (currReq.AsDict().count(type) && currReq.AsDict().at(type).IsString()) {
				result.typeOfQuery = currReq.AsDict().at(type).AsString();
			}

			// getting number of bus
			if (result.typeOfQuery == "Bus"sv) {

				if (currReq.AsDict().count(id) && currReq.AsDict().at(id).IsInt()) {
					result.id_query = currReq.AsDict().at(id).AsInt();
				}

				if (currReq.AsDict().count(name) && currReq.AsDict().at(name).IsString()) {
					result.nameBus = currReq.AsDict().at(name).AsString();
				}
			}
			else if (result.typeOfQuery == "Map"sv) {

				if (currReq.AsDict().count(id) && currReq.AsDict().at(id).IsInt()) {
					result.id_query = currReq.AsDict().at(id).AsInt();
				}
			}
			else if (result.typeOfQuery == "Route"sv) {

				if (currReq.AsDict().count(id) && currReq.AsDict().at(id).IsInt()) {
					result.id_query = currReq.AsDict().at(id).AsInt();
				}
				if (currReq.AsDict().count(from) && currReq.AsDict().at(from).IsString()) {
					result.from = currReq.AsDict().at(from).AsString();
				}
				if (currReq.AsDict().count(to) && currReq.AsDict().at(to).IsString()) {
					result.to = currReq.AsDict().at(to).AsString();
				}
			}
			else {
				if (currReq.AsDict().count(id) && currReq.AsDict().at(id).IsInt()) {
					result.id_query = currReq.AsDict().at(id).AsInt();
				}

				if (currReq.AsDict().count(name) && currReq.AsDict().at(name).IsString()) {
					result.nameStop = currReq.AsDict().at(name).AsString();
				}
			}
			// push data to container	
			queriesToBase.push_back(result);
		}
		return queriesToBase;
	}

	void JsonReader::ReadRenderQuery(const json::Dict & dict_sets, renderer::Settings& setup)
	{
		// save in structure
		if (dict_sets.at("width"s).IsDouble()) {
			setup.width = dict_sets.at("width"s).AsDouble();
		}
		if (dict_sets.at("height"s).IsDouble()) {
			setup.height = dict_sets.at("height"s).AsDouble();
		}
		if (dict_sets.at("padding"s).IsDouble()) {
			setup.padding = dict_sets.at("padding"s).AsDouble();
		}
		if (dict_sets.at("line_width"s).IsDouble()) {
			setup.line_width = dict_sets.at("line_width"s).AsDouble();
		}
		if (dict_sets.at("stop_radius"s).IsDouble()) {
			setup.stop_radius = dict_sets.at("stop_radius"s).AsDouble();
		}
		if (dict_sets.at("bus_label_font_size"s).IsInt()) {
			setup.bus_label_font_size = dict_sets.at("bus_label_font_size"s).AsInt();
		}
		if (dict_sets.at("bus_label_offset"s).IsArray()) {
			std::size_t sizeArr_bus = dict_sets.at("bus_label_offset"s).AsArray().size();
			for (std::size_t i = 0; i < sizeArr_bus; i++) {
				if (dict_sets.at("bus_label_offset"s).AsArray()[i].IsDouble()) {
					setup.bus_label_offset.push_back(dict_sets.at("bus_label_offset"s).AsArray()[i].AsDouble());
				}
			}
		}
		if (dict_sets.at("stop_label_font_size"s).IsInt()) {
			setup.stop_label_font_size = dict_sets.at("stop_label_font_size"s).AsInt();
		}
		if (dict_sets.at("stop_label_offset"s).IsArray()) {
			std::size_t sizeArr_stop = dict_sets.at("stop_label_offset"s).AsArray().size();
			for (std::size_t i = 0; i < sizeArr_stop; i++) {
				if (dict_sets.at("stop_label_offset"s).AsArray()[i].IsDouble()) {
					setup.stop_label_offset.push_back(dict_sets.at("stop_label_offset"s).AsArray()[i].AsDouble());
				}
			}
		}
		/*****************underlayer_color***************/
		if (dict_sets.at("underlayer_color"s).IsArray()
			|| dict_sets.at("underlayer_color"s).IsString()) {

			// define color by one word? like "green" or "red"
			if (dict_sets.at("underlayer_color"s).IsString())
			{
				setup.underlayer_color = { dict_sets.at("underlayer_color"s).AsString() };
			}
			else if (dict_sets.at("underlayer_color"s).IsArray())
			{
				std::size_t sizeArr = dict_sets.at("underlayer_color"s).AsArray().size();
				// define which type of RGB 
				if (sizeArr == 3) {

					std::uint8_t int_11 = dict_sets.at("underlayer_color"s).AsArray()[0].AsInt();
					std::uint8_t int_21 = dict_sets.at("underlayer_color"s).AsArray()[1].AsInt();
					std::uint8_t int_31 = dict_sets.at("underlayer_color"s).AsArray()[2].AsInt();

					setup.underlayer_color = svg::Rgb{ int_11, int_21, int_31 };
				}
				// or RGBa
				else if (sizeArr == 4)
				{
					std::uint8_t int_1 = dict_sets.at("underlayer_color"s).AsArray()[0].AsInt();
					std::uint8_t int_2 = dict_sets.at("underlayer_color"s).AsArray()[1].AsInt();
					std::uint8_t int_3 = dict_sets.at("underlayer_color"s).AsArray()[2].AsInt();
					auto dbl_4 = dict_sets.at("underlayer_color"s).AsArray()[3].AsDouble();

					setup.underlayer_color = svg::Rgba{ int_1, int_2, int_3, dbl_4 };
				}
			}
		}

		/****************underlayer_width****************/
		if (dict_sets.at("underlayer_width"s).IsDouble()) {
			setup.underlayer_width = dict_sets.at("underlayer_width"s).AsDouble();
		}
		/****************read color_palette**************/
		if (dict_sets.at("color_palette"s).IsArray()) {
			std::size_t sizeArr_color = dict_sets.at("color_palette"s).AsArray().size();
			for (std::size_t i = 0; i < sizeArr_color; i++) {

				// define color by one word? like "green" or "red"
				if (dict_sets.at("color_palette"s).AsArray()[i].IsString())
				{
					setup.color_palette.push_back(dict_sets.at("color_palette"s).AsArray()[i].AsString());
				}
				else if (dict_sets.at("color_palette"s).AsArray()[i].IsArray())
				{
					std::size_t sizeArr = dict_sets.at("color_palette"s).AsArray()[i].AsArray().size();
					// define which type of RGB 
					if (sizeArr == 3) {

						std::uint8_t int_11 = dict_sets.at("color_palette"s).AsArray()[i].AsArray()[0].AsInt();
						std::uint8_t int_21 = dict_sets.at("color_palette"s).AsArray()[i].AsArray()[1].AsInt();
						std::uint8_t int_31 = dict_sets.at("color_palette"s).AsArray()[i].AsArray()[2].AsInt();

						setup.color_palette.push_back(svg::Rgb{ int_11, int_21, int_31 });
					}
					// or RGBa
					else if (sizeArr == 4)
					{
						std::uint8_t int_1 = dict_sets.at("color_palette"s).AsArray()[i].AsArray()[0].AsInt();
						std::uint8_t int_2 = dict_sets.at("color_palette"s).AsArray()[i].AsArray()[1].AsInt();
						std::uint8_t int_3 = dict_sets.at("color_palette"s).AsArray()[i].AsArray()[2].AsInt();
						auto dbl_4 = dict_sets.at("color_palette"s).AsArray()[i].AsArray()[3].AsDouble();

						setup.color_palette.push_back(svg::Rgba{ int_1, int_2, int_3, dbl_4 });
					}
					else
					{
						continue;
					}
				}
				else {
					continue;
				}


			}
		}
	}

	detail::RouteSet JsonReader::ReadRoutingQuery(const json::Dict & input)
	{
		detail::RouteSet routeSet;

		if (input.at(velocity).IsDouble()) {
			routeSet.velocity = input.at(velocity).AsDouble();
		}
		if (input.at(time).IsInt()) {
			routeSet.waitTime = input.at(time).AsInt();
		}
		return routeSet;
	}

	ResponseData JsonReader::ReadData(std::istream & input, renderer::Settings& setup) {

		std::deque<detail::Query> queriesToAdd;
		std::deque<std::unordered_map<std::string, std::vector<detail::Distance>>> queryTowardStop;
		std::deque<detail::Query> queriesToBase;
		detail::RouteSet routeSettings;
		std::string nameBase;

		const json::Node root = LoadJSON(input).GetRoot();

		if (root.IsDict() && root.AsDict().empty()) {
			return { queriesToAdd, queryTowardStop, queriesToBase, routeSettings, nameBase };
		}

		json::Dict queries = root.AsDict();

		for (auto& query : queries) {
			if (query.first == baseReq) {
				/**********Read query to add data***********/
				if (query.second.IsArray() && query.second.AsArray().empty()) {
					continue;
				}
				queriesToAdd = ReadAddQuery(query.second);
				queryTowardStop = ReadAddTowardStop(query.second);
			}
			else if (query.first == statReq) {
				/********Read query to get data************/
				if (query.second.IsArray() && query.second.AsArray().empty()) {
					continue;
				}
				queriesToBase = ReadGetQuery(query.second);
			}
			else if (query.first == rendSet) {
				/**********Read settings into map_renderer******/
				if (query.second.IsDict() && query.second.AsDict().empty()) {
					continue;
				}
				ReadRenderQuery(query.second.AsDict(), setup);
			}
			else if (query.first == routeSet) {
				/**********Read settings into Route******/
				if (query.second.IsDict() && query.second.AsDict().empty()) {
					continue;
				}
				routeSettings = ReadRoutingQuery(query.second.AsDict());
			}
			else if(query.first == serialSet) {
				/**********Read settings serialization******/
				if (query.second.IsDict() && query.second.AsDict().empty()) {
					continue;
				}
				nameBase = query.second.AsDict().at("file").AsString();
			}
			else {
				json::ParsingError("Input data is wrong"s);
			}
		}

		return { queriesToAdd, queryTowardStop, queriesToBase, routeSettings, nameBase };
	}

	json::Document JsonReader::LoadJSON(std::istream & s)
	{
		return json::Load(s);
	}

	/*********************************Write******************************/
	void JsonReader::GetData(std::ostream& output, const handler::RequestHandler& reqHandler,
		const std::deque<detail::Query> queriesReq)
	{
		bool first_printed{ false };

		output << "[\n";

		for (auto& query : queriesReq) {
			if (first_printed) {
				output << ",\n";
			}

			if (query.typeOfQuery == "Bus"s) {
				const auto& stat = reqHandler.GetBusStat(query.nameBus);
				PrintData(output, stat, query.id_query);
				first_printed = true;
			}
			else if (query.typeOfQuery == "Map"s) {
				const svg::Document doc = reqHandler.RenderMap();
				PrintData(output, doc, query.id_query);
				first_printed = true;
			}
			else if (query.typeOfQuery == "Stop"s) {
				const auto* stop_to_buses = reqHandler.GetBusesByStop(query.nameStop);
				PrintData(output, stop_to_buses, query.id_query, query.nameStop);
				first_printed = true;
			}
			else if (query.typeOfQuery == "Route"s) {
				PrintData(output, query.from, query.to, reqHandler, query.id_query);
				first_printed = true;
			}
			else { continue; }
		}
		output << "\n]";
	}
	
	void JsonReader::PrintData(std::ostream& output, const std::optional<Stat>& data, const int id_req)
	{
		if (!(data.has_value())) {
			json::Node dict_node = json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id_req)
				.Key("error_message"s)
				.Value("not found"s)
				.EndDict()
				.Build();
			output << PrintJSON(dict_node);
			return;
		}
		const auto&[numStops, numUniqueStops, lengthRoute, curvature] = data.value();

		json::Node dict_node = json::Builder{}
			.StartDict()
			.Key("request_id"s)
			.Value(id_req)
			.Key("curvature"s)
			.Value(curvature)
			.Key("route_length"s)
			.Value(lengthRoute)
			.Key("stop_count"s)
			.Value(numStops)
			.Key("unique_stop_count"s)
			.Value(numUniqueStops)
			.EndDict()
			.Build();
		output << PrintJSON(dict_node);
	}

	void JsonReader::PrintData(std::ostream& output, const std::unordered_set<Buses>* data,
		const int id_req, const std::string_view nameStop) {
		if (nameStop.empty() || data == nullptr) {
			json::Node dict_node = json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id_req)
				.Key("error_message"s)
				.Value("not found"s)
				.EndDict()
				.Build();

			output << PrintJSON(dict_node);
			return;
		}
		else
		{
			std::vector<std::string_view> result;
			std::copy(data->begin(), data->end(), std::back_inserter(result));
			std::sort(result.begin(), result.end());

			json::Array arr_node;
			for (auto it = result.begin(); it != result.end(); it++) {
				json::Node node((std::string)*it);
				arr_node.push_back(node);
			}
			json::Node dict_node = json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id_req)
				.Key("buses"s)
				.Value(arr_node)
				.EndDict()
				.Build();

			output << PrintJSON(dict_node);
			return;
		}
	}

	void JsonReader::PrintData(std::ostream & output, const svg::Document & doc, const int id_req)
	{
		std::ostringstream map;
		doc.Render(map);
		json::Node dict_node = json::Builder{}
			.StartDict()
			.Key("request_id"s)
			.Value(id_req)
			.Key("map"s)
			.Value(map.str())
			.EndDict()
			.Build();

		output << PrintJSON(dict_node);
	}

	void JsonReader::PrintData(std::ostream& output, const std::string& from,
		const std::string& to, const handler::RequestHandler& reqHandler,
		const int id_req)
	{
		const auto& stopIds = reqHandler.GetRouter().GetMakedGraph().GetStopIds();
		if (!(stopIds.count(from) && stopIds.count(to))) {
			json::Node dict_node = json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id_req)
				.Key("error_message"s)
				.Value("not found"s)
				.EndDict()
				.Build();
			output << PrintJSON(dict_node);
			return;
		}
	
		const size_t fromVertId = stopIds.at(from);
		const size_t toVertId = stopIds.at(to);
		const std::optional<Route>& data = reqHandler.GetRouter().GetPtrRoute().get()->BuildRoute(fromVertId, toVertId);
		if (!(data.has_value())) {
			json::Node dict_node = json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id_req)
				.Key("error_message"s)
				.Value("not found"s)
				.EndDict()
				.Build();
			output << PrintJSON(dict_node);
			return;
		}
	
		const auto& graph = reqHandler.GetRouter().GetMakedGraph().GetGraph();
		
		json::Node dict_node = json::Builder{}
			.StartDict()
			.Key("items"s)
			.Value(GetEdges(data->edges, graph))
			.Key("total_time"s)
			.Value(data->weight)
			.Key("request_id"s)
			.Value(id_req)
			.EndDict()
			.Build();		
		output << PrintJSON(dict_node);
	}

	std::string JsonReader::PrintJSON(const json::Node & node)
	{
		std::ostringstream out;
		Print(json::Document{ node }, out);
		return out.str();
	}
	json::Node JsonReader::GetEdges(const std::vector<graph::EdgeId>& edges,
		const graph::DirectedWeightedGraph<double>& graph) const
	{
		json::Array items;
		items.reserve(edges.size());
		for (const auto& edgeId : edges){
			const graph::Edge<double>& edge = graph.GetEdge(edgeId);
			if (edge.span_count == 0) {
				items.emplace_back(
					json::Builder{}
					.StartDict()
					.Key("type"s).Value("Wait"s)
					.Key("stop_name"s).Value(edge.name)
					.Key("time"s).Value(edge.weight)
					.EndDict()
					.Build()
				);
			}
			else {
				items.emplace_back(
					json::Builder{}
					.StartDict()
					.Key("type"s).Value("Bus"s)
					.Key("bus"s).Value(edge.name)
					.Key("span_count"s).Value((int)edge.span_count)
					.Key("time"s).Value(edge.weight)
					.EndDict()
					.Build()
				);
			}
		}
		return items;
	}
}