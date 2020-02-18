#include <gtest/gtest.h> // googletest header file

#include <cqasm.hpp>

TEST(test, test) {
  auto a = cqasm::Analyzer("grover.cq");
  for (auto err : a.errors) {
    EXPECT_EQ(err, "");
  }
}
