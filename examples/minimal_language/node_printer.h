#pragma once

#include "source/grammar.h"
#include <iostream>
#include <string>
#include <string_view>

namespace language {

/*
Visual ASCII Tree Printer
*/
inline void print_ast(const Node& node, 
                      const std::string& prefix = "", 
                      bool is_last = true) {
    std::cout << prefix;
    std::cout << (is_last ? "└── " : "├── ");

    // Print Rule Name
    if (!node.rule_name.empty()) {
        std::cout << "\033[1;36m" << node.rule_name << "\033[0m"; // Cyan text
    } else {
        std::cout << "\033[1;33m[UnnamedNode]\033[0m";
    }

    // Print Matched Text Slice
    std::cout << " -> \"\033[32m" << node.match_text << "\033[0m\"";

    // Print Child Count if present
    if (!node.children.empty()) {
        std::cout << " \033[90m(" << node.children.size() << " children)\033[0m";
    }
    std::cout << "\n";

    // Recursively print children with tree connectors
    for (size_t i = 0; i < node.children.size(); ++i) {
        bool last_child = (i == node.children.size() - 1);
        std::string new_prefix = prefix + (is_last ? "    " : "│   ");
        print_ast(node.children[i], new_prefix, last_child);
    }
}

/*
JSON Serializer Utility
*/
inline std::string escape_json_string(std::string_view sv) {
    std::string out;
    out.reserve(sv.size());
    for (char c : sv) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

inline std::string to_json(const Node& node, int indent_level = 0) {
    std::string indent(indent_level * 2, ' ');
    std::string child_indent((indent_level + 1) * 2, ' ');

    std::string json = "{\n";
    json += child_indent + "\"rule_name\": \"" + escape_json_string(node.rule_name) + "\",\n";
    json += child_indent + "\"match_text\": \"" + escape_json_string(node.match_text) + "\"";

    if (!node.children.empty()) {
        json += ",\n" + child_indent + "\"children\": [\n";
        for (size_t i = 0; i < node.children.size(); ++i) {
            json += child_indent + "  " + to_json(node.children[i], indent_level + 2);
            if (i + 1 < node.children.size()) {
                json += ",";
            }
            json += "\n";
        }
        json += child_indent + "]\n";
    } else {
        json += "\n";
    }

    json += indent + "}";
    return json;
}

} // namespace language