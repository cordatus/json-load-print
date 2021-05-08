#include "json.h"

#include <iomanip>
#include <string_view>

using namespace std;

namespace json {
    namespace {

        bool IsLowercaseLetter(char c) {
            return c >= 'a' && c <= 'z';
        }

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c;
            while (input >> c && c != ']') {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (c != ']') {
                throw ParsingError("Invalid array"s);
            }
            return Node(move(result));
        }

        Node LoadSpecialValue(istream& input) {    
            string result_str;
            char c;
            while(!input.eof()) {
                c = input.peek();
                if (!IsLowercaseLetter(c)) {
                    break;
                }
                c = input.get();
                result_str += c;
            }
            if (result_str == "false"s) {
                return Node(false);
            } else if (result_str == "true"s) {
                return Node(true);
            } else if (result_str == "null"s) {
                return Node();
            } else {
                throw ParsingError("Invalid special value"s);
            }
        }

        Node LoadNumber(istream& input) {    

            string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };
            
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
            
            if (input.peek() == '0') {
                read_char();                
            } else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }
            
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
                    try {
                        return Node(std::stoi(parsed_num));
                    } catch (...) {
                    }
                }
                return Node(std::stod(parsed_num));
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream& input) {
            char c = ' ';
            char previous_c = '\\';
            string line;
            while(!input.eof()) {
                c = input.get();
                if (c == '\\' && !input.eof()) {
                    c = input.get();
                    if (c == 'r') {
                        line += '\r';
                    } else if (c == 'n') {
                        line += '\n';
                    } else if (c == 't') {
                        line += '\t';
                    } else if (c == '\"') {
                        line += '\"';
                    } else {
                        line += '\\';
                    }
                    previous_c = c;
                } else 
                if (c == '\"' && previous_c != '\\') {
                    break;
                } else {
                    line += c;
                    previous_c = c;
                }        
            }
            
            if (c != '\"') {
                throw ParsingError("Invalid string"s);
            }
            return Node(move(line));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;
            while (input >> c && c != '}') {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }

            if (c != '}') {
                throw ParsingError("Invalid dictionary"s);
            }
            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 't' || c == 'f' || c == 'n') {
                input.putback(c);
                return LoadSpecialValue(input);
            } else if (c == '-' || std::isdigit(c)) {
                input.putback(c);
                return LoadNumber(input);
            } else {
                throw ParsingError("Invalid JSON"s);
            }
        }
    } //namespace

    void NodePrinter::operator()(Array array) const {
            bool is_first = true;
            out << "["sv;
            for (const Node& node: array) {
                if (is_first){
                    node.Print(out);
                    is_first = false;
                } else {
                    out << ","sv;
                    node.Print(out);
                }                
            }
            out << "]"sv;
        }

    void NodePrinter::operator()(Dict dict) const {
            bool is_first = true;
            out << "{"sv;
            for (const auto& [key, node]: dict) {
                if (is_first){
                    out << "\""sv << key << "\""sv << ":"sv;
                    node.Print(out);
                    is_first = false;
                } else {
                    out << ", \""sv << key << "\""sv << ":"sv;
                    node.Print(out);
                }                
            }
            out << "}"sv;
        }

    void NodePrinter::operator()(std::string str) const {
        std::string output_string = "\""s;
        for ( char c: str) {
            if (c == '\\' || c == '"') {
                output_string += '\\';                
            } else if (c == '\n') {
                output_string += '\\';
                c = 'n';
            } else if (c == '\t') {
                output_string += '\\';
                c = 't';
            } else if (c == '\r') {
                output_string += '\\';
                c = 'r';
            }
            output_string += c;
        }
        output_string += "\""s;
        out << output_string;
    }

    void NodePrinter::operator()(bool value) const {
        out << std::boolalpha << value << std::noboolalpha ;
        }
        
    void NodePrinter::operator()(int value) const {
        out << value;
    }
        
    void NodePrinter::operator()(double value) const {
        out << value;
    }

    void NodePrinter::operator()(std::nullptr_t) const {
        out << "null"sv;
    }

    void Node::Print(std::ostream& output) const {
        std::visit(NodePrinter{output}, value_);
    }

    bool Node::IsNull() const {      
        if (std::get_if<std::nullptr_t>(&value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsInt() const {
        if (std::get_if<int>(&value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsDouble() const {
        if (std::get_if<int>(&value_) || std::get_if<double>(&value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsPureDouble() const {
        if (std::get_if<double>(&value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsBool() const {
        if (std::get_if<bool>(&value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsString() const {
        if (std::get_if<string>(&value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsArray() const {
        if (std::get_if<Array>(&value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsMap() const {
        if (std::get_if<Dict>(&value_)) {
            return true;
        }
        return false;
    }

    const Array& Node::AsArray() const {
        const Array* ptr = std::get_if<Array>(&value_);
        if (!ptr) {
            throw std::invalid_argument("Wrong variant"s);
        }
            return *ptr;
        }

    const Dict& Node::AsMap() const {
        const Dict* ptr = std::get_if<Dict>(&value_);
        if (!ptr) {
            throw std::invalid_argument("Wrong variant"s);
        }
        return *ptr;
    }

    int Node::AsInt() const {
        const int* ptr = std::get_if<int>(&value_);
        if (!ptr) {
            throw std::invalid_argument("Wrong variant"s);
        }
        return *ptr;
    }

    double Node::AsDouble() const {
        if (IsInt()) {
        const int* ptr = std::get_if<int>(&value_);
            if (!ptr) {
                throw std::invalid_argument("Wrong variant"s);
            }  
        return static_cast<double>(*ptr);
        } else {
            const double* ptr = std::get_if<double>(&value_); 
            if (!ptr) {
                throw std::invalid_argument("Wrong variant"s);
            } 
            return *ptr;
        }        
    }

    bool Node::AsBool() const {
        const bool* ptr = std::get_if<bool>(&value_);
        if (!ptr) {
            throw std::invalid_argument("Wrong variant"s);
        }
        return *ptr;
    }

    const string& Node::AsString() const {
        const string* ptr = std::get_if<string>(&value_);
        if (!ptr) {
            throw std::invalid_argument("Wrong variant"s);
        }
        return *ptr;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void Print(const Document& doc, std::ostream& output) {
        doc.GetRoot().Print(output);
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.value_ == rhs.value_;
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return rhs.root_ == lhs.root_;
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }
} //namespace json