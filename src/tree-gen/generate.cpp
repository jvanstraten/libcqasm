#include <ios>
#include <fstream>
#include <iostream>
#include "generate.hpp"

/**
 * Formats a docstring.
 */
static void format_doc(
    std::ofstream &stream,
    const std::string &doc,
    const std::string &indent = ""
) {
    stream << indent << "/**" << std::endl;
    auto word = std::ostringstream();
    auto line = std::ostringstream();
    line << indent << " *";
    bool line_empty = true;
    for (char c : doc) {
        bool flush = false;
        if (c == '\n') {
            line << " " << word.str();
            word = std::ostringstream();
            flush = true;
        } else if (c == ' ') {
            line << " " << word.str();
            line_empty = false;
            word = std::ostringstream();
        } else {
            word << c;
            flush = !line_empty && line.str().size() + word.str().size() > 79;
        }
        if (flush) {
            stream << line.str() << std::endl;
            line = std::ostringstream();
            line << indent << " *";
            line_empty = true;
        }
    }
    if (!word.str().empty()) {
        line << " " << word.str();
        line_empty = false;
    }
    if (!line_empty) {
        stream << line.str() << std::endl;
    }
    stream << indent << " */" << std::endl;
}

/**
 * Generates the class for the given node.
 */
static void generate_node_class(
    std::ofstream &header,
    std::ofstream &source,
    NodeType &node
) {

    // Print class header.
    if (!node.doc.empty()) {
        format_doc(header, node.doc);
    }
    header << "class " << node.title_case_name << ": public ";
    if (node.parent) {
         header << node.parent->title_case_name;
    } else {
         header << "Node";
    }
    header << " {" << std::endl;
    header << "public:" << std::endl << std::endl;

    // Print children.
    for (auto &child : node.children) {
        if (!child.doc.empty()) {
            format_doc(header, child.doc, "    ");
        }
        header << "    ";
        switch (child.type) {
            case Maybe:
                header << "Maybe<" << child.node_type->title_case_name << "> ";
                break;
            case One:
                header << "One<" << child.node_type->title_case_name << "> ";
                break;
            case Any:
                header << "Any<" << child.node_type->title_case_name << "> ";
                break;
            case Many:
                header << "Many<" << child.node_type->title_case_name << "> ";
                break;
            case Str:
                header << "Str ";
                break;
            case Int:
                header << "Int ";
                break;
            case Real:
                header << "Real ";
                break;
            case Version:
                header << "std::vector<Int> ";
                break;
        }
        header << child.name << ";" << std::endl << std::endl;
    }

    // Print is_complete function.
    if (node.derived.empty()) {
        auto doc = "Returns whether this `" + node.title_case_name + "` is complete/fully defined.";
        format_doc(header, doc, "    ");
        header << "    bool is_complete() const override;" << std::endl << std::endl;
        format_doc(source, doc);
        source << "bool " << node.title_case_name;
        source << "::is_complete() const {" << std::endl;
        if (node.is_error_marker) {
            source << "    return false;" << std::endl;
        } else {
            for (auto &child : node.all_children()) {
                switch (child.type) {
                    case Maybe:
                    case One:
                    case Any:
                    case Many:
                        source << "    if (!" << child.name << ".is_complete()) return false;" << std::endl;
                        break;
                    case Version:
                        source << "    if (" << child.name << ".empty()) return false;" << std::endl;
                        break;
                    default:
                        break;
                }
            }
            source << "    return true;" << std::endl;
        }
        source << "}" << std::endl << std::endl;
    }

    // Print type() function.
    if (node.derived.empty()) {
        auto doc = "Returns the `NodeType` of this node.";
        format_doc(header, doc, "    ");
        header << "    NodeType type() const override;" << std::endl << std::endl;
        format_doc(source, doc);
        source << "NodeType " << node.title_case_name;
        source << "::type() const {" << std::endl;
        source << "    return NodeType::" << node.title_case_name << ";" << std::endl;
        source << "}" << std::endl << std::endl;
    }

    // Print equality operator.
    if (node.derived.empty()) {
        auto doc = "Equality operator. Ignores annotations!";
        format_doc(header, doc, "    ");
        header << "    bool operator==(const Node& rhs) const override;" << std::endl << std::endl;
        format_doc(source, doc);
        source << "bool " << node.title_case_name;
        source << "::operator==(const Node& rhs) const {" << std::endl;
        source << "    if (rhs.type() != NodeType::" << node.title_case_name << ") return false;" << std::endl;
        auto children = node.all_children();
        if (!children.empty()) {
            source << "    auto rhsc = static_cast<const " << node.title_case_name << "&>(rhs);" << std::endl;
            for (auto &child : children) {
                source << "    if (this->" << child.name << " != rhsc." << child.name << ") return false;" << std::endl;
            }
        }
        source << "    return true;" << std::endl;
        source << "}" << std::endl << std::endl;
    }

    // Print visitor function.
    if (node.derived.empty()) {
        auto doc = "Visit a `" + node.title_case_name + "` node.";
        format_doc(header, doc, "    ");
        header << "    void visit(Visitor &visitor) override;" << std::endl << std::endl;
        format_doc(source, doc);
        source << "void " << node.title_case_name;
        source << "::visit(Visitor &visitor) {" << std::endl;
        source << "    visitor.visit_" << node.snake_case_name;
        source << "(*this);" << std::endl;
        source << "}" << std::endl << std::endl;
    }

    // Print class footer.
    header << "};" << std::endl << std::endl;
}

