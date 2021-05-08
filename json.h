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
    using CurrentNode = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    struct NodePrinter {
        
        std::ostream& out;
        void operator()(std::nullptr_t) const;    
        void operator()(Array array) const;    
        void operator()(Dict dict) const;    
        void operator()(bool value) const;    
        void operator()(int value) const;    
        void operator()(double value) const;    
        void operator()(std::string str) const;    
    };

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        Node() 
            : value_(nullptr) {
        }
        template <typename Value>
        Node(Value value)
            : value_(std::move(value)) {
        }

        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;    
        void Print(std::ostream& output) const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;
        friend bool operator==(const Node& lhs, const Node& rhs);
        friend bool operator!=(const Node& lhs, const Node& rhs);

    private:
        CurrentNode value_;  
    };

    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;
        friend bool operator==(const Document& lhs, const Document& rhs);
        friend bool operator!=(const Document& lhs, const Document& rhs);

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

} // namespace json