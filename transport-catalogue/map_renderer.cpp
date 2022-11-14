#include "map_renderer.h"

namespace renderer
{
	inline const double EPSILON = 1e-6;
	bool IsZero(double value)
	{
		return std::abs(value) < EPSILON;
	}

	namespace object {

		/***************BusLines***************/
		BusLines::BusLines(const std::vector<svg::Point>& vec_points,
			svg::Color stroke_color, double stroke_width)
			:vec_points_(vec_points), stroke_color_(stroke_color),
			stroke_width_(stroke_width) {}

		void BusLines::Draw(svg::ObjectContainer & container) const
		{
			svg::Polyline object;

			for (auto p : vec_points_) {
				object.AddPoint(p);
			}
			// setup renderer settings
			object.SetFillColor(svg::NoneColor).
				SetStrokeColor(stroke_color_).
				SetStrokeWidth(stroke_width_).
				SetStrokeLineCap(svg::StrokeLineCap::ROUND).
				SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			// add object to container
			container.Add(object);
		}
		/**************************************/

		/***************BusLabels**************/
		BusLabels::BusLabels(const std::string_view name, svg::Point pos,
			svg::Color stroke_color, const renderer::Settings & settings)
			:nameBus_((std::string)name), pos_(pos), text_color_(stroke_color)
		{
			dx_ = settings.bus_label_offset.front();
			dy_ = settings.bus_label_offset.back();
			font_size_ = settings.bus_label_font_size;
			underFillColor_ = settings.underlayer_color;
			underStrokeColor_ = settings.underlayer_color;
			stroke_width_ = settings.underlayer_width;
		}

		void BusLabels::Draw(svg::ObjectContainer & container) const
		{
			// put settings to underlayer object
			using namespace std::string_literals;
			svg::Text underText;
			underText.SetFillColor(underFillColor_)
				.SetStrokeColor(underStrokeColor_)
				.SetStrokeWidth(stroke_width_)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
				.SetPosition(pos_)
				.SetOffset({ dx_, dy_ })
				.SetFontSize(font_size_)
				.SetFontFamily("Verdana"s)
				.SetFontWeight("bold"s)
				.SetData(nameBus_);
			// put settings to text
			svg::Text text;
			text.SetFillColor(text_color_)
				.SetPosition(pos_)
				.SetOffset({ dx_, dy_ })
				.SetFontSize(font_size_)
				.SetFontFamily("Verdana"s)
				.SetFontWeight("bold"s)
				.SetData(nameBus_);

			// add object to container
			container.Add(underText);
			container.Add(text);
		}
		/**************************************/

		/**************StopPoints**************/
		StopPoints::StopPoints(const std::vector<svg::Point>& spotPoints,
			double spot_radius)
			:spot_points_(spotPoints), spot_radius_(spot_radius) {}

		void StopPoints::Draw(svg::ObjectContainer & container) const
		{
			using namespace std::string_literals;

			// set renderer settings to spot
			for (auto spot_point : spot_points_) {
				svg::Circle spot;
				spot.SetCenter(spot_point)
					.SetRadius(spot_radius_)
					.SetFillColor("white"s);
				// add spot to container
				container.Add(spot);
			}
		}
		/**************************************/

		/**************StopLabels**************/
		StopLabels::StopLabels(const std::string & name, svg::Point pos,
			const renderer::Settings & settings)
			:nameStop_(name), pos_(pos)
		{
			dx_ = settings.stop_label_offset.front();
			dy_ = settings.stop_label_offset.back();
			font_size_ = settings.stop_label_font_size;
			underFillColor_ = settings.underlayer_color;
			underStrokeColor_ = settings.underlayer_color;
			stroke_width_ = settings.underlayer_width;
		}

		void StopLabels::Draw(svg::ObjectContainer & container) const
		{
			// put settings to underlayer object
			using namespace std::string_literals;
			svg::Text underText;
			underText.SetFillColor(underFillColor_)
				.SetStrokeColor(underStrokeColor_)
				.SetStrokeWidth(stroke_width_)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(pos_)
				.SetOffset({ dx_, dy_ })
				.SetFontSize(font_size_)
				.SetFontFamily("Verdana"s)
				.SetData(nameStop_);

			// put settings to text
			svg::Text text;
			text.SetFillColor("black"s)
				.SetPosition(pos_)
				.SetOffset({ dx_, dy_ })
				.SetFontSize(font_size_)
				.SetFontFamily("Verdana"s)
				.SetData(nameStop_);

			// add object to container
			container.Add(underText);
			container.Add(text);
		}
		/**************************************/
	}

