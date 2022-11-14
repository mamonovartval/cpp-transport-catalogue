#include "json.h"

#include <cctype>

using namespace std;

namespace json {

	namespace {

		Node LoadNode(istream& input);

		Node LoadArray(istream& input) {
			Array result;

			for (char c; input >> c && c != ']';) {
				if (c != ',') {
					input.putback(c);
				}
				result.push_back(LoadNode(input));
			}
			if (!input) {
				throw ParsingError("Array Error!");
			}

			return Node(move(result));
		}

		Node LoadString(istream& input) {
			string line;
			int numQoutes = 1;
			while (input.peek() != EOF) {
				char ch = input.get();
				if (ch == '"') {
					++numQoutes;
					break;
				}
				if (ch == '\\') {
					input.get(ch);
					switch (ch) {
					case '"': {
						line.push_back('"');
						break;
					}
					case '\\': {
						line.push_back('\\');
						break;
					}
					case 'n': {
						line.push_back('\n');
						break;
					}
					case 'r': {
						line.push_back('\r');
						break;
					}
					case 't': {
						line.push_back('\t');
						break;
					}
					default:
						throw ParsingError("invalid escape character!"s);
					}
				}
				else {
					line.push_back(ch);
				}
			}
			if (numQoutes % 2 != 0) {
				throw ParsingError("Unpaired quotes!");
			}
			return Node(move(line));
		}

		Node LoadDict(istream& input) {
			Dict result;

			for (char c; input >> c && c != '}';) {
				if (c == ',') {
					input >> c;
				}

				string key = LoadString(input).AsString();
				input >> c;
				result.insert({ move(key), LoadNode(input) });
			}
			if (!input) {
				throw ParsingError("Dict Error!");
			}
			return Node(move(result));
		}

		Node LoadBool(istream& input) {
			string str;
			while (isalpha(input.peek())) {
				str += input.get();
			}
			if (str == "true") {
				return Node(true);
			}
			else if (str == "false") {
				return Node(false);
			}
			else {
				throw ParsingError("Input is wrong"s);
			}
		}

		Node LoadNull(istream& input) {
			string str;
			while (isalpha(input.peek())) {
				str += input.get();
			}
			if (str == "null") {
				return Node();
			}
			else {
				throw ParsingError("Input is wrong"s);
			}
		}

