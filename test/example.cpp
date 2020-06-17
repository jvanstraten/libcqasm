#include <gtest/gtest.h> // googletest header file

#include <cqasm.hpp>

TEST(test, test) {
    auto a = cqasm::Analyzer();
    auto r = a.analyze("grover.cq");
    EXPECT_TRUE(r.ast_root.is_complete());
    for (auto err : r.errors) {
        EXPECT_EQ(err, "");
    }
    std::cerr << *r.ast_root << std::endl;
    //EXPECT_TRUE(false);
}
