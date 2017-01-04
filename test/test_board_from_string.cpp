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
        EXPECT_EQ( ' ', b(0,0));
        EXPECT_EQ( ' ', b(3,2));
        EXPECT_EQ( ' ', b(4,1));

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
                               "    e"
                               "   d "
                               "  c  "
                               " b   "
                               "a    ");

        EXPECT_EQ( 'a', b(0,0));
        EXPECT_EQ( 'b', b(1,1));
        EXPECT_EQ( 'c', b(2,2));
        EXPECT_EQ( 'd', b(3,3));
        EXPECT_EQ( 'e', b(4,4));

}
