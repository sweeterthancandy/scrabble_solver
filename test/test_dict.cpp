#include <gtest/gtest.h>

#include "ss_dict.h"
#include "ss_dict_helpers.h"

class dictionary : public testing::Test {
protected:
        virtual void SetUp()override {
                std::vector<std::string> proto;
                proto.emplace_back("hello");
                proto.emplace_back("world");

                dict = ss::std_dictionary{std::move(proto)};
        }
        ss::std_dictionary dict;
};

TEST_F(dictionary, contains){
        EXPECT_TRUE( dict.contains("hello") );
        EXPECT_TRUE( dict.contains("world") );

        EXPECT_FALSE( dict.contains("hell") );
        EXPECT_FALSE( dict.contains("helloo") );


        // this is really a policy
        EXPECT_FALSE( dict.contains("") );
}

TEST_F(dictionary, prefix){
        EXPECT_TRUE( dict.contains_prefix("hello") );
        EXPECT_TRUE( dict.contains_prefix("hell") );
        EXPECT_TRUE( dict.contains_prefix("hel") );
        EXPECT_TRUE( dict.contains_prefix("he") );
        EXPECT_TRUE( dict.contains_prefix("h") );
        
        // this similfied algorithms
        EXPECT_TRUE( dict.contains_prefix("") );

        EXPECT_FALSE( dict.contains_prefix("qh") );
}
