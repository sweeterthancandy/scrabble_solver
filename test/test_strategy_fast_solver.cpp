#include <gtest/gtest.h>

#include "ss_strategy.h"
#include "ss_move_db.h"
#include "ss_util.h"
#include "ss_dict.h"

using namespace ss;

TEST(strategy, simple){
        std::string board_str = 
        //       012345678901234
           /*0*/"               "
           /*1*/"               "
           /*2*/"               "
           /*3*/"               "
           /*4*/"               "
           /*5*/"               "
           /*6*/"               "
           /*7*/"    HELLOS     "
           /*8*/"               "
           /*9*/"               "
          /*10*/"               "
          /*11*/"               "
          /*12*/"               "
          /*13*/"               "
          /*14*/"               "
        ;
                                
        auto dict = std::make_unique<dictionary_t>();
        dict->emplace_back("WO");
        dict->emplace_back("WORLD");
        dict->emplace_back("WORLDS");
        dict->emplace_back("HELLOS");


        board board(15,15,' ');
        read_board_from_string(board, board_str);
        std::string rck_str = "WORLD";
        rack rck(rck_str);
        auto strat = strategy_factory::get_inst()->make("fast_solver");
        
        move_db db;
        strat->yeild(board, rck, *dict, db.accepter());
        db.dump();

        // sanity checks
        EXPECT_EQ(0, db.lookup(array_orientation::vertical,0,0).size() );
        EXPECT_FALSE( db.lookup(array_orientation::vertical,9,2).count( "DUMMY" ) );
        EXPECT_FALSE( db.lookup(array_orientation::vertical,2,2).count( "SOMETHING" ) );

        // check it picks up the words
        EXPECT_TRUE( db.lookup(array_orientation::vertical,9,2).count( "WORLDS" ));
        EXPECT_TRUE( db.lookup(array_orientation::vertical,8,6).count( "WO" ));
        EXPECT_TRUE( db.lookup(array_orientation::vertical,8,6).count( "WORLD" ));
}


