#pragma once
#include "transport_catalogue.h"
#include "router.h"

#include <memory>

namespace graph {

	struct HasherStops {
		size_t operator()(const std::string& stopName) const noexcept;
	private:
		std::hash<std::string> ptr_hasher;
	};

	using StopNameToVertexId = std::unordered_map<std::string, std::size_t, HasherStops>;

	class TransportGraph {
	public:
		TransportGraph(const tc::TransportCatalogue & db, double velocity, int waitTime);

		const graph::DirectedWeightedGraph<double>& GetGraph() const;
		const StopNameToVertexId& GetStopIds() const;
		//const std::unique_ptr<graph::Router<double>>& GetPtrRoute() const;
	private:
		graph::DirectedWeightedGraph<double> graph_;
		//std::unique_ptr<graph::Router<double>> ptrRoute_;
		StopNameToVertexId stopIds_;

		// set vertex into graph
		void SetVertex(int waitTime, const std::vector<domain::Stop>& stops);
		// set edge into graph
		void SetEdge(double velocity, const tc::TransportCatalogue & db);
	};

	class TransportRouter final: public graph::Router<double> {
	public:
		TransportRouter(const TransportGraph& makedGraph);
		
		const TransportGraph& GetMakedGraph() const;
		const std::unique_ptr<graph::Router<double>>& GetPtrRoute() const;
	private:
		const TransportGraph& makedGraph_;
		std::unique_ptr<graph::Router<double>> ptrRoute_;
	};
}
