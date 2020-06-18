#include <gtest/gtest.h> // googletest header file

#include <cqasm.hpp>

TEST(test, test) {
    auto r = cqasm::parser::parse_file("grover.cq");
    EXPECT_TRUE(r.root.is_complete());
    for (auto err : r.errors) {
        EXPECT_EQ(err, "");
    }

    auto a = cqasm::analyzer::Analyzer();
    auto r2 = a.analyze(*r.root->as_program());
    //EXPECT_TRUE(r2.root.is_complete());
    for (auto err : r2.errors) {
        EXPECT_EQ(err, "");
    }

    //std::cerr << *r.root << std::endl;
    std::cerr << *r2.root << std::endl;
    //EXPECT_TRUE(false);
}
