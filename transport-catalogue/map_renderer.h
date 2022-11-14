#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>

namespace renderer
{
	struct Settings {
		double width{ 0.0 };
		double	height{ 0.0 };

		double	padding{ 0.0 };

		double	line_width{ 0.0 };
		double	stop_radius{ 0.0 };

		int bus_label_font_size{ 0 };
		std::vector<double> bus_label_offset;

		int stop_label_font_size{ 0 };
		std::vector<double> stop_label_offset;

		svg::Color underlayer_color;
		double underlayer_width{ 0.0 };

		std::vector<svg::Color> color_palette;
	};

	bool IsZero(double value);

	namespace object
	{
		class BusLines :public svg::Drawable {
		public:
			BusLines(const std::vector<svg::Point>& vec_points,
				svg::Color stroke_color, double stroke_width);

			void Draw(svg::ObjectContainer& container) const override;

		private:
			std::vector<svg::Point> vec_points_;
			svg::Color stroke_color_;
			double stroke_width_;
		};
		class BusLabels :public svg::Drawable {
		public:
			BusLabels(const std::string_view name, svg::Point pos,
				svg::Color stroke_color, const renderer::Settings& settings);

			void Draw(svg::ObjectContainer& container) const override;

		private:
			std::string nameBus_;
			svg::Point pos_;
			double dx_;
			double dy_;
			int font_size_;
			svg::Color underFillColor_;
			svg::Color underStrokeColor_;
			svg::Color text_color_;
			double stroke_width_;
		};
		class StopPoints :public svg::Drawable {
		public:
			StopPoints(const std::vector<svg::Point>& spotPoints, double spot_radius);

			void Draw(svg::ObjectContainer& container) const override;

		private:
			std::vector<svg::Point> spot_points_;
			double spot_radius_;
		};
		class StopLabels :public svg::Drawable {
		public:
			StopLabels(const std::string& name, svg::Point pos, const renderer::Settings& settings);

			void Draw(svg::ObjectContainer& container) const override;

		private:
			std::string nameStop_;
			svg::Point pos_;
			double dx_;
			double dy_;
			int font_size_;
			svg::Color underFillColor_;
			svg::Color underStrokeColor_;
			double stroke_width_;
		};
	}

	template <typename DrawableIterator>
	void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {

		for (auto it = begin; it != end; ++it) {
			(*it)->Draw(target);
		}
	}

	template <typename Container>
	void DrawPicture(const Container& container, svg::ObjectContainer& target) {
		using namespace std;
		DrawPicture(begin(container), end(container), target);

	}

	class MapRenderer {
	public:
		template <typename PointInputIt>
		MapRenderer(PointInputIt points_begin, PointInputIt points_end, double max_width,
			double max_height, double padding) : padding_(padding)
		{
			if (points_begin == points_end) {
				return;
			}

			const auto[left_it, right_it]
				= std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
				return lhs.longitude < rhs.longitude;
			});
			min_lon_ = left_it->longitude;
			const double max_lon = right_it->longitude;

			const auto[bottom_it, top_it]
				= std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
				return lhs.latitude < rhs.latitude;
			});
			const double min_lat = bottom_it->latitude;
			max_lat_ = top_it->latitude;

			std::optional<double> width_zoom;
			if (!IsZero(max_lon - min_lon_)) {
				width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
			}

			std::optional<double> height_zoom;
			if (!IsZero(max_lat_ - min_lat)) {
				height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
			}

			if (width_zoom && height_zoom) {
				zoom_coeff_ = std::min(*width_zoom, *height_zoom);
			}
			else if (width_zoom) {
				zoom_coeff_ = *width_zoom;
			}
			else if (height_zoom) {
				zoom_coeff_ = *height_zoom;
			}
		}

		svg::Point operator()(geo::Coordinates coords) const;

		void SaveSettings(const Settings& settings);

		svg::Document GetMap(const std::map< std::string_view, const domain::Bus*>& sortedBuses) const;
		
	private:
		double padding_;
		double min_lon_ = 0;
		double max_lat_ = 0;
		double zoom_coeff_ = 0;
		
		Settings settings_;

		void BusLines(svg::Document& doc, const std::map< std::string_view, const domain::Bus*>& sortedBuses) const;
		void BusLabels(svg::Document& doc, const std::map< std::string_view, const domain::Bus*>& sortedBuses) const;
		void StopPoints(svg::Document& doc, const std::map< std::string_view, const domain::Bus*>& sortedBuses) const;
		void StopLabels(svg::Document& doc, const std::map< std::string_view, const domain::Bus*>& sortedBuses) const;

		std::vector<svg::Point> GetPoints(const std::vector<const domain::Stop*>& ptr_stops) const;
		std::vector<svg::Point> GetPoints(const std::map<std::string, const domain::Stop*>& ptr_stops) const;
		std::map<std::string, const domain::Stop*> GetSortedStops(const std::map< std::string_view, const domain::Bus*>& sortedBuses) const;
	};
}