// Generate the visitor base class.
static void generate_visitor_base_class(
    std::ofstream &header,
    std::ofstream &source,
    Nodes &nodes
) {

    // Print class header.
    format_doc(
        header,
        "Base class for the visitor pattern for the AST.\n\n"
        "To operate on the AST, derive from this class, describe your "
        "operation by overriding the appropriate visit functions. and then "
        "call `node->visit(your_visitor)`. The default implementations for "
        "the node-specific functions fall back to the more generic functions, "
        "eventually leading to `visit_node()`, which must be implemented with "
        "the desired behavior for unknown nodes.");
    header << "class Visitor {" << std::endl;
    header << "public:" << std::endl << std::endl;

    // Virtual destructor.
    format_doc(header, "Virtual destructor for proper cleanup.", "    ");
    header << "    virtual ~Visitor() {};" << std::endl << std::endl;

    // Fallback for any kind of node.
    format_doc(header, "Fallback function for nodes of any type.", "    ");
    header << "    virtual void visit_node(Node &node) = 0;" << std::endl << std::endl;

    // Functions for all node types.
    for (auto &node : nodes) {
        std::string doc;
        if (node->derived.empty()) {
            doc = "Visitor function for `" + node->title_case_name + "` nodes.";
        } else {
            doc = "Fallback function for `" + node->title_case_name + "` nodes.";
        }
        format_doc(header, doc, "    ");
        header << "    virtual void visit_" << node->snake_case_name;
        header << "(" << node->title_case_name << " &node);" << std::endl << std::endl;
        format_doc(source, doc);
        source << "void Visitor::visit_" << node->snake_case_name;
        source << "(" << node->title_case_name << " &node) {" << std::endl;
        if (node->parent) {
            source << "    visit_" << node->parent->snake_case_name << "(node);" << std::endl;
        } else {
            source << "    visit_node(node);" << std::endl;
        }
        source << "}" << std::endl << std::endl;
    }

    header << "};" << std::endl << std::endl;
}

// Generate the recursive visitor class.
static void generate_recursive_visitor_class(
    std::ofstream &header,
    std::ofstream &source,
    Nodes &nodes
) {

    // Print class header.
    format_doc(
        header,
        "Visitor base class defaulting to DFS traversal.\n\n"
        "The visitor functions for nodes with children default to DFS "
        "traversal instead of falling back to more generic node types.");
    header << "class RecursiveVisitor : public Visitor {" << std::endl;
    header << "public:" << std::endl << std::endl;

    // Functions for all node types.
    for (auto &node : nodes) {
        auto all_children = node->all_children();
        bool empty = true;
        for (auto &child : all_children) {
            if (child.node_type) {
                empty = false;
                break;
            }
        }
        if (empty) {
            continue;
        }
        auto doc = "Recursive traversal for `" + node->title_case_name + "` nodes.";
        format_doc(header, doc, "    ");
        header << "    void visit_" << node->snake_case_name;
        header << "(" << node->title_case_name << " &node) override;" << std::endl << std::endl;
        format_doc(source, doc);
        source << "void RecursiveVisitor::visit_" << node->snake_case_name;
        source << "(" << node->title_case_name << " &node) {" << std::endl;
        for (auto &child : all_children) {
            if (child.node_type) {
                source << "    node." << child.name << ".visit(*this);" << std::endl;
            }
        }
        source << "}" << std::endl << std::endl;
    }

    header << "};" << std::endl << std::endl;
}

