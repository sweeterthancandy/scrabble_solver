#include <gtest/gtest.h>

#include "ss_board.h"

TEST(board, len) {
	ss::board b(10,20);
	EXPECT_EQ( 10, b.x_len() );
	EXPECT_EQ( 20, b.y_len() );
}

TEST(board, access) {
	ss::board b(10,10);

	b(0,0) = 'a';
	b(1,1) = 'b';
	b(2,1) = 'c';
	b(9,9) = 'd';

	EXPECT_EQ( 'a', b(0,0));
	EXPECT_EQ( 'b', b(1,1));
	EXPECT_EQ( 'c', b(2,1));
	EXPECT_EQ( 'd', b(9,9));
}
