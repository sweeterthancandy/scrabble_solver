
#include <gtest/gtest.h>

#include "ss_board.h"

#ifdef NOT_DEFINED

TEST(strategy, test){
        std::string board_str = 
        //       123456789012345
           /*1*/"               "
           /*2*/"               "
           /*3*/"               "
           /*4*/"               "
           /*5*/"               "
           /*6*/"        COMP   "
           /*7*/"        L      "
           /*8*/"       PATHS   "
           /*9*/"        M  E   "
          /*10*/"         BELIE "
          /*11*/"           L   "
          /*12*/"           E   "
          /*13*/"           R   "
          /*14*/"               "
          /*15*/"               "
        ;
        std::string rck_str = "SHACKLE";

        auto word_set(str, rck_str);

        EXPECT_TRUE(generate_set.contains("COMPS"));
        EXPECT_FALSE(generate_set.contains("SCLAM"));
}


#endif
