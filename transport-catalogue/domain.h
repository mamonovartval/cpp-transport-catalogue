#pragma once
#include "geo.h"

#include <string>
#include <vector>

namespace domain
{
	struct Stop {
		Stop(const std::string& name, const double& lat,
			const double& lon);
		std::string nameStop;
		double latitude;
		double longitude;
	};

	struct Bus {
		Bus(const std::string& name, const bool typeRoute);
		std::string nameBus;
		bool isRing; // true - ring route, false - reverse route
		int numStops{ 0 };
		int numUniqueStops{ 0 };
		std::vector<const Stop*> ptr_ToStops;
		double lengthRoute{ 0.0 };
		double curvature{ 0.0 };
		std::string endStop; //end stop on no ring route where bus change motion to back direction
	};
}

