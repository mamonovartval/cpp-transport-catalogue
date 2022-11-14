#include "svg.h"
#include <string>
#include <iomanip>

namespace svg {

	using namespace std::literals;

	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}

	// ---------- Circle ------------------

	Circle& Circle::SetCenter(Point center) {
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		radius_ = radius;
		return *this;
	}

	Circle & Circle::SetFillColor(Color color)
	{
		return PathProps::SetFillColor(color);
	}

	Circle & Circle::SetStrokeColor(Color color)
	{
		return PathProps::SetStrokeColor(color);
	}

	Circle & Circle::SetStrokeWidth(double width)
	{
		return PathProps::SetStrokeWidth(width);
	}

	Circle & Circle::SetStrokeLineCap(StrokeLineCap line_cap)
	{
		return PathProps::SetStrokeLineCap(line_cap);
	}

	Circle & Circle::SetStrokeLineJoin(StrokeLineJoin line_join)
	{
		return PathProps::SetStrokeLineJoin(line_join);
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\" "sv;
		// Выводим атрибуты, унаследованные от PathProps
		RenderAttrs(context.out);
		out << "/>"sv;
	}

	// ---------- Polyline ------------------

	Polyline & Polyline::AddPoint(Point point)	{
		points_.push_back(point);
		return *this;
	}

	Polyline & Polyline::SetFillColor(Color color)
	{
		return PathProps::SetFillColor(color);
	}

	Polyline & Polyline::SetStrokeColor(Color color)
	{
		return PathProps::SetStrokeColor(color);
	}

	Polyline & Polyline::SetStrokeWidth(double width)
	{
		return PathProps::SetStrokeWidth(width);
	}

	Polyline & Polyline::SetStrokeLineCap(StrokeLineCap line_cap)
	{
		return PathProps::SetStrokeLineCap(line_cap);
	}

	Polyline & Polyline::SetStrokeLineJoin(StrokeLineJoin line_join)
	{
		return PathProps::SetStrokeLineJoin(line_join);
	}

	void Polyline::RenderObject(const RenderContext & context) const	{
		auto& out = context.out;
		bool flag = true;
		
		if (points_.empty()) {
			out << "<polyline points=\""sv;
		}
		else {
			for (auto& p : points_) {
				// empty string by default
				if (points_.size() == 1 && p.x == 0 && p.y == 0) {
					out << "<polyline points=\""sv;
					break;
				}

				if (flag) {
					out << "<polyline points=\""sv << p.x << ","sv << p.y;
					flag = false;
				}
				else {
					out << " " << p.x << ","sv << p.y;
				}
			}
		}
		out << "\" "sv;
		// Выводим атрибуты, унаследованные от PathProps
		RenderAttrs(context.out);
		out << "/>"sv;
	}

	// ---------- Text ------------------

	Text & Text::SetPosition(Point pos)	{
		pos_ = pos;
		return *this;
	}

	Text & Text::SetOffset(Point offset) {
		offset_ = offset;
		return *this;
	}

	Text & Text::SetFontSize(uint32_t size)	{
		size_ = size;
		return *this;
	}

	Text & Text::SetFontFamily(std::string font_family)	{
		font_family_ = font_family;
		return *this;
	}

	Text & Text::SetFontWeight(std::string font_weight)	{
		font_weight_ = font_weight;
		return *this;
	}

	Text & Text::SetData(std::string data)	{
		data_ = data;
		return *this;
	}

	Text & Text::SetFillColor(Color color)
	{
		return PathProps::SetFillColor(color);
	}

	Text & Text::SetStrokeColor(Color color)
	{
		return PathProps::SetStrokeColor(color);
	}

	Text & Text::SetStrokeWidth(double width)
	{
		return PathProps::SetStrokeWidth(width);
	}

	Text & Text::SetStrokeLineCap(StrokeLineCap line_cap)
	{
		return PathProps::SetStrokeLineCap(line_cap);
	}

	Text & Text::SetStrokeLineJoin(StrokeLineJoin line_join)
	{
		return PathProps::SetStrokeLineJoin(line_join);
	}

	void Text::RenderObject(const RenderContext & context) const
	{
		auto& out = context.out;
		
		// cast default features to SVG format
		out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y
			<< "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y
			<< "\" font-size=\""sv << size_;

		if (!font_family_.empty()) {
			out << "\" font-family=\""sv << font_family_;
		}
		
		if (!font_weight_.empty())	{
			out << "\" font-weight=\""sv << font_weight_;
		}
		out << "\""sv;
		// Выводим атрибуты, унаследованные от PathProps
		RenderAttrs(context.out);
		out << ">"sv;

		if (!data_.empty()) {
			std::string text;
			// cast text to SVG format
			for (const auto& ch : data_) {
				switch (ch)
				{
				case '"': {
					text += "&qout;";
					break;
				}
				case '\'': {
					text += "&apos;";
					break;
				}
				case '<': {
					text += "&lt;";
					break;
				}
				case '>': {
					text += "&gt;";
					break;
				}
				case '&': {
					text += "&amp;";
					break;
				}
				default:
					text += ch;
					break;
				}
			}
			out << text;
		}
				
		out << "</text>"sv;
	}

	// ---------- Document ------------------

	void Document::AddPtr(std::unique_ptr<Object>&& obj)
	{
		objects_.emplace_back(std::move(obj));
	}

	void Document::Render(std::ostream & out) const
	{
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
		for (const auto& obj : objects_) {
			obj->Render(out);
		}
		out << "</svg>"sv;
	}
		
	std::ostream & operator<<(std::ostream & out, const StrokeLineCap & line_cap)
	{
		using namespace std::literals;
		switch (line_cap) {
		case StrokeLineCap::BUTT: {
			out << "butt"sv;
			break;
		}
		case StrokeLineCap::ROUND: {
			out << "round"sv;
			break;
		}
		case StrokeLineCap::SQUARE: {
			out << "square"sv;
			break;
		}
		default:
			break;
		}
		return out;
	}

	std::ostream & operator<<(std::ostream & out, const StrokeLineJoin & line_join)
	{
		using namespace std::literals;
		switch (line_join) {
		case StrokeLineJoin::ARCS: {
			out << "arcs"sv;
			break;
		}
		case StrokeLineJoin::BEVEL: {
			out << "bevel"sv;
			break;
		}
		case StrokeLineJoin::MITER: {
			out << "miter"sv;
			break;
		}
		case StrokeLineJoin::MITER_CLIP: {
			out << "miter-clip";
			break;
		}
		case StrokeLineJoin::ROUND: {
			out << "round"sv;
			break;
		}
		default:
			break;
		}

		return out;
	}

	void OstreamColorPrinter::operator()(std::monostate) const
	{
		return;
	}

	void OstreamColorPrinter::operator()(std::string s) const
	{
		out << s;
	}

	void OstreamColorPrinter::operator()(Rgb rgb) const
	{
		out << "rgb("s << std::to_string(rgb.red) << ","s << std::to_string(rgb.green)
			<< ","s << std::to_string(rgb.blue) << ")"s;
	}

	void OstreamColorPrinter::operator()(Rgba rgba) const
	{
		out << "rgba("s << std::to_string(rgba.red) << ","s << std::to_string(rgba.green)
			<< ","s << std::to_string(rgba.blue) << ","s << rgba.opacity << ")"s;
	}

}  // namespace svg