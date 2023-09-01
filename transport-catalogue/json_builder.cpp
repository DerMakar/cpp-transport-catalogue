#include "json_builder.h"


namespace json {
    using namespace std::literals;

    Node Builder::Build() { 
            if (!end_of_object) {
                throw std::logic_error("you try Build right after constructor"s);
            }
            if (!nodes_stack.empty()) {
                throw std::logic_error("you try Build not ended object"s);
            }
            return root_;
        }

    Builder::BaseContex Builder::EndDict() {
        if (nodes_stack.empty()) {
            throw std::logic_error("Dict was not even created"s);
        }
        if (!nodes_stack.back()->IsMap()) {
            throw std::logic_error("you try EndDict out of Dict"s);
        }
        root_ = *nodes_stack.back();
        nodes_stack.pop_back();
        if (nodes_stack.empty()) end_of_object = true;
        return  Builder::BaseContex (*this);
    }

    Builder::BaseContex Builder::EndArray() {
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

    Builder::BaseContex Builder::Value(Node::Value value) { 
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
        else if (nodes_stack.back()->IsMap()) {
            const_cast<Node::Value&>(const_cast<Dict&>(nodes_stack.back()->AsMap())[key_.value()].GetValue()) = move(value);
            key_.reset();
        }
        else {
            throw std::logic_error("invalid use of Value()"s);
        }
        return  Builder::BaseContex(*this);
    }
        
    Node Builder::Start(const Node& root_node) {
        if (nodes_stack.empty()) {
            if (end_of_object) {
                throw std::logic_error("you try to add to ready object"s);
            }
            nodes_stack.emplace_back(new Node(root_node));
        }
        else if (nodes_stack.back()->IsArray() && !end_of_object) {
            auto& tmp = std::get<Array>((*nodes_stack.back()).GetValueRef());
            Node* node = &tmp.emplace_back(Node(root_node));
            nodes_stack.push_back(node);
        }
        else if (nodes_stack.back()->IsMap() && !end_of_object) {
            std::get<Dict>((*nodes_stack.back()).GetValueRef())[key_.value()] = Node(root_node);
            Node* node = &std::get<Dict>((*nodes_stack.back()).GetValueRef()).at(key_.value());
            nodes_stack.push_back(node);
            key_.reset();
        }
        return  *nodes_stack.back();
    }
        
    Builder::DictItemContext Builder::StartDict() { 
        Start(Dict());
        return Builder::DictItemContext(*this);
    }

    Builder::ValueItemContextAfterKey Builder::Key(std::string key) { 
        key_ = key;
        return  Builder::ValueItemContextAfterKey (*this);
    }

    Builder::ArrayItemContext Builder::StartArray() {
        Start(Array());
        return  Builder::ArrayItemContext( *this );
    }
        
    Builder& Builder::BaseContex::GetBase() {
        return builder_;
    }
        
    Builder::DictItemContext Builder::BaseContex::StartDict() {
        return DictItemContext (builder_.StartDict().GetBase());
    }

    Builder::ValueItemContextAfterKey Builder::BaseContex::Key(std::string key) {
        return builder_.Key(key);
    }

    Builder::ArrayItemContext Builder::BaseContex::StartArray() {
        return ArrayItemContext(builder_.StartArray().GetBase());
    }

    Builder::ArrayItemContext Builder::BaseContex::Value(Node::Value value) {
        return Builder::ArrayItemContext(builder_.Value(value));
    }

    Builder::BaseContex Builder::BaseContex::EndDict() {
        return builder_.EndDict();
    }
        
    Builder::BaseContex Builder::BaseContex::EndArray() {
        return builder_.EndArray();
    }
        
    Node Builder::BaseContex::Build() {
        return builder_.Build();
    }
       
    Builder::ValueItemContextAfterKey Builder::DictItemContext::Key(std::string key) {
        return builder.Key(key);
    }

    Builder::BaseContex Builder::DictItemContext::EndDict() {
        return builder.EndDict();
    }

    Builder::DictItemContext Builder::ValueItemContextAfterKey::Value(Node::Value value) {
        return Builder::DictItemContext(builder.Value(value));
    }
           
    Builder::BaseContex Builder::ArrayItemContext::EndArray() {
        return builder.EndArray();
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        return Builder::ArrayItemContext(builder.Value(value));
    }
          
} // namespace json