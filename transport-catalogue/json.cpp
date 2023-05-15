#include "json.h"
#include <sstream>
#include <string>


namespace json {
    namespace {
        std::string Trim(std::string_view str) {
            while (!str.empty() && (str.front() == ' ' || str.front() == '\t'
                || str.front() == '\n' || str.front() == '\r')) {
                str.remove_prefix(1);
            }
            while (!str.empty() && (str.back() == ' ' || str.back() == '\t'
                || str.back() == '\n' || str.back() == '\r')) {
                str.remove_suffix(1);
            }
            return std::string(str);
        }
    }

    const Node::Value& Node::GetValue() const {
        return value_;
    }
    
    Node::Value& Node::GetValueRef() {
        return value_;
    }


    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsLUInt() const {
        return std::holds_alternative<long unsigned int>(value_);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    int Node::AsInt() const {
        using namespace std::literals;
        if (IsInt()){
            return std::get<int>(value_);
        }
            throw std::logic_error("no int in Node"s);
        
    }

    long unsigned int Node::AsLUInt() const {
        using namespace std::literals;
        if (IsLUInt()) {
            return std::get<long unsigned int>(value_);
        }
        throw std::logic_error("no long unsint in Node"s);

    }

    double Node::AsDouble() const {
        using namespace std::literals;
        if (IsPureDouble()) {
            return std::get<double>(value_);
        }
        else if (IsInt()) {
            return std::get<int>(value_) * 1.0;
        }
        else {
            throw std::logic_error("no double in Node"s);
        }

    }

    bool Node::AsBool() const {
        using namespace std::literals;
        if(IsBool()){
            return std::get<bool>(value_);
        }
        
            throw std::logic_error("no boo in Node"s);
        
    }

    const std::string& Node::AsString() const {
        using namespace std::literals;
        if(IsString()){
            return std::get<std::string>(value_);
        }
        
            throw std::logic_error("no string in Node"s);
        
    }

    const Array& Node::AsArray() const {
        using namespace std::literals;
        if(IsArray()){
            return std::get<Array>(value_);
        }
        
            throw std::logic_error("no array in Node"s);
        
    }

    const Dict& Node::AsMap() const {
        using namespace std::literals;
        if(IsMap()){
            return std::get<Dict>(value_);
        }
        
            throw std::logic_error("no map in Node"s);
        
    }

    bool Node::operator==(const Node& rhs) const {
        return value_ == rhs.value_;
    }

    bool Node::operator!=(const Node& rhs) const {
        return value_ != rhs.value_;
    }

   
namespace {
    Node LoadNode(std::istream& input);

Node LoadArray(std::istream& input) {
    Array result;
    char c = 'a';
    input >> c;
    if (c != '[') {
        throw ParsingError("Error: Expected '['");
    }
    input >> std::ws;
    c = input.peek();
    while (c != ']') {
        input >> std::ws;
        result.push_back(LoadNode(input));
        input >> std::ws;
        c = input.peek();
        if (c == ',') {
            input >> c; 
        }
        else if (input.peek() != ']') {
            throw ParsingError("Error: Expected ',' or ']'");
        }
    }
    input >> c;  

    return Node(move(result));
}

using Number = std::variant<int, double>;

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
                return Node(std::move(std::stoi(parsed_num)));
            }
            catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::move(std::stod(parsed_num)));
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}



Node LoadString(std::istream & input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        }
        else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        }
        else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(move(s));
}

Node LoadNull(std::istream& input) {
    using namespace std::literals;
    std::string text;
    getline(input, text, ',');
    text = Trim(text);
    if (text == "null"s) {
        input.putback(',');
        return Node(nullptr);
    }
    throw ParsingError("Failed to read nuul from stream"s);
}

Node LoadBool(std::istream& input) {
    using namespace std::literals;
    std::string text;
    getline(input, text, ',');
    auto brackets1_pos = text.find('}');
    auto brackets2_pos = text.find(']');
    if(brackets2_pos != std::string::npos) text.erase(brackets2_pos);
    if (brackets1_pos != std::string::npos) text.erase(brackets1_pos);
    text = Trim(text);
    if (text == "true"s) {
        input.putback(',');
        if (brackets2_pos != std::string::npos) input.putback(']');
        if (brackets1_pos != std::string::npos) input.putback('}');
        return Node(true);
    }
    else if (text == "false"s) {
       input.putback(',');
       if (brackets2_pos != std::string::npos) input.putback(']');
       if (brackets1_pos != std::string::npos) input.putback('}');
       return Node(false);
    }
    else {
        throw ParsingError("Failed to read bool from stream"s);
    }
}

