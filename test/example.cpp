#include <gtest/gtest.h> // googletest header file

#include <cqasm.hpp>

TEST(test, test) {
    //auto a = cqasm::Analyzer();
    auto r = cqasm::parser::parse_file("grover.cq");
    EXPECT_TRUE(r.root.is_complete());
    for (auto err : r.errors) {
        EXPECT_EQ(err, "");
    }
    std::cerr << *r.root << std::endl;
    //EXPECT_TRUE(false);
}
