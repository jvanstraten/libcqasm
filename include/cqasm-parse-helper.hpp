#pragma once

#include "cqasm-ast.hpp"
#include "cqasm-semantic.hpp"
#include <cstdio>

namespace cqasm {
namespace parser {

/**
 * Internal helper class for parsing cQASM files.
 */
class ParseHelper {
public:

    /**
     * File pointer being scanned.
     */
    FILE *fptr = nullptr;

    /**
     * Flex reentrant scanner data.
     */
    void *scanner = nullptr;

    /**
     * Name of the file being parsed.
     */
    std::string filename;

    /**
     * List of any errors encountered.
     */
    std::vector<std::string> errors;

    /**
     * The root node, if parsing was sufficiently successful.
     */
    tree::One<ast::Root> root;

    /**
     * Construct the parse helper for the given filename, and analyze the file
     * with flex/bison.
     */
    ParseHelper(const std::string &filename);

    /**
     * Destroys the parse helper.
     */
    virtual ~ParseHelper();

    /**
     * Pushes an error.
     */
    void push_error(const std::string &error);

};

/**
 * Source location annotation object, containing source file line numbers etc.
 */
class SourceLocation {
public:

    /**
     * The name of the source file.
     */
    std::string filename;

    /**
     * The first line of the range, or 0 if unknown.
     */
    uint32_t first_line;

    /**
     * The first column of the range, or 0 if unknown.
     */
    uint32_t first_column;

    /**
     * The last line of the range, or 0 if unknown.
     */
    uint32_t last_line;

    /**
     * The last column of the range, or 0 if unknown.
     */
    uint32_t last_column;

    /**
     * Constructs a source location object.
     */
    SourceLocation(
        const std::string &filename,
        uint32_t first_line = 0,
        uint32_t first_column = 0,
        uint32_t last_line = 0,
        uint32_t last_column = 0
    );

    /**
     * Expands the location range to contain the given location in the source
     * file.
     */
    void expand_to_include(uint32_t line, uint32_t column = 1);

};

} // namespace parser
} // namespace cqasm

/**
 * Stream << overload for source location objects.
 */
std::ostream& operator<<(std::ostream& os, const cqasm::parser::SourceLocation& object);
