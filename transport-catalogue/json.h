#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <exception>
#include <typeinfo>
#include <cmath>
#include <stdexcept>


namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


class Node {
public:
    
    using Value = std::variant<std::nullptr_t, Array, Dict, int, double, std::string, bool, long unsigned int>;
    Node() = default;
    
    template <typename V>
    Node(V value) : value_(value) {
    }

    const Value& GetValue() const; 

    Value& GetValueRef();
    
    bool IsInt() const;

    bool IsLUInt() const;

    bool IsDouble() const;

    bool IsPureDouble() const;

    bool IsBool() const;

    bool IsString() const;

    bool IsNull() const;

    bool IsArray() const;

    bool IsMap() const;

    int AsInt() const;

    long unsigned int AsLUInt() const;
    
    double AsDouble() const;
    
    bool AsBool() const;

    const std::string& AsString() const;

    const Array& AsArray() const;

    const Dict& AsMap() const;

    bool operator==(const Node& rhs) const;

    bool operator!=(const Node& rhs) const;

private:
    Value value_;
};

template <typename Value>
void PrintValue(const Value& value, std::ostream& out, [[maybe_unused]] int indent_step) {
    out << value;
}

void PrintValue(std::nullptr_t, std::ostream& out, [[maybe_unused]] int indent_step);

void PrintValue(const std::string& str, std::ostream& out, [[maybe_unused]] int indent_step);

void PrintValue(bool bool_, std::ostream& out, [[maybe_unused]] int indent_step);

void PrintIndent(std::ostream& out, int indent_step);

void PrintNode(const Node& node, std::ostream& out, int indent_step);

void PrintValue(const Array& array_, std::ostream& out, int indent_step);

void ParseString(const std::string& str, std::ostream& out);

void PrintValue(const Dict& dict, std::ostream& out, int indent_step);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& rhs) const;

    bool operator!=(const Document& rhs) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output, int indent_step = 0);

}  // namespace json