#include "domain.h"

namespace domain {
	Stop::Stop(const std::string& name, const double& lat, const double& lon)
		:nameStop(name), latitude(lat), longitude(lon) {}

	Bus::Bus(const std::string& name, const bool typeRoute)
		: nameBus(name), isRing(typeRoute) {}
}