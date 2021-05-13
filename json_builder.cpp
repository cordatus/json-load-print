#include "json_builder.h"

using namespace std::literals;

namespace json {
    DictItemContext Builder::StartDict() {
        if (IsObjectReady()) {
            throw std::logic_error("Call not Build method for ready object"s);
        }
        if (!IsAcceptableCall()) {
            throw std::logic_error("Wrong call of StartDict"s);
        }
		nodes_stack_.push_back(new Node(Dict()));
        start_dict_call_point_.push_back(nodes_stack_.back());
        key_called_ = false;        
        start_container_call_.push_back(JsonContainerType::kDict);
		return DictItemContext(*this); 
    }

    ArrayItemContext Builder::StartArray() {
        if (IsObjectReady()) {
            throw std::logic_error("Call not Build method for ready object"s);
        }
        if (!IsAcceptableCall()) {
            throw std::logic_error("Wrong call StartArray"s);
        }
		nodes_stack_.push_back(new Node(Array()));
        start_array_call_point_.push_back(nodes_stack_.back());
        key_called_ = false;
        start_container_call_.push_back(JsonContainerType::kArray);
		return ArrayItemContext(*this); 
    }
    Builder& Builder::EndDict() {
        if (IsObjectReady()) {
            throw std::logic_error("Call not Build method for ready object"s);
        }
        if (start_container_call_.back() != JsonContainerType::kDict) {
            throw std::logic_error("EndDict in wrong position"s);
        }
        Node* d_end = start_dict_call_point_.back();
        Node* ptr_v = nodes_stack_.back();
        Node* ptr_k;
        Dict dict;
        while (ptr_v != d_end) { 
            nodes_stack_.pop_back();
            ptr_k = nodes_stack_.back();
            nodes_stack_.pop_back();
            dict.insert({std::move(ptr_k->AsString()), std::move(*ptr_v)});
            delete ptr_v;
            delete ptr_k;
            ptr_v = nodes_stack_.back();
        }

        delete ptr_v;
        nodes_stack_.pop_back();
        start_dict_call_point_.pop_back();
        nodes_stack_.push_back(new Node(dict));       
        start_container_call_.pop_back();

        return *this;
    }
    Builder& Builder::EndArray() {
        if (IsObjectReady()) {
            throw std::logic_error("Call not Build method for ready object"s);
        }
        if (start_container_call_.back() != JsonContainerType::kArray) {
            throw std::logic_error("EndArray in wrong position"s);
        }
        Node* a_end = start_array_call_point_.back();
        Node* ptr = nodes_stack_.back();
        Array array;
        while ( ptr != a_end) {
            array.push_back(*ptr);
            delete ptr;
            nodes_stack_.pop_back();
            ptr = nodes_stack_.back();
        }

        delete ptr;
        nodes_stack_.pop_back();
        start_array_call_point_.pop_back();
        nodes_stack_.push_back(new Node(Array(array.rbegin(), array.rend())));
        start_container_call_.pop_back();

        return *this;
    }

    Builder& Builder::Value(json::CurrentNode value) {
        if (IsObjectReady()) {
            throw std::logic_error("Call not Build method for ready object"s);
        }
        if (!IsAcceptableCall()) {
            throw std::logic_error("Wrong call of Value"s);
        }
        std::visit([&](json::CurrentNode&& arg) {
        if (std::holds_alternative<std::nullptr_t>(arg)) {
            nodes_stack_.push_back(new Node(nullptr));
        } else if (std::holds_alternative<Array>(arg)) {
            nodes_stack_.push_back(new Node(std::move(std::get<Array>(arg))));
        } else if (std::holds_alternative<Dict>(arg)) {
            nodes_stack_.push_back(new Node(std::move(std::get<Dict>(arg))));
        } else if (std::holds_alternative<bool>(arg)) {
            nodes_stack_.push_back(new Node(std::get<bool>(arg)));
        } else if (std::holds_alternative<int>(arg)) {
            nodes_stack_.push_back(new Node(std::get<int>(arg)));
        } else if (std::holds_alternative<double>(arg)) {
            nodes_stack_.push_back(new Node(std::get<double>(arg)));
        } else if (std::holds_alternative<std::string>(arg)) {
            nodes_stack_.push_back(new Node(std::move(std::get<std::string>(arg))));
        }
        }, value);

        key_called_ = false;
		return *this;
    }
    KeyItemContext Builder::Key(std::string key) {
        if (IsObjectReady()) {
            throw std::logic_error("Call not Build method for ready object"s);
        }
        if (key_called_ || start_dict_call_point_.size() == 0) {
            throw std::logic_error("Wrong Key method called"s);
        }
        nodes_stack_.push_back(new Node(std::move(key)));
        key_called_ = true;
		return *this; 
    }
    Node Builder::Build() {
        if (IsObjectNotReady()) {
            throw std::logic_error("Wrong Build call"s);
        }
        Node* tmp = nodes_stack_.back();
        root_ = std::move(*tmp);
        nodes_stack_.pop_back();
        delete tmp;
        return  root_;
    }

    inline bool Builder::IsObjectReady() {
        return nodes_stack_.size() == 1 && start_array_call_point_.size() == 0 && start_dict_call_point_.size() == 0;
    }

    inline bool Builder::IsObjectNotReady() {
        return nodes_stack_.size() == 0 || start_dict_call_point_.size() != 0 || start_array_call_point_.size() != 0;
    }

    inline bool Builder::IsAcceptableCall() {
        return nodes_stack_.size() == 0 || key_called_ || start_array_call_point_.size() != 0;
    }   

    KeyItemContext DictItemContext::Key(std::string key) {
            return builder_.Key(key);
        }

    Builder& DictItemContext::EndDict() {
            return builder_.EndDict();
        }

    KeyValueItemContext KeyItemContext::Value(json::CurrentNode value) {
            return KeyValueItemContext(builder_.Value(value));
        }
    
    DictItemContext KeyItemContext::StartDict() {
            return builder_.StartDict();
        }

    ArrayItemContext KeyItemContext::StartArray() {
            return builder_.StartArray();
        }

    KeyItemContext KeyValueItemContext::Key(std::string key) {
            return builder_.Key(key);
        }

    Builder& KeyValueItemContext::EndDict() {
            return builder_.EndDict();
        }

    ArrayValueItemContext ArrayItemContext::Value(json::CurrentNode value) {
            return ArrayValueItemContext(builder_.Value(value));
        }

    Builder& ArrayItemContext::EndArray() {
            return builder_.EndArray();
        }

    DictItemContext ArrayItemContext::StartDict() {
            return builder_.StartDict();
        }

    ArrayItemContext ArrayItemContext::StartArray() {
            return builder_.StartArray();
        }

    ArrayValueItemContext ArrayValueItemContext::Value(json::CurrentNode value) {
            return ArrayValueItemContext(builder_.Value(value));
        }

    DictItemContext ArrayValueItemContext::StartDict() {
            return builder_.StartDict();
        }

    ArrayItemContext ArrayValueItemContext::StartArray() {
            return builder_.StartArray();
        }

    Builder& ArrayValueItemContext::EndArray() {
            return builder_.EndArray();
        }
}