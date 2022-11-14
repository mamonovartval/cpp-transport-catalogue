#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	using Json = std::variant < std::nullptr_t, Array, Dict, bool, int, double, std::string>;
	inline const std::string NoneData{ "null" };

	struct OstreamJSONPrinter {
		std::ostream& out;
		void operator()(std::nullptr_t) const;
		void operator()(const Array& arr) const;
		void operator()(const Dict& map) const;
		void operator()(const bool value) const;
		void operator()(const int value) const;
		void operator()(const double value) const;
		void operator()(const std::string value) const;
	};

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node {
	public:
		Node() = default;
		Node(std::nullptr_t);
		Node(Array array);
		Node(Dict map);
		Node(bool value);
		Node(int value);
		Node(double value);
		Node(std::string value);

		// методы Is проверяют что лежит в variant, если другой тип, выбрасываем исключение
		bool IsNull() const;
		bool IsArray() const;
		bool IsDict() const;
		bool IsBool() const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsString() const;

		// методы As проверяют, что лежит дата-члене класса и возвращают этот вариант	   
		const Array& AsArray() const;
		const Dict& AsDict() const;
		bool AsBool() const;
		int AsInt() const;
		double AsDouble() const;
		const std::string& AsString() const;

		bool operator==(const Node& other) const;
		bool operator!=(const Node& other) const;

		const Json& GetData() const;
		Json& GetData();

	private:
		Json jNode_;
	};

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

		bool operator==(const Document& rhs);
		bool operator!=(const Document& rhs);

	private:
		Node root_;
	};

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);
	void PrintNode(const Node& node, std::ostream& output);
	
}  // namespace json