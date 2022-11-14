#include "transport_catalogue.h"

#include <algorithm>
#include <exception>


namespace tc
{
	size_t HasherPtrStops::operator()(const std::pair<const domain::Stop*, const domain::Stop*> stop) const noexcept
	{
		size_t h_from = ptr_hasher(stop.first);
		size_t h_to = ptr_hasher(stop.second);

		return h_from + h_to * 37;
	}

	void TransportCatalogue::AddRouteToBase(const std::string& nameBus,
		const std::vector<std::string>& orderStops, const bool typeRoute) {

		domain::Bus bus(nameBus, typeRoute);
		// define type of route
		if (!bus.isRing) {
			// assign end stop on route where bus change motion to back direction
			bus.endStop = orderStops.back();
			//resize vector of stops for reverse route
			std::vector<std::string> tempVec = orderStops;
			//tempVec.resize(orderStops.size() - 1);
			// put data in reverse direction except end vector of stops
			for (auto it = (orderStops.rbegin() + 1); it != orderStops.rend(); it++) {
				tempVec.push_back(*it);
			}
			// add all ptr to stops for reverse bus 
			for (const std::string_view nameStop : tempVec) {
				auto it_stop = stopname_to_stop_.find(nameStop);
				bus.ptr_ToStops.push_back(it_stop->second);
			}
			// assign number of stops
			bus.numStops = tempVec.size();
			// assign number of unique stops
			std::unordered_set<std::string_view> uniqueNames(tempVec.begin(), tempVec.end());
			bus.numUniqueStops = uniqueNames.size();
		}
		else
		{
			// add all ptr to stops for ring bus
			for (const std::string_view nameStop : orderStops) {
				auto it_stop = stopname_to_stop_.find(nameStop);
				bus.ptr_ToStops.push_back(it_stop->second);
			}
			// assign number of stops
			bus.numStops = orderStops.size();
			// assign number of unique stops
			std::unordered_set<std::string_view> uniqueNames(orderStops.begin(), orderStops.end());
			bus.numUniqueStops = uniqueNames.size();
		}

		// calculate length of route
		bus.lengthRoute = ComputeLengthRoute(bus);

		// calculate curvature of route
		bus.curvature = ComputeCurvature(bus);

		// put data to buses_
		buses_.push_back(std::move(bus));
		// put data to hash container
		busname_to_bus_[nameBus] = (std::move(&buses_.back()));

		// add number of buses on each stop
		auto it_bus = busname_to_bus_.find(nameBus);
		for (auto& ptr_stop : it_bus->second->ptr_ToStops) {
			if (!stopname_to_buses_.count(ptr_stop)) {
				continue;
			}
			stopname_to_buses_[ptr_stop].insert(it_bus->second->nameBus);
		}
	}

	void TransportCatalogue::AddStopToBase(const std::string & nameStop,
		const double& lat, const double& lon) {

		domain::Stop stop(nameStop, lat, lon);
		stops_.push_back(std::move(stop));
		// insert to stopname_to_stop_
		stopname_to_stop_[nameStop] = (std::move(&stops_.back()));
		// put all ptr to stop to hash container stopName to buses
		stopname_to_buses_[std::move(&stops_.back())];
	}

	void TransportCatalogue::SetDistanceBetweenStops(const std::pair<const domain::Stop*,
		const domain::Stop*> towardStop, const int distance) {

		stops_to_distance_[towardStop] = static_cast<unsigned int>(distance);
	}

	const domain::Bus* TransportCatalogue::SearchRoute(const std::string_view& nameBus) const
	{
		// search data about route
		if (!busname_to_bus_.count(nameBus)) {
			const domain::Bus* bus = nullptr;
			return bus;
		}
		return busname_to_bus_.at(nameBus);
	}

	const domain::Stop* TransportCatalogue::SearchStop(const std::string_view& nameStop) const
	{
		if (!stopname_to_stop_.count(nameStop)) {
			const domain::Stop* stop = nullptr;
			return stop;
		}
		return stopname_to_stop_.at(nameStop);
	}

	const std::unordered_set<std::string_view>* TransportCatalogue::GetStopToBuses(const std::string_view & nameStop) const
	{
		const domain::Stop* stop = SearchStop(nameStop);
		if (!stopname_to_buses_.count(stop)) {
			return {};
		}
		return &stopname_to_buses_.at(stop);
	}

	std::vector<domain::Stop> TransportCatalogue::GetStops() const
	{
		if (stopname_to_stop_.empty()) {
			return {};
		}

		std::vector<domain::Stop> result;
		for (auto&[nameStop, dataStop] : stopname_to_stop_) {
			if (GetStopToBuses(nameStop)->empty()) {
				continue;
			}
			result.push_back(*dataStop);
		}
		return result;
	}

	const std::map< std::string_view, const domain::Bus*> TransportCatalogue::GetSortedBuses() const
	{
		if (busname_to_bus_.empty()) {
			return {};
		}

		std::map< std::string_view, const domain::Bus*> result;

		for (const auto&[busName, busData] : busname_to_bus_) {
			result[busName] = busData;
		}
		
		return result;
	}
	
	unsigned int TransportCatalogue::GetDistanceBetweenStops(const domain::Stop* from, const domain::Stop* to) const
	{
		if (stops_to_distance_.count(std::make_pair(from, to))) {
			return stops_to_distance_.at(std::make_pair(from, to));
		}
		// if in storage no key {from, to} reverse it to {to, from}
		return stops_to_distance_.at(std::make_pair(to, from));
	}

	double TransportCatalogue::ComputeLengthRoute(const domain::Bus bus)
	{
		double lengthRoute{ 0.0 };

		// calculate length by means setted distance
 		for (std::vector<const domain::Stop*>::const_iterator it(bus.ptr_ToStops.begin());
			it != (bus.ptr_ToStops.end() - 1); ++it) {
			lengthRoute += GetDistanceBetweenStops(*it, *(it + 1));
		}

		return lengthRoute;
	}

	double TransportCatalogue::ComputeCurvature(const domain::Bus bus)
	{
		double geoLengthRoute{ 0.0 };
		geo::Coordinates prevCoord;
		prevCoord.lat = 0.0;
		prevCoord.lng = 0.0;
		for (auto& ptr_stop : bus.ptr_ToStops) {
			geo::Coordinates nextCoord;
			nextCoord.lat = ptr_stop->latitude;
			nextCoord.lng = ptr_stop->longitude;

			if (prevCoord.lat == 0.0 && prevCoord.lng == 0.0) {
				prevCoord.lat = nextCoord.lat;
				prevCoord.lng = nextCoord.lng;
				continue;
			}
			// calculate geographical length of route by coordinate
			geoLengthRoute += ComputeDistance(prevCoord, nextCoord);
			prevCoord.lat = nextCoord.lat;
			prevCoord.lng = nextCoord.lng;
		}

		double curvature = bus.lengthRoute / geoLengthRoute;

		return curvature;
	}
}