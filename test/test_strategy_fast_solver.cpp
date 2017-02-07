#include <gtest/gtest.h>

#include "ss_strategy.h"
#include "ss_move_db.h"
#include "ss_util.h"
#include "ss_dict.h"

using namespace ss;

struct strategy_test : public testing::Test{
protected:
        virtual void SetUp()override {
		std::string board_str = 
		//       012345678901234
		   /*0*/"               "
		   /*1*/"               "
		   /*2*/"               "
		   /*3*/"               "
		   /*4*/"               "
		   /*5*/"       A       "
		   /*6*/"       A       "
		   /*7*/"    AAAAAAAAA  "
		   /*8*/"       A       "
		   /*9*/"    AAAA       "
		  /*10*/"       A       "
		  /*11*/"       A       "
		  /*12*/"               "
		  /*13*/"               "
		  /*14*/"               "
		;
		brd = board{15,15,' '};
		read_board_from_string(brd, board_str);
		std::string rck_str = "AAAAAAA";
		rck = rack{rck_str};
		strat = strategy_factory::get_inst()->make("fast_solver");
	}
        board brd;
        rack rck;
	std::shared_ptr<strategy> strat;
};

TEST_F(strategy_test, false_dict){
        move_db db;
	false_dictionary dict;
        strat->yeild(brd, rck, dict, db.accepter());
        EXPECT_EQ(0, db.size());
}

TEST_F(strategy_test, true_dict){
        move_db db;
	true_dictionary dict;
        strat->yeild(brd, rck, dict, db.accepter());
        
        EXPECT_NE(0, db.size());

        // this is a valid move
        EXPECT_TRUE(  db.lookup(array_orientation::vertical, 4,7).count("AAAAAA") );

        // this isn't a move the tiles already exist (and no one letter words)
        EXPECT_FALSE( db.lookup(array_orientation::vertical, 7,7).count("A") );
        EXPECT_FALSE( db.lookup(array_orientation::vertical, 7,7).count("AAA") );

        // this is already on the board
        EXPECT_FALSE(  db.lookup(array_orientation::horizontal, 4,7).count("AAAAAAAAA") );
        // this isn't
        EXPECT_FALSE(  db.lookup(array_orientation::horizontal, 4,7).count("AAAAAAAAAA") );
        EXPECT_FALSE(  db.lookup(array_orientation::horizontal, 3,7).count("AAAAAAAAAA") );
        EXPECT_FALSE(  db.lookup(array_orientation::horizontal, 3,7).count("AAAAAAAAAAA") );

}

