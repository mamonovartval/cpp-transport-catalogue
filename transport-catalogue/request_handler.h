#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <optional>
#include <memory>

 // ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
 // � ������� ������������ ����������.
 // ��. ������� �������������� �����: https://ru.wikipedia.org/wiki/�����_(������_��������������)

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

		// ���������� ���������� � �������� (������ Bus)
		std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

		// ���������� ��������, ���������� �����
		const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

		// ������ �����
		svg::Document RenderMap() const;

		const graph::TransportRouter& GetRouter() const;
		
	private:
		// RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
		const tc::TransportCatalogue& db_;
		const renderer::MapRenderer& renderer_;
		const graph::TransportRouter& rdb_;
	};
}