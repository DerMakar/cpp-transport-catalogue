#include "json_builder.h"


namespace json {
    
    Node Builder::Build() { // Вызов любого метода, кроме Build, при готовом объекте. - exception
            if (!end_of_object) {
                throw std::logic_error("you try Build right after constructor"s);
            }
            if (!nodes_stack.empty()) {
                throw std::logic_error("you try Build not ended object"s);
            }
            return root_;
        }

        //EndDict(). Завершает определение сложного значения-словаря. Последним незавершённым вызовом Start* должен быть StartDict.
            BaseContex Builder::EndDict() { // Вызов EndDict или EndArray в контексте другого контейнера. - exception
            if (nodes_stack.empty()) {
                throw std::logic_error("Dict was not even created"s);
            }
            if (!nodes_stack.back()->IsMap()) {
                throw std::logic_error("you try EndDict out of Dict"s);
            }
            root_ = *nodes_stack.back();
            nodes_stack.pop_back();
            if (nodes_stack.empty()) end_of_object = true;
            return BaseContex (*this);
        }

        //EndArray(). Завершает определение сложного значения-массива. Последним незавершённым вызовом Start* должен быть StartArray.
            BaseContex Builder::EndArray() { // Вызов EndDict или EndArray в контексте другого контейнера. - exception
            if (nodes_stack.empty()) {
                throw std::logic_error("Array was not even created"s);
            }
            if (!nodes_stack.back()->IsArray()) {
                throw std::logic_error("you try EndArray out of Array"s);
            }
            root_ = *nodes_stack.back();
            nodes_stack.pop_back();
            if (nodes_stack.empty()) end_of_object = true;
            return BaseContex (*this);
        }

            BaseContex Builder::Value(Node::Value value) { //Вызов Value, StartDict или StartArray где - либо, кроме как после конструктора, после Key или после предыдущего элемента массива - exception
                if (nodes_stack.empty()) {
                    if (end_of_object) {
                        throw std::logic_error("invalid use of Value()"s);
                    }
                    root_.GetValueRef() = move(value);
                    if (nodes_stack.empty()) end_of_object = true;
                }
                else if (nodes_stack.back()->IsArray()) {
                    size_t cur_size = nodes_stack.back()->AsArray().size();
                    std::get<Array>((*nodes_stack.back()).GetValueRef()).resize(cur_size + 1);
                    std::get<Array>((*nodes_stack.back()).GetValueRef()).back() = move(value);
                }
                else {
                    throw std::logic_error("invalid use of Value()"s);
                }
                return ArrayItemContext(*this);
            }
        // Value(Node::Value). Задаёт значение, соответствующее ключу при определении словаря, очередной элемент массива или,
        // если вызвать сразу после конструктора json::Builder, всё содержимое конструируемого JSON-объекта.
        // Может принимать как простой объект — число или строку — так и целый массив или словарь. Здесь Node::Value — это синоним для базового класса Node, шаблона variant с набором возможных типов - значений.
        DictItemContext Builder::KeyValue(Node::Value value) { //Вызов Value, StartDict или StartArray где - либо, кроме как после конструктора, после Key или после предыдущего элемента массива - exception
            const_cast<Node::Value&>(const_cast<Dict&>(nodes_stack.back()->AsMap())[key_].GetValue()) = move(value);
            key_ = "no_key"s;
            return DictItemContext (*this);
        }

        ArrayItemContext Builder::ArrayValue(Node::Value value) { //Вызов Value, StartDict или StartArray где - либо, кроме как после конструктора, после Key или после предыдущего элемента массива - exception
            if (nodes_stack.empty()) {
                if (end_of_object) {
                    throw std::logic_error("invalid use of Value()"s);
                }
                const_cast<Node::Value&>(root_.GetValue()) = move(value);
                if (nodes_stack.empty()) end_of_object = true;
            }
            else if (nodes_stack.back()->IsArray()) {
                size_t cur_size = nodes_stack.back()->AsArray().size();
                std::get<Array>((*nodes_stack.back()).GetValueRef()).resize(cur_size + 1);
                std::get<Array>((*nodes_stack.back()).GetValueRef()).back() = move(value);
            }
            else {
                throw std::logic_error("invalid use of Value()"s);
            }
            return ArrayItemContext (*this);
        
        }

