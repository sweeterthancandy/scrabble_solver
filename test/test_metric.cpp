
#include <gtest/gtest.h>

#include "ss_metric.h"

using namespace ss;

class scrabble_metric : public testing::Test {
protected:
        virtual void SetUp()override {
                metric =  ss::metric_factory::get_inst()->make("scrabble_metric");
        }
        std::shared_ptr<ss::metric> metric;
};

TEST_F( scrabble_metric, simple ){
        /*
         * These are starting from the middlem should 
         */
        EXPECT_EQ( 2 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AA", "AA"} ) );
        EXPECT_EQ( 2 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AA", " A"} ) );
        EXPECT_EQ( 2 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AA", "A "} ) );

        EXPECT_EQ( 3 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AAA", "AAA"} ) );
        EXPECT_EQ( 3 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AAA", " AA"} ) );
        EXPECT_EQ( 3 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AAA", "A A"} ) );
        EXPECT_EQ( 3 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AAA", "AA "} ) );
        EXPECT_EQ( 3 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AAA", "  A"} ) );
        EXPECT_EQ( 3 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AAA", " A "} ) );
        EXPECT_EQ( 3 , metric->calculate( word_placement{8,7, array_orientation::horizontal, "AAA", "A  "} ) );
}

TEST_F( scrabble_metric, starting_moves ){
        /*
         * These are starting from the middlem should 
         */
        EXPECT_EQ(2*4 , metric->calculate( word_placement{7,7, array_orientation::horizontal, "AAAA", "AAAA"} ) );
        EXPECT_EQ(  4 , metric->calculate( word_placement{7,7, array_orientation::horizontal, "AAAA", " AA "} ) );
}

TEST_F( scrabble_metric, multiplirs){
        EXPECT_EQ(3*2, metric->calculate( word_placement{0,0, array_orientation::horizontal, "AA", "AA"} ) );
        EXPECT_EQ(3*5, metric->calculate( word_placement{0,0, array_orientation::horizontal, "AAAA", "AAAA"} ) );
        EXPECT_EQ(3*4, metric->calculate( word_placement{0,0, array_orientation::horizontal, "AAAA", "AAA "} ) );
        EXPECT_EQ(9*9, metric->calculate( word_placement{0,0, array_orientation::horizontal, "AAAAAAAA", "AAAAAAAA" } ));
        EXPECT_EQ(9*8, metric->calculate( word_placement{0,0, array_orientation::horizontal, "AAAAAAAA", "AAA AAAA" } ));
}