		Node LoadNumber(std::istream& input) {
			using namespace std::literals;

			std::string parsed_num;

			// Считывает в parsed_num очередной символ из input
			auto read_char = [&parsed_num, &input] {
				parsed_num += static_cast<char>(input.get());
				if (!input) {
					throw ParsingError("Failed to read number from stream"s);
				}
			};

			// Считывает одну или более цифр в parsed_num из input
			auto read_digits = [&input, read_char] {
				if (!std::isdigit(input.peek())) {
					throw ParsingError("A digit is expected"s);
				}
				while (std::isdigit(input.peek())) {
					read_char();
				}
			};

			if (input.peek() == '-') {
				read_char();
			}
			// Парсим целую часть числа
			if (input.peek() == '0') {
				read_char();
				// После 0 в JSON не могут идти другие цифры
			}
			else {
				read_digits();
			}

			bool is_int = true;
			// Парсим дробную часть числа
			if (input.peek() == '.') {
				read_char();
				read_digits();
				is_int = false;
			}

			// Парсим экспоненциальную часть числа
			if (int ch = input.peek(); ch == 'e' || ch == 'E') {
				read_char();
				if (ch = input.peek(); ch == '+' || ch == '-') {
					read_char();
				}
				read_digits();
				is_int = false;
			}

			try {
				if (is_int) {
					// Сначала пробуем преобразовать строку в int
					try {
						return Node(std::stoi(parsed_num));
					}
					catch (...) {
						// В случае неудачи, например, при переполнении,
						// код ниже попробует преобразовать строку в double
					}
				}
				return Node(std::stod(parsed_num));
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		Node LoadNode(istream& input) {
			char ch;
			if (!(input >> ch)) {
				throw ParsingError("Input is empty"s);
			}
			else if (ch == '[') {
				return LoadArray(input);
			}
			else if (ch == '{') {
				return LoadDict(input);
			}
			else if (ch == '"') {
				return LoadString(input);
			}
			else if (ch == 't' || ch == 'f') {
				input.putback(ch);
				return LoadBool(input);
			}
			else if (ch == 'n') {
				input.putback(ch);
				return LoadNull(input);
			}
			else {
				input.putback(ch);
				return LoadNumber(input);
			}
		}

	}  // namespace

	/*************************Print operators********************/

	void OstreamJSONPrinter::operator()(std::nullptr_t) const
	{
		out << NoneData;
	}

	void OstreamJSONPrinter::operator()(const Array& arr) const
	{
		out << "["s;
		bool first_step = true;
		for (auto& element : arr) {
			if (first_step) {
				first_step = false;
			}
			else {
				out << ", "s;
			}
			PrintNode(element, out);
		}
		out << "]"s;
	}

	void OstreamJSONPrinter::operator()(const Dict& map) const
	{
		out << "{"s;
		bool first_step = true;
		for (auto&[key, value] : map) {
			if (first_step) {
				first_step = false;
			}
			else {
				out << ", "s;
			}
			PrintNode(key, out);
			out << ": "s;
			PrintNode(value, out);
		}
		out << "}"s;
	}

	void OstreamJSONPrinter::operator()(const bool value) const
	{
		if (value) {
			out << "true"s;
		}
		else {
			out << "false"s;
		}

	}

	void OstreamJSONPrinter::operator()(const int value) const
	{
		out << value;
	}

	void OstreamJSONPrinter::operator()(const double value) const
	{
		out << value;
	}

	void OstreamJSONPrinter::operator()(const std::string value) const
	{
		out << "\""s;
		for (const auto ch : value) {
			switch (ch) {
			case '"': {
				out << '\\' << '\"';
				break;
			}
			case '\\': {
				out << '\\' << '\\';
				break;
			}
			case '\n': {
				out << '\\' << 'n';
				break;
			}
			case '\r': {
				out << '\\' << 'r';
				break;
			}
			case '\t': {
				out << '\t';
				break;
			}
			default:
				out << ch;
				break;
			}
		}
		out << "\"";
	}


	/*************************Node******************************/
	/*************************Constructors**********************/

	Node::Node(std::nullptr_t t)
		: jNode_(t)
	{
	}

	Node::Node(Array array)
		: jNode_(move(array)) {
	}

	Node::Node(Dict map)
		: jNode_(move(map)) {
	}

	Node::Node(bool value)
		: jNode_(move(value))
	{
	}

	Node::Node(int value)
		: jNode_(value) {
	}

	Node::Node(double value)
		: jNode_(value)
	{
	}

	Node::Node(string value)
		: jNode_(move(value)) {
	}

	// методы Is проверяют что лежит в variant, если другой тип, выбрасываем исключение

	bool Node::IsNull() const {
		return std::holds_alternative<std::nullptr_t>(this->jNode_);
	}

	bool Node::IsArray() const {
		return std::holds_alternative<Array>(this->jNode_);
	}

	bool Node::IsDict() const {
		return std::holds_alternative<Dict>(this->jNode_);
	}

	bool Node::IsBool() const {
		return std::holds_alternative<bool>(this->jNode_);
	}

	bool Node::IsInt() const {
		return std::holds_alternative<int>(this->jNode_);
	}

	bool Node::IsDouble() const {
		return IsInt() || IsPureDouble();
	}

	bool Node::IsPureDouble() const {
		return std::holds_alternative<double>(this->jNode_);
	}

	bool Node::IsString() const {
		return std::holds_alternative<std::string>(this->jNode_);
	}

	// методы As проверяют, что лежит дата-члене класса и возвращают этот вариант

	const Array& Node::AsArray() const {
		if (IsArray()) {
			return std::get<Array>(jNode_);
		}
		else {
			throw logic_error("Type is not Array"s);
		}
	}

	const Dict& Node::AsDict() const {
		if (IsDict()) {
			return std::get<Dict>(jNode_);
		}
		else {
			throw logic_error("Type is not Dict"s);
		}
	}

	bool Node::AsBool() const {
		if (IsBool()) {
			return std::get<bool>(jNode_);
		}
		else {
			throw logic_error("Type is not bool"s);
		}
	}

	int Node::AsInt() const {
		if (IsInt()) {
			return std::get<int>(jNode_);
		}
		else {
			throw logic_error("Type is not int"s);
		}
	}

	double Node::AsDouble() const {
		if (IsPureDouble()) {
			return std::get<double>(jNode_);
		}
		else if (IsInt()) {
			return std::get<int>(jNode_);
		}
		else {
			throw logic_error("Type is not double"s);
		}
	}

	const std::string& Node::AsString() const {
		if (IsString()) {
			return std::get<std::string>(jNode_);
		}
		else {
			throw logic_error("Type is not string"s);
		}
	}

	/***********************************************************/

	bool Node::operator==(const Node & other) const
	{
		return this->jNode_ == other.jNode_;
	}

	bool Node::operator!=(const Node & other) const
	{
		return !(this->jNode_ == other.jNode_);
	}

	/***********************************************************/

	const Json& Node::GetData() const 
	{
		return this->jNode_;
	}

	Json& Node::GetData()
	{
		return this->jNode_;
	}

	/********************Document*******************************/

	Document::Document(Node root)
		: root_(move(root)) {
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	bool Document::operator==(const Document & rhs)
	{
		return (root_ == rhs.GetRoot());
	}

	bool Document::operator!=(const Document & rhs)
	{
		return !(root_ == rhs.GetRoot());
	}

	Document Load(istream& input) {
		return Document{ LoadNode(input) };
	}

	void Print(const Document& doc, std::ostream& output)
	{
		std::visit(OstreamJSONPrinter{ output }, doc.GetRoot().GetData());
	}

	void PrintNode(const Node& node, std::ostream& output)
	{
		std::visit(OstreamJSONPrinter{ output }, node.GetData());
	}
}  // namespace json