#include <gtest/gtest.h>

#include "ss_util.h"

TEST(read_board_from_string, empty) {
	ss::board b(5,5);

        read_board_from_string(b,
                               "     "
                               "     "
                               "     "
                               "     "
                               "     ");
        EXPECT_EQ( '\0', b(0,0));
        EXPECT_EQ( '\0', b(3,2));
        EXPECT_EQ( '\0', b(4,1));

}
TEST(read_board_from_string, bad_dimensions) {
	ss::board b(5,5);

        EXPECT_ANY_THROW(read_board_from_string(b,
                               "     "
                               "         "
                               "     "
                               "     "
                               "     "));

}

TEST(read_board_from_string, read) {
	ss::board b(5,5);

        read_board_from_string(b,
                               "a    "
                               " b   "
                               "  c  "
                               "   d "
                               "    e"
                               );

        EXPECT_EQ( 'A', b(0,0));
        EXPECT_EQ( '\0', b(0,1));
        EXPECT_EQ( '\0', b(1,0));
        EXPECT_EQ( 'B', b(1,1));
        EXPECT_EQ( 'C', b(2,2));
        EXPECT_EQ( 'D', b(3,3));
        EXPECT_EQ( 'E', b(4,4));

}