// { \"key1\": \"value1\", \"key2\": 42 }
Node LoadDict(std::istream& input) {
    using namespace std::literals;
    Dict result;
    char c = 'a';
    input >> c;
    if (c != '{') {
        throw ParsingError("{ is expected for dict load"s);
    }
    c = input.peek();
    while (c != '}') {
        input >> std::ws;
        input >> c;
        if (c == '}') {
            input.putback(c);
            break;
        }
        std::string key = LoadString(input).AsString();
        input >> c;
        input >> std::ws;
        result[key] = LoadNode(input);
        input >> std::ws;
        c = input.peek();
        if (c == ',') {
            input >> c;
            input >> std::ws;
        }
        else if (input.peek() != '}') {
            throw ParsingError("Error: Expected ',' or '}'");
        }
    }
    input >> c; 

    return Node(move(result));
}

Node LoadNode(std::istream& input) {
    using namespace std::literals;
    char c = 'a';
    input >> c;

    if (c == '[') {
        input.putback(c);
        return LoadArray(input);
    } else if (c == '{') {
        input.putback(c);
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    }
    else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else if (isdigit(c) || c == '-') {
        input.putback(c);
        return LoadNumber(input);
    }
    else {
        throw ParsingError("Unexpected load stream"s);
    }
}

}  // namespace

void PrintValue(std::nullptr_t, std::ostream& out, [[maybe_unused]]  int indent_step) {
    using namespace std::literals;
    out << "null"sv;
}

void ParseString(const std::string& str, std::ostream& out) {
    std::string result;
    for (char c : str) {
        if (c == '\r') {        
            result += "\\r";
        }
        else if (c == '\n') { 
            result += "\\n";
        }
        else if (c == '\t') {
            result += "\t";     
        }
        else if (c == '\\') {
            result += "\\\\";   
        }
        else if (c == '\"') {
            result += "\\\"";   
        }
        else {
            result += c;
        }
    }
    out << "\"" << result << "\""; 
}
   
void PrintValue(const std::string& str, std::ostream& out, [[maybe_unused]]  int indent_step) {
    ParseString(str, out);
}

void PrintValue(bool bool_, std::ostream& out, [[maybe_unused]]  int indent_step) {
    out << std::boolalpha << bool_;    
}

void PrintIndent(std::ostream& out, [[maybe_unused]] int indent_step) {
    for (int i = 0; i < indent_step; ++i) {
        out << ' ';
    }
}

void PrintValue(const Array& array_, std::ostream& out, int indent_step) {
    using namespace std::literals;
    bool IsFirst = true;
    out << "["s << std::endl;
    for (const auto& node : array_) {
        if (!IsFirst) {
            out << ","s << std::endl;

        }
        else {
            IsFirst = false;
        }
        PrintIndent(out, indent_step);
        PrintNode(node, out, indent_step);

    }
    out << std::endl;
    PrintIndent(out, indent_step - 4);
    out << "]"s;
}

void PrintValue(const Dict& dict, std::ostream& out, int indent_step) {
    using namespace std::literals;
    out << "{"s << std::endl;
    bool IsFirst = true;
    for (const auto& [str, node] : dict) {
        if (!IsFirst) {
            out << ","s << std::endl;
        }
        else {
            IsFirst = false;
        }
        PrintIndent(out, indent_step);
        PrintNode(str, out, indent_step);
        out << ": "s;
        PrintNode(node, out, indent_step);
    }
    out << std::endl;
    PrintIndent(out, indent_step - 4);
    out << "}"s;
}

void PrintNode(const Node& node, std::ostream& out, int indent_step) {
    std::visit(
        [&out, indent_step](const auto& value) { PrintValue(value, out, indent_step + 4); },
        node.GetValue());

}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output, int indent_step) {
    PrintNode(doc.GetRoot(), output, indent_step);
}

bool Document::operator==(const Document& rhs) const {
    return GetRoot().GetValue() == rhs.GetRoot().GetValue();
}

bool Document::operator!=(const Document& rhs) const {
    return GetRoot().GetValue() != rhs.GetRoot().GetValue();
}

}  // namespace json