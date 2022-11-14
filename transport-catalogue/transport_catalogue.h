#pragma once

#include "domain.h"

#include <queue>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <vector>
#include <map>

namespace tc
{
	using namespace std::literals;

	struct HasherPtrStops {
		size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*> stop) const noexcept;
	private:
		std::hash<const void*> ptr_hasher;
	};

	//using ResponseInfo = std::tuple<std::string_view, std::unordered_set<std::string_view>>;
	//using ResponseRoute = std::tuple<int, int, double, double>;
	using StorageBusNameToBus = std::unordered_map<std::string_view, const domain::Bus*>;
	using StorageStopNameToStop = std::unordered_map<std::string_view, const domain::Stop*>;
	using StorageStopNameToBuses = std::unordered_map<const domain::Stop*, std::unordered_set<std::string_view>>;
	using StorageStopsToDistance = std::unordered_map<const std::pair<const domain::Stop*, const domain::Stop*>, unsigned int, HasherPtrStops>;

	class TransportCatalogue
	{
	public:
		void AddRouteToBase(const std::string& nameBus,
			const std::vector<std::string>& orderStops, const bool typeRoute);
		void AddStopToBase(const std::string & nameStop,
			const double& lat, const double& lon);
		void SetDistanceBetweenStops(const std::pair<const domain::Stop*, const domain::Stop*>
			towardStop, const int distance);

		const domain::Bus* SearchRoute(const std::string_view& nameBus) const;
		const domain::Stop* SearchStop(const std::string_view& nameStop) const;
		const std::unordered_set<std::string_view>* GetStopToBuses(const std::string_view& nameStop) const;
		std::vector<domain::Stop> GetStops() const;
		unsigned int GetDistanceBetweenStops(const domain::Stop* from, const domain::Stop* to) const;
		const std::map< std::string_view, const domain::Bus*> GetSortedBuses() const;

	private:
		std::deque<domain::Bus> buses_;
		std::deque<domain::Stop> stops_;
		StorageBusNameToBus busname_to_bus_;
		StorageStopNameToStop stopname_to_stop_;
		StorageStopNameToBuses stopname_to_buses_;
		StorageStopsToDistance stops_to_distance_;

		double ComputeLengthRoute(const domain::Bus bus);
		double ComputeCurvature(const domain::Bus bus);
	};
}