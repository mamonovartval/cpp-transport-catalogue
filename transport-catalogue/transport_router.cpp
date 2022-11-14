#include "transport_router.h"


namespace graph {

	size_t HasherStops::operator()(const std::string& stopName) const noexcept
	{
		size_t h_str = ptr_hasher(stopName);

		return h_str;
	}
	   	  	
	/********************************TransportGraph*****************************/
	TransportGraph::TransportGraph(const tc::TransportCatalogue & db, double velocity, int waitTime)
		:graph_(db.GetSortedStops().size() * 2)
	{
		SetVertex(waitTime, db.GetSortedStops());
		SetEdge(velocity, db);
	}

	const graph::DirectedWeightedGraph<double>& TransportGraph::GetGraph() const
	{
		return graph_;
	}

	const StopNameToVertexId & TransportGraph::GetStopIds() const
	{
		return stopIds_;
	}

	void TransportGraph::SetGraph(graph::DirectedWeightedGraph<double>&& graph)
	{
		graph_ = std::move(graph);
	}

	void TransportGraph::SetStopIds(StopNameToVertexId&& stop_ids)
	{
		stopIds_ = std::move(stop_ids);
	}

	void TransportGraph::SetVertex(int waitTime, const std::vector<domain::Stop>& stops)
	{
		graph::VertexId counterVertex{ 0 };
		std::size_t span_count{ 0 };
		// fill vertex with wating time to graph
		for (const auto& s : stops) {
			// init container by name stop FROM
			//add vertex and weight( time from stop to stop) to graph
			// first add two ids stops and wait time on stop, but second id will be even
			stopIds_[s.nameStop] = counterVertex;
			// put span count equal zero it means that on giving stop wait duration = 0 in during motion
			graph_.AddEdge({ s.nameStop, span_count, counterVertex, ++counterVertex, (double)waitTime });
			// increase counter
			++counterVertex;
		}
	}

	void TransportGraph::SetEdge(double velocity, const tc::TransportCatalogue & db)
	{
		// get sorted all buses from tc
		const auto& busesSorted = db.GetSortedBuses();
		// make link between stops where are distances
		for (const auto&[nameBus, ptrBus] : busesSorted)
		{
			const std::vector<const domain::Stop*> ptrStops = ptrBus->ptr_ToStops;
			size_t stops_count = ptrStops.size();
			// start cycle and assign index [i] for vertex stop FROM
			for (size_t i = 0; i < stops_count; ++i) {
				// start cycle and assign index [j] for vertex stop TO
				for (size_t j = i + 1; j < stops_count; ++j) {
					// get pointer for each stop
					const domain::Stop* stop_from = ptrStops[i];
					const domain::Stop* stop_to = ptrStops[j];
					// get vertex id for each stop
					// vertex FROM
					graph::VertexId from = stopIds_.at(stop_from->nameStop);
					//vertex TO
					graph::VertexId to = stopIds_.at(stop_to->nameStop);
					// get distance between stops
					unsigned int dist = 0;
					for (size_t k = i + 1; k <= j; ++k) {
						dist += db.GetDistanceBetweenStops(ptrStops[k - 1], ptrStops[k]);
					}
					// compute neccessary time in route from stop to another one stop(multiple distance to 1.0)
					// km/h - > m / min     
					// km * 1000 / 60
					/*in minutes*/double timePath = dist * 1.0 / (velocity * 1000 / 60);
					std::size_t span_count = j - i;
					graph_.AddEdge({ ptrBus->nameBus, span_count, from + 1, to, timePath });
					// verify condition
					if (!ptrBus->isRing && stop_to->nameStop == ptrBus->endStop && j == stops_count / 2) break;
				}
			}
		}
	}
	
	TransportRouter::TransportRouter(const TransportGraph& makedGraph)
		: graph::Router<double>(makedGraph.GetGraph()), makedGraph_(makedGraph)
	{
		ptrRoute_ = std::make_unique<graph::Router<double>>(makedGraph.GetGraph());
	}

	const TransportGraph & TransportRouter::GetMakedGraph() const
	{
		return makedGraph_;
	}
	const std::unique_ptr<graph::Router<double>>& TransportRouter::GetPtrRoute() const
	{
		return ptrRoute_;
	}
}