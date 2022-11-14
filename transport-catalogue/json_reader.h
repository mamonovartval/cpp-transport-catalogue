#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

#include "json.h"

#include <sstream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace reader 
{
	namespace detail 
	{
		struct Query {
			std::string typeOfQuery;
			std::string nameStop;
			double latitude{ 0.0 };
			double longitude{ 0.0 };
			std::string nameBus;
			std::vector<std::string> routeStops;
			bool isRing{ false };
			int id_query{ 0 };
			std::string from;
			std::string to;
		};

		struct Distance {
			std::string to;
			int distance{ 0 };
		};

		struct RouteSet {/*********************************/
			double velocity;
			int waitTime;
		};
	}

	using ResponseAddQuery = std::deque<detail::Query>;
	using ResponseAddTowardStop = std::deque<std::unordered_map<std::string, std::vector<detail::Distance>>>;
	using ResponseData = std::tuple<std::deque<detail::Query>,
		std::deque<std::unordered_map<std::string, std::vector<detail::Distance>>>,
		std::deque<detail::Query>, detail::RouteSet>;
	using ResponseRoute = std::tuple<std::vector<std::string>, bool>;
	using Stat = handler::BusStat;
	using Buses = handler::BusPtr;
	using Route = graph::Router<double>::RouteInfo;
	using namespace std::literals;

	class JsonReader {
	public:
		JsonReader() = default;

		ResponseData ReadData(std::istream& input, renderer::Settings& setup);

		void GetData(std::ostream& output, const handler::RequestHandler& reqHandler,
			const std::deque<detail::Query> queriesReq);

	private:
		std::vector<std::string> GetStopsRoute(json::Node& input);
		std::vector<detail::Distance> GetStopsDistance(json::Node& input);
		ResponseAddQuery ReadAddQuery(const json::Node& input);
		ResponseAddTowardStop ReadAddTowardStop(const json::Node& input);
		std::deque<detail::Query> ReadGetQuery(const json::Node& input);
		void ReadRenderQuery(const json::Dict& input, renderer::Settings& setup);
		detail::RouteSet ReadRoutingQuery(const json::Dict& input);
		json::Document LoadJSON(std::istream& s);

		void PrintData(std::ostream& output, const std::optional<Stat>& data, const int id_req);
		void PrintData(std::ostream& output, const std::unordered_set<Buses>* data,
			const int id_req, const std::string_view nameStop);
		void PrintData(std::ostream& output, const svg::Document& doc,
			const int id_req);
		void PrintData(std::ostream& output, const std::string& from,
			const std::string& to, const handler::RequestHandler& reqHandler,
			const int id_req);
		
		std::string PrintJSON(const json::Node & node);

		json::Node GetEdges(const std::vector<graph::EdgeId>& edges,
			const graph::DirectedWeightedGraph<double>& graph) const;
	};
}

