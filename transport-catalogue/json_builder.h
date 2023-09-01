#pragma once

#include "json.h"
#include <stdexcept>
#include <optional>


namespace json {
        class Builder {
        class BaseContex;
        class DictItemContext;
        class ArrayItemContext;
        class ValueItemContextAfterKey;

    private:
        class BaseContex {

        public:
            BaseContex(Builder& builder) : builder_(builder) {
            }
            Builder& GetBase();
            DictItemContext StartDict();
            ValueItemContextAfterKey Key(std::string key);
            ArrayItemContext StartArray();
            ArrayItemContext Value(Node::Value value);
            BaseContex EndDict();
            BaseContex EndArray();
            Node Build();

        private:
            Builder& builder_;
        };

        class DictItemContext : public BaseContex {
        public:
            DictItemContext(BaseContex base) : BaseContex(base), builder(base.GetBase()) {
            }

            DictItemContext(Builder& builder_main) : BaseContex(builder_main), builder(builder_main) {
            }
            ValueItemContextAfterKey Key(std::string key);
            BaseContex EndDict();
            ArrayItemContext StartArray() = delete;
            DictItemContext StartDict() = delete;
            BaseContex EndArray() = delete;
            ArrayItemContext Value(Node::Value value) = delete;
            Node Build() = delete;
        private:
            Builder& builder;
        };

        class ValueItemContextAfterKey : public BaseContex {
        public:
            ValueItemContextAfterKey(Builder& builder_main) : BaseContex(builder_main), builder(builder_main) {
            }
            DictItemContext Value(Node::Value value); 
            BaseContex EndDict() = delete;
            BaseContex EndArray() = delete;
            Node Build() = delete;
            ValueItemContextAfterKey Key(std::string key) = delete;
        
        private:
            Builder& builder;
        };
        
        class ArrayItemContext : public BaseContex {
        public:
            ArrayItemContext(BaseContex base) : BaseContex(base), builder(base.GetBase()) {
            }

            ArrayItemContext(Builder& builder_main) : BaseContex(builder_main), builder(builder_main) {
            }

            BaseContex EndArray(); 
            ArrayItemContext Value(Node::Value value); 
            ValueItemContextAfterKey Key(std::string key) = delete;
            BaseContex EndDict() = delete; 
            Node Build() = delete;
        private:
            Builder& builder;
        };
        
        public:
            Node Build();
            BaseContex EndDict();
            BaseContex EndArray();
            BaseContex Value(Node::Value value);
            DictItemContext StartDict();
            ValueItemContextAfterKey Key(std::string key);
            ArrayItemContext StartArray();

    private:
        Node root_;
        std::vector<Node*> nodes_stack; 
        bool end_of_object = false;              
        std::optional<std::string> key_;
        Node Start(const Node& node);
    };
} // namespace json