	svg::Point MapRenderer::operator()(geo::Coordinates coords) const
	{
		return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
					(max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
	}

	void MapRenderer::SaveSettings(const Settings& settings)
	{
		settings_ = std::move(settings);
	}

	svg::Document MapRenderer::GetMap(const std::map< std::string_view, const domain::Bus*>& sortedBuses) const
	{
		// don't change order call of object
		svg::Document doc;
		BusLines(doc, sortedBuses);
		BusLabels(doc, sortedBuses);
		StopPoints(doc, sortedBuses);
		StopLabels(doc, sortedBuses);

		return doc;
	}

	void MapRenderer::BusLines(svg::Document & doc,
		const std::map< std::string_view, const domain::Bus*>& sortedBuses) const
	{
		std::vector<std::unique_ptr<svg::Drawable>> lines;
		unsigned int countColor{ 0 };

		for (const auto&[nameBus, busData] : sortedBuses) {
			// skip bus where no one stops
			if (busData->ptr_ToStops.empty()) {
				continue;
			}
			std::vector<svg::Point> vec_points = GetPoints(busData->ptr_ToStops);
			// compute id color
			/*¬ общем % это оператор модул€ (modulo).
			x % y делит x на y и возвращает остаток делени€.
			„исло всегда будет в диапазоне от 0 до y.
			„ем удобно пользоватьс€ дл€ палитры*/
			int id_color = countColor % settings_.color_palette.size();
			// put renderer settings to Busline object
			lines.emplace_back(std::make_unique<object::BusLines>(vec_points, settings_.color_palette[id_color],
				settings_.line_width));

			countColor++;
		}
		// draw lines buses
		DrawPicture(lines, doc);
	}

	void MapRenderer::BusLabels(svg::Document & doc,
		const std::map< std::string_view, const domain::Bus*>& sortedBuses) const
	{
		std::vector<std::unique_ptr<svg::Drawable>> labels;
		unsigned int countColor{ 0 };

		for (const auto&[nameBus, busData] : sortedBuses) {
			// skip bus where no one stops
			if (busData->ptr_ToStops.empty()) {
				continue;
			}
			std::vector<svg::Point> vec_points = GetPoints(busData->ptr_ToStops);
			// compute id color
			/*¬ общем % это оператор модул€ (modulo).
			x % y делит x на y и возвращает остаток делени€.
			„исло всегда будет в диапазоне от 0 до y.
			„ем удобно пользоватьс€ дл€ палитры*/
			int id_color = countColor % settings_.color_palette.size();
			// put renderer settings to Buslabels object for first stop on route
			labels.emplace_back(std::make_unique<object::BusLabels>(nameBus, vec_points.front(),
				settings_.color_palette[id_color], settings_));
			// looking for index of second end stop
			// number of stops in not ring route is equal odd number, that is why
			// second end stop is middle index of vector of points for stops
			int id = vec_points.size() / 2;
			// put renderer settings to Buslabels object for last stop on bus
			if (!(busData->isRing)
				&& (busData->ptr_ToStops.front()->nameStop != busData->ptr_ToStops[id]->nameStop))
			{
				labels.emplace_back(std::make_unique<object::BusLabels>(nameBus, vec_points[id],
					settings_.color_palette[id_color], settings_));
			}
			countColor++;
		}
		// draw labels stops
		DrawPicture(labels, doc);
	}

	void MapRenderer::StopPoints(svg::Document & doc,
		const std::map< std::string_view, const domain::Bus*>& sortedBuses) const
	{
		std::vector<std::unique_ptr<svg::Drawable>> points;

		// get coordinates of each stops
		std::vector<svg::Point> vec_points = GetPoints(GetSortedStops(sortedBuses));
		// put renderer settings to Stoppoints object for each route
		points.emplace_back(std::make_unique<object::StopPoints>(vec_points, settings_.stop_radius));
		// draw spots stops and add to doc
		DrawPicture(points, doc);
	}

	void MapRenderer::StopLabels(svg::Document& doc,
		const std::map< std::string_view, const domain::Bus*>& sortedBuses) const
	{
		std::vector<std::unique_ptr<svg::Drawable>> labels;

		for (const auto&[nameStop, ptr_stop] : GetSortedStops(sortedBuses)) {
			
			// put renderer settings to Stoplabels object for each stop
			geo::Coordinates coord;
			coord.lat = ptr_stop->latitude;
			coord.lng = ptr_stop->longitude;
			
			auto stopPoints = this->operator()(coord);
			labels.emplace_back(std::make_unique<object::StopLabels>(nameStop, stopPoints, settings_));
		}
		// draw spots stops and add to doc
		DrawPicture(labels, doc);
	}

	std::vector<svg::Point> MapRenderer::GetPoints(const std::vector<const domain::Stop*>& ptr_stops) const
	{
		// get coordinates of each stops
		std::vector<svg::Point> vec_points;
		for (const auto& stop : ptr_stops) {
			geo::Coordinates coord;
			coord.lat = stop->latitude;
			coord.lng = stop->longitude;
			
			auto stopPoints =this->operator()(coord);
			vec_points.emplace_back(stopPoints);
		}
		return vec_points;
	}

	std::vector<svg::Point> MapRenderer::GetPoints(const std::map<std::string, const domain::Stop*>& ptr_stops) const
	{
		// get and put coordinates for each sorted stops
		std::vector<svg::Point> vec_points;
		for (const auto&[name, stop] : ptr_stops) {
			geo::Coordinates coord;
			coord.lat = stop->latitude;
			coord.lng = stop->longitude;

			auto stopPoints = this->operator()(coord);
			vec_points.emplace_back(stopPoints);
		}
		return vec_points;
	}

	std::map<std::string, const domain::Stop*> MapRenderer::GetSortedStops(const std::map< std::string_view, const domain::Bus*>& sortedBuses) const
	{
		std::map<std::string, const domain::Stop*> sortedStops;

		for (const auto&[nameBus, busData] : sortedBuses) {
			// skip bus where no one stops
			if (busData->ptr_ToStops.empty()) {
				continue;
			}
			for (const auto& stop : busData->ptr_ToStops) {
				// make sort name of stops and put it to map container
				sortedStops[stop->nameStop] = stop;
			}
		}
		return sortedStops;
	}
}