        //StartDict(). Начинает определение сложного значения-словаря. Вызывается в тех же контекстах, что и Value. Следующим вызовом обязательно должен быть Key или EndDict.
        DictItemContext Builder::StartDict() { //Вызов Value, StartDict или StartArray где - либо, кроме как после конструктора, после Key или после предыдущего элемента массива - exception
            if (nodes_stack.empty()) {
                if (end_of_object) {
                    throw std::logic_error("you try to add to ready object"s);
                }
                nodes_stack.emplace_back(new Node(Dict()));
            }else if (nodes_stack.back()->IsArray() && !end_of_object) {
                auto& tmp = std::get<Array>((*nodes_stack.back()).GetValueRef());
                Node* node = &tmp.emplace_back(Node(Dict()));
                nodes_stack.push_back(node);
            }
            else if(nodes_stack.back()->IsMap() && !end_of_object) {
                std::get<Dict>((*nodes_stack.back()).GetValueRef())[key_] = Node(Dict());
                Node* node = &std::get<Dict>((*nodes_stack.back()).GetValueRef()).at(key_);
                nodes_stack.push_back(node);
                key_ = "no_key"s;
            }
            return DictItemContext( *this );
        }

        //Key(std::string). При определении словаря задаёт строковое значение ключа для очередной пары ключ-значение. 
        // Следующий вызов метода обязательно должен задавать соответствующее этому ключу значение с помощью метода Value или начинать его определение с помощью StartDict или StartArray
        ValueItemContextAfterKey Builder::Key(std::string key) { //Вызов метода Key снаружи словаря или сразу после другого Key. - exception
            key_ = key;
            return ValueItemContextAfterKey (*this);
        }

        //StartArray(). Начинает определение сложного значения-массива. Вызывается в тех же контекстах, что и Value. 
        //Следующим вызовом обязательно должен быть EndArray или любой, задающий новое значение: Value, StartDict или StartArray.
        ArrayItemContext Builder::StartArray() {//Вызов Value, StartDict или StartArray где - либо, кроме как после конструктора, после Key или после предыдущего элемента массива - exception
            if (nodes_stack.empty()) {
                if (end_of_object) {
                    throw std::logic_error("you try to add to ready object"s);
                }
                nodes_stack.emplace_back(new Node(Array()));
            }else if (nodes_stack.back()->IsMap() && !end_of_object) {
                std::get<Dict>((*nodes_stack.back()).GetValueRef())[key_] = Node(Array());
                Node* node = &std::get<Dict>((*nodes_stack.back()).GetValueRef()).at(key_);
                nodes_stack.push_back(node);
                key_ = "no_key"s;
                                
            }
            else if (nodes_stack.back()->IsArray() && !end_of_object) {
                Node* node = &std::get<Array>((*nodes_stack.back()).GetValueRef()).emplace_back(Node(Array()));
                nodes_stack.push_back(node);
                              
            }
            return ArrayItemContext( *this );
        }
        
        // методы BaseContex запускают Dict или Array
        Builder& BaseContex::GetBase() {
            return builder_;
        }
        
        DictItemContext BaseContex::StartDict() {
            return DictItemContext (builder_.StartDict().GetBase());
        }

        ValueItemContextAfterKey BaseContex::Key(std::string key) {
            return builder_.Key(key);
        }

        ArrayItemContext BaseContex::StartArray() {
            return ArrayItemContext(builder_.StartArray().GetBase());
        }

        ArrayItemContext BaseContex::Value(Node::Value value) {
            return ArrayItemContext(builder_.ArrayValue(value));
        }

        BaseContex BaseContex::EndDict() {
            return builder_.EndDict();
        }
        BaseContex BaseContex::EndArray() {
            return builder_.EndArray();
        }
        
        Node BaseContex::Build() {
            return builder_.Build();
        }
        // методы DictContex создают ключ или закрывают словарь
        ValueItemContextAfterKey DictItemContext::Key(std::string key) {
            return builder.Key(key);
        }

        BaseContex DictItemContext::EndDict() {
            return builder.EndDict();
        }

        // методы ValueContexAfterKey создают Value и возвращают в Dict, могут запусть Dict или Array от BaseContex
        DictItemContext ValueItemContextAfterKey::Value(Node::Value value) {
            return builder.KeyValue(value);
        }

        //методы ArrayItemContext
        BaseContex ArrayItemContext::EndArray() {
            return builder.EndArray();
        }

        ArrayItemContext ArrayItemContext::Value(Node::Value value) {
            return builder.ArrayValue(value);
        }


               
} // namespace json