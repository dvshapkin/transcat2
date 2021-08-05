#include "json_builder.h"

namespace json {

    Node Builder::Build() const {
        if (is_root_empty_ || !nodes_stack_.empty()) {
            throw std::logic_error("Json is not ready for build");
        }
        return root_;
    }

    DictItemContext Builder::StartDict() {
        return DictItemContext(Value(Dict{}, true));
    }

    Builder &Builder::EndDict() {
        if (!is_root_empty_ && nodes_stack_.empty()) {
            throw std::logic_error("Json is ready now");
        }
        if (key_.has_value()) {
            throw std::logic_error("Dict key without value");
        } else if (nodes_stack_.empty() || !nodes_stack_.top()->IsDict()) {
            throw std::logic_error("Dict end without start");
        }
        nodes_stack_.pop();
        return *this;
    }

    ArrayItemContext Builder::StartArray() {
        return ArrayItemContext(Value(Array {}, true));
    }

    Builder &Builder::EndArray() {
        if (!is_root_empty_ && nodes_stack_.empty()) {
            throw std::logic_error("Json is ready now");
        }
        if (nodes_stack_.empty() || !nodes_stack_.top()->IsArray()) {
            throw std::logic_error("Array end without start");
        }
        nodes_stack_.pop();
        return *this;
    }

    KeyItemContext Builder::Key(const std::string &key) {
        if (!is_root_empty_ && nodes_stack_.empty()) {
            throw std::logic_error("Json is ready now");
        }
        if (nodes_stack_.empty() || !nodes_stack_.top()->IsDict() || key_.has_value()) {
            throw std::logic_error("Key is in an unexpected place");
        }
        key_ = key;
        return KeyItemContext(*this);
    }

    Builder &Builder::Value(const Node & value, bool is_start) {
        if (!is_root_empty_ && nodes_stack_.empty()) {
            throw std::logic_error("Json is ready now");
        }

        if (!(is_root_empty_ || key_.has_value() || nodes_stack_.top()->IsArray())) {
            throw std::logic_error("Bad place for new json element");
        }

        if (is_root_empty_) {
            root_ = value;
            is_root_empty_ = false;
            if (is_start && (value.IsDict() || value.IsArray())) {
                nodes_stack_.push(&root_);
            }
        } else if (!nodes_stack_.empty() && nodes_stack_.top()->IsDict()) {
            Dict& dict = const_cast<Dict &>(nodes_stack_.top()->AsDict());
            auto new_val = dict.insert({*key_, value});
            if (new_val.second && is_start && (value.IsDict() || value.IsArray())) {
                nodes_stack_.push(&(new_val.first->second));
            }
            key_.reset();
        } else if (!nodes_stack_.empty() && nodes_stack_.top()->IsArray()) {
            Array& arr = const_cast<Array &>(nodes_stack_.top()->AsArray());
            arr.push_back(value);
            if (is_start && (value.IsDict() || value.IsArray())) {
                nodes_stack_.push(&arr.back());
            }
        }
        return *this;
    }

    ////////////////     KeyValueContext        //////////////

    KeyValueContext::KeyValueContext(Builder &builder) : builder_(builder) {}

    KeyItemContext KeyValueContext::Key(const std::string &key) {
        return KeyItemContext(builder_.Key(key));
    }

    Builder &KeyValueContext::EndDict() {
        return builder_.EndDict();
    }

    ////////////////     KeyItemContext        //////////////

    KeyItemContext::KeyItemContext(Builder &builder) : builder_(builder) {}

    DictItemContext KeyItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext KeyItemContext::StartArray() {
        return builder_.StartArray();
    }

    KeyValueContext KeyItemContext::Value(const Node &value, bool is_start) {
        return KeyValueContext(builder_.Value(value, is_start));
    }

    ////////////////     DictItemContext        //////////////

    DictItemContext::DictItemContext(Builder &builder) : builder_(builder) {}

    KeyItemContext DictItemContext::Key(const std::string &key) const {
        return builder_.Key(key);
    }

    Builder &DictItemContext::EndDict() const {
        return builder_.EndDict();
    }

    ////////////////     ArrayItemContext       //////////////

    ArrayItemContext::ArrayItemContext(Builder &builder) : builder_(builder) {}

    DictItemContext ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder &ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

    ArrayItemContext ArrayItemContext::Value(const Node &value, bool is_start) {
        return ArrayItemContext(builder_.Value(value, is_start));
    }

} // namespace json
