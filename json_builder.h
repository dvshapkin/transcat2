#pragma once

#include <stack>
#include <optional>

#include "json.h"

namespace json {

    class KeyItemContext;
    class DictItemContext;
    class ArrayItemContext;

    class Builder {
    public:
        Builder() = default;
        Node Build() const;
        DictItemContext StartDict();
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
        KeyItemContext Key(const std::string & key);
        Builder& Value(const Node & value, bool is_start = false);

    private:
        Node root_ = nullptr;
        bool is_root_empty_ = true;
        std::stack<Node *> nodes_stack_;
        std::optional<std::string> key_;
    };

    class KeyValueContext {
    public:
        explicit KeyValueContext(Builder &builder);
        KeyItemContext Key(const std::string & key);
        Builder& EndDict();
    private:
        Builder &builder_;
    };

    class KeyItemContext {
    public:
        explicit KeyItemContext(Builder &builder);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        KeyValueContext Value(const Node & value, bool is_start = false);
    private:
        Builder &builder_;
    };

    class DictItemContext {
    public:
        explicit DictItemContext(Builder &builder);
        KeyItemContext Key(const std::string & key) const;
        Builder& EndDict() const;
    private:
        Builder &builder_;
    };

    class ArrayItemContext {
    public:
        explicit ArrayItemContext(Builder &builder);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
        ArrayItemContext Value(const Node & value, bool is_start = false);
    private:
        Builder &builder_;
    };

} // namespace json
