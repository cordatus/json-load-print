#include "json.h"

namespace json {

    enum class JsonContainerType {
        kArray,
        kDict
    };

    class KeyItemContext;
    class KeyValueItemContext;
    class DictItemContext;    
    class ValueItemContext;
    class ArrayItemContext;
    class ArrayValueItemContext;    

    class Builder{
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::vector<Node*> start_array_call_point_;
        std::vector<Node*> start_dict_call_point_;
        std::vector<JsonContainerType> start_container_call_;
        bool key_called_;

    public:        
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Builder& Value(json::CurrentNode);
        KeyItemContext Key(std::string);
        Node Build();

        inline bool IsObjectReady();
        inline bool IsObjectNotReady();
        inline bool IsAcceptableCall();
    };

    class DictItemContext {
    private:
        Builder& builder_;

    public:
        DictItemContext(Builder& builder)
        :builder_(builder) {

        }
        KeyItemContext Key(std::string key);
        Builder& EndDict();
    };    

    class KeyItemContext {
    private:
        Builder& builder_;

    public:
        KeyItemContext(Builder& builder)
        :builder_(builder) {

        }
        KeyValueItemContext Value(json::CurrentNode value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    };

    class KeyValueItemContext {
    private:
        Builder& builder_;

    public:
        KeyValueItemContext(Builder& builder)
        :builder_(builder) {

        }
        KeyItemContext Key(std::string key);
        Builder& EndDict();
    };    

    class ArrayItemContext {
    private:
        Builder& builder_;

    public:
        ArrayItemContext(Builder& builder)
        :builder_(builder) {

        }
        ArrayValueItemContext Value(json::CurrentNode value);
        Builder& EndArray();
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    };

    class ArrayValueItemContext {
    private:
        Builder& builder_;

    public:
        ArrayValueItemContext(Builder& builder)
        :builder_(builder) {

        }
        ArrayValueItemContext Value(json::CurrentNode value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
    };
}