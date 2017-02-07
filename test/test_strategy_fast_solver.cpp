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
		   /*5*/"               "
		   /*6*/"               "
		   /*7*/"       A       "
		   /*8*/"               "
		   /*9*/"               "
		  /*10*/"               "
		  /*11*/"               "
		  /*12*/"               "
		  /*13*/"               "
		  /*14*/"               "
		;
		brd = board{15,15,' '};
		read_board_from_string(brd, board_str);
		std::string rck_str = "B";
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

        db.dump();
        
        EXPECT_EQ(4, db.size());
        
        EXPECT_EQ( 1,  db.lookup(array_orientation::horizontal, 7,7).size() );
        EXPECT_EQ( 1,  db.lookup(array_orientation::horizontal, 7,7).size() );
        
        EXPECT_EQ( 1,  db.lookup(array_orientation::vertical, 7,6).size() );
        EXPECT_EQ( 1,  db.lookup(array_orientation::vertical, 7,7).size() );

}

