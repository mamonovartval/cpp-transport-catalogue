#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <optional>
#include <memory>

 // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
 // с другими подсистемами приложения.
 // См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace handler {

	struct HasherStops {
		size_t operator()(const std::string& stopName) const noexcept;
	private:
		std::hash<std::string> ptr_hasher;
	};
	
	using BusStat = std::tuple<int, int, double, double>;
	using BusPtr = std::string_view;
	using StopNameToVertexId = std::unordered_map<std::string, std::size_t, HasherStops>;

	class RequestHandler {
	public:
		RequestHandler(const tc::TransportCatalogue& db, const renderer::MapRenderer& renderer,
			const graph::TransportRouter& tr);

		// Возвращает информацию о маршруте (запрос Bus)
		std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

		// Возвращает маршруты, проходящие через
		const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

		// Строит карту
		svg::Document RenderMap() const;

		const graph::TransportRouter& GetRouter() const;
		
	private:
		// RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
		const tc::TransportCatalogue& db_;
		const renderer::MapRenderer& renderer_;
		const graph::TransportRouter& rdb_;
	};
}