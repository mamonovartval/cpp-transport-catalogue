#pragma once
#include "geo.h"

#include <string>
#include <vector>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
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

