#include <gtest/gtest.h> // googletest header file

#include <cqasm.hpp>

TEST(test, test) {
    auto a = cqasm::Analyzer();
    auto r = a.analyze("grover.cq");
    for (auto err : r.errors) {
        EXPECT_EQ(err, "");
    }
}