// Generate the dumper class.
static void generate_dumper_class(
    std::ofstream &header,
    std::ofstream &source,
    Nodes &nodes
) {

    // Print class header.
    format_doc(header, "Visitor class that debug-dumps an AST to a stream");
    header << "class Dumper : public RecursiveVisitor {" << std::endl;
    header << "protected:" << std::endl << std::endl;
    format_doc(header, "Output stream to dump to.", "    ");
    header << "    std::ostream &out;" << std::endl << std::endl;
    format_doc(header, "Current indentation level.", "    ");
    header << "    int indent = 0;" << std::endl << std::endl;

    // Print function that prints indentation level.
    format_doc(header, "Writes the current indentation level's worth of spaces.", "    ");
    header << "    void write_indent();" << std::endl << std::endl;
    format_doc(source, "Writes the current indentation level's worth of spaces.");
    source << "void Dumper::write_indent() {" << std::endl;
    source << "    for (int i = 0; i < indent; i++) {" << std::endl;
    source << "        out << \"  \";" << std::endl;
    source << "    }" << std::endl;
    source << "}" << std::endl << std::endl;

    // Write constructor.
    header << "public:" << std::endl << std::endl;
    format_doc(header, "Construct a dumping visitor.", "    ");
    header << "    Dumper(std::ostream &out) : out(out) {};" << std::endl << std::endl;

    // Print fallback function.
    format_doc(header, "Dumps a `Node`.", "    ");
    header << "    void visit_node(Node &node) override;" << std::endl;
    format_doc(source, "Dumps a `Node`.");
    source << "void Dumper::visit_node(Node &node) {" << std::endl;
    source << "    write_indent();" << std::endl;
    source << "    out << \"!Node()\" << std::endl;" << std::endl;
    source << "}" << std::endl << std::endl;

    // Functions for all node types.
    for (auto &node : nodes) {
        auto doc = "Dumps a `" + node->title_case_name + "` node.";
        format_doc(header, doc, "    ");
        header << "    void visit_" << node->snake_case_name;
        header << "(" << node->title_case_name << " &node) override;" << std::endl << std::endl;
        format_doc(source, doc);
        source << "void Dumper::visit_" << node->snake_case_name;
        source << "(" << node->title_case_name << " &node) {" << std::endl;
        source << "    write_indent();" << std::endl;
        auto children = node->all_children();
        source << "    if (!node.is_complete()) {" << std::endl;
        source << "        out << \"!\";" << std::endl;
        source << "    }" << std::endl;
        source << "    out << \"" << node->title_case_name << "(\";" << std::endl;
        source << "    if (node.has_annotation<SourceLocation>()) {" << std::endl;
        source << "        out << \" # \" << *node.get_annotation<SourceLocation>();" << std::endl;
        source << "    }" << std::endl;
        source << "    out << std::endl;" << std::endl;
        if (!children.empty()) {
            source << "    indent++;" << std::endl;
            for (auto &child : children) {
                source << "    write_indent();" << std::endl;
                source << "    out << \"" << child.name << ": \";" << std::endl;
                switch (child.type) {
                    case Maybe:
                    case One:
                        source << "    if (node." << child.name << ".empty()) {" << std::endl;
                        if (child.type == One) {
                            source << "        out << \"!MISSING\" << std::endl;" << std::endl;
                        } else {
                            source << "        out << \"-\" << std::endl;" << std::endl;
                        }
                        source << "    } else {" << std::endl;
                        source << "        out << \"<\" << std::endl;" << std::endl;
                        source << "        indent++;" << std::endl;
                        source << "        node." << child.name << ".visit(*this);" << std::endl;
                        source << "        indent--;" << std::endl;
                        source << "        write_indent();" << std::endl;
                        source << "        out << \">\" << std::endl;" << std::endl;
                        source << "    }" << std::endl;
                        break;

                    case Any:
                    case Many:
                        source << "    if (node." << child.name << ".empty()) {" << std::endl;
                        if (child.type == One) {
                            source << "        out << \"!MISSING\" << std::endl;" << std::endl;
                        } else {
                            source << "        out << \"[]\" << std::endl;" << std::endl;
                        }
                        source << "    } else {" << std::endl;
                        source << "        out << \"[\" << std::endl;" << std::endl;
                        source << "        indent++;" << std::endl;
                        source << "        for (auto &sptr : node." << child.name << ") {" << std::endl;
                        source << "            if (sptr) {" << std::endl;
                        source << "                sptr->visit(*this);" << std::endl;
                        source << "            } else {" << std::endl;
                        source << "                write_indent();" << std::endl;
                        source << "                out << \"!NULL\" << std::endl;" << std::endl;
                        source << "            }" << std::endl;
                        source << "        }" << std::endl;
                        source << "        indent--;" << std::endl;
                        source << "        write_indent();" << std::endl;
                        source << "        out << \"]\" << std::endl;" << std::endl;
                        source << "    }" << std::endl;
                        break;

                    case Str:
                    case Int:
                    case Real:
                        source << "    out << node." << child.name << " << std::endl;" << std::endl;
                        break;

                    case Version:
                        source << "    for (size_t i = 0; i < node." << child.name << ".size(); i++) {" << std::endl;
                        source << "        if (i) {" << std::endl;
                        source << "            out << \".\";" << std::endl;
                        source << "        }" << std::endl;
                        source << "        out << node." << child.name << "[i];" << std::endl;
                        source << "    }" << std::endl;
                        source << "    out << std::endl;" << std::endl;
                        break;

                }
            }
            source << "    indent--;" << std::endl;
            source << "    write_indent();" << std::endl;
        }
        source << "    out << \")\" << std::endl;" << std::endl;
        source << "}" << std::endl << std::endl;
    }

    header << "};" << std::endl << std::endl;
}

