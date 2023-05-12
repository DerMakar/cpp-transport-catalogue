#pragma once

#include "json.h"
#include <stdexcept>

using namespace std::literals;
namespace json {
    class Builder;
    class DictItemContext;
    class ArrayItemContext;
    class ValueItemContextAfterKey;

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
        
        DictItemContext(Builder& builder_main) : BaseContex(builder_main), builder (builder_main){
            
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
        DictItemContext Value(Node::Value value); // вернет к возможности добавить ключ или закрыть словарь
        // может от родителя запустить Dict
        // может от родителя запустить Array
        BaseContex EndDict() = delete;
        BaseContex EndArray() = delete;
        Node Build() = delete;
        ValueItemContextAfterKey Key(std::string key) = delete;

       

    private:
        Builder& builder;

    };

    class ArrayItemContext : public BaseContex {
    public:
        
        ArrayItemContext(Builder& builder_main) : BaseContex(builder_main), builder(builder_main) {
            
        }
        
        BaseContex EndArray(); // закроет массив и вернет к BC
        ArrayItemContext Value(Node::Value value); // позволит положить Value в массив
        // может от родителя запустить Dict
        // может от родителя запусть Array
        ValueItemContextAfterKey Key(std::string key) = delete;
        BaseContex EndDict() = delete; // не может закрывать словари
        Node Build() = delete;
    private:
        Builder& builder;

    };

    class Builder {
        
    public:
        Node Build();

        BaseContex EndDict();

        BaseContex EndArray();

        BaseContex Value(Node::Value value);
                
        DictItemContext KeyValue(Node::Value value);
        
        ArrayItemContext ArrayValue(Node::Value value);

        DictItemContext StartDict();

        ValueItemContextAfterKey Key(std::string key);

        ArrayItemContext StartArray();

                     

    private:
        // храним текущее описываемое значение и цепочка его родителей.
        // Он поможет возвращаться в нужный контекст после вызова End - методов.
        Node root_;
        std::vector<Node*> nodes_stack; // При реализации обратите внимание на метод emplace_back у вектора: 
        bool end_of_object = false;                 //в отличие от push_back он принимает не сам добавляемый объект, а аргументы конструктора этого объекта
        std::string key_ = "no_key"s;
    };

    
    
} // namespace json