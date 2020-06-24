#include <gtest/gtest.h> // googletest header file

#include "qasm_semantic.hpp"

TEST(compat, grover) {

    // open a file handle to a particular file:
    FILE *myfile = fopen("grover.cq", "r");

    compiler::QasmSemanticChecker sm(myfile);

    auto qasm_representation = sm.getQasmRepresentation();

    int result = sm.parseResult();

    EXPECT_TRUE(result == 0);   // Stop here if it fails.
}