/**
 * Main function for generating the the header and source file for a tree.
 */
int generate(
    int argc,
    char *argv[],
    const std::string &name,
    Nodes &nodes
) {

    // Check command line and open files.
    if (argc != 3) {
        std::cerr << "Usage: ast-gen <header_dir> <source_dir>" << std::endl;
        return 1;
    }
    auto header = std::ofstream(std::string(argv[1]) + "/cqasm-" + name + "-gen.hpp");
    if (!header.is_open()) {
        std::cerr << "Failed to open header file for writing" << std::endl;
        return 1;
    }
    auto source = std::ofstream(std::string(argv[2]) + "/cqasm-" + name + "-gen.cpp");
    if (!source.is_open()) {
        std::cerr << "Failed to open source file for writing" << std::endl;
        return 1;
    }

    // Uppercase the name.
    std::string upper_name = name;
    std::transform(
        upper_name.begin(), upper_name.end(), upper_name.begin(),
        [](unsigned char c){
            return std::toupper(c);
        }
    );

    // Header for the header file.
    header << "#ifndef _CQASM_" << upper_name << "_GEN_HPP_INCLUDED_" << std::endl;
    header << "#define _CQASM_" << upper_name << "_GEN_HPP_INCLUDED_" << std::endl;
    header << std::endl;
    header << "#include \"cqasm-" << name << "-head.hpp\"" << std::endl;
    header << std::endl;
    header << "namespace cqasm {" << std::endl;
    header << "namespace " << name << " {" << std::endl;
    header << std::endl;

    // Header for the source file.
    source << "#include \"cqasm-" << name << "-gen.hpp\"" << std::endl;
    source << "#include \"cqasm-analyzer.hpp\"" << std::endl;
    source << std::endl;
    source << "using namespace cqasm;" << std::endl;
    source << std::endl;
    source << "namespace cqasm {" << std::endl;
    source << "namespace " << name << " {" << std::endl;
    source << std::endl;

    // Generate the NodeType enum.
    std::vector<std::string> variants;
    for (auto &node : nodes) {
        if (node->derived.empty()) {
            variants.push_back(node->title_case_name);
        }
    }
    format_doc(header, "Enumeration of all ast node types.");
    header << "enum class NodeType {" << std::endl;
    for (size_t i = 0; i < variants.size(); i++) {
        header << "    " << variants[i];
        if (i < variants.size() - 1) {
            header << ",";
        }
        header << std::endl;
    }
    header << "};" << std::endl << std::endl;

    // Generate forward references for all the classes.
    header << "// Forward declarations for " << name << " nodes." << std::endl;
    for (auto &node : nodes) {
        header << "class " << node->title_case_name << ";" << std::endl;
    }
    header << std::endl;

    // Generate the node classes.
    for (auto &node : nodes) {
        generate_node_class(header, source, *node);
    }

    // Generate the visitor classes.
    generate_visitor_base_class(header, source, nodes);
    generate_recursive_visitor_class(header, source, nodes);
    generate_dumper_class(header, source, nodes);

    // Footer for the header file.
    header << "} // namespace " << name << std::endl;
    header << "} // namespace cqasm" << std::endl;
    header << std::endl;
    header << "#endif" << std::endl;

    // Footer for the source file.
    source << "} // namespace " << name << std::endl;
    source << "} // namespace cqasm" << std::endl;

    return 0;
}
