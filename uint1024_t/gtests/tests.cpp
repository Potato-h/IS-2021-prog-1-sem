#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <map>

extern "C" {
    #include "../uint1024.h"
}

TEST(uint1024_t, string_convertor) { 
    std::vector<const char*> expected = {
        "7137817238"
        "12738917293871238712",
        "9183091832098120938109381238",
        "8138917239871293871387198739817398173981273987123"
    };

    for (auto input : expected) {
        uint1024_t x = stoi(input);
        char* obtained = itos(x);
        EXPECT_EQ(std::string(input), std::string(obtained));
    }
}

TEST (uint1024_t, add) { 
    std::map<std::pair<const char*, const char*>, const char*> tests = {
        { { "7137817238", "139183091832" }, "146320909070" },
        { { "12738917293871238712", "9130918293081938" }, "12748048212164320650" },
        { { "9183091832098120938109381238", "1203102930193123" }, "9183091832099324041039574361" },
        { { "8138917239871293871387198739817398173981273987123", "913891391839183291832" }, "8138917239871293871387198740731289565820457278955" }
    };

    for (auto test : tests) {
        auto input = test.first;
        uint1024_t x = stoi(input.first);
        uint1024_t y = stoi(input.second);
        uint1024_t res = add_op(x, y);
        char* obtained = itos(res);
        EXPECT_EQ(std::string(test.second), std::string(obtained));
    }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}