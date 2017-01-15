
#include <gtest/gtest.h>

#include "ss_board.h"
#include "ss_util.h"

#include <boost/range/algorithm.hpp>

namespace{
        namespace detail{
                template<class T, class U>
                bool equal(T&& left, U&& right){
                        boost::sort(left);
                        boost::unique(left);
                        boost::sort(right);
                        boost::unique(right);
                        return boost::equal(left,right);
                }
        } // detail
} // anon

TEST(words_from_board, empty){
	ss::board b(10,10);
        ss::read_board_from_string(b,
           //                   1234567890
                               "          "
                               "          "
                               "          "
                               "          "
                               "          "
                               "          "
                               "          "
                               "          "
                               "          "
                               "          "
                              );
        auto words = ss::words_from_board(b, ss::search_direction::horizontal);

        EXPECT_TRUE( words.empty());
}

TEST(words_from_board, simple){
	ss::board b(10,10);
        ss::read_board_from_string(b,
           //                   1234567890
                               "          "
                               "          "
                               " HELLO    "
                               "   WORLD  "
                               "          "
                               "          "
                               "          "
                               "          "
                               "          "
                               "          "
                              );
        auto hor = ss::words_from_board(b, ss::search_direction::horizontal);
        auto vert = ss::words_from_board(b, ss::search_direction::vertical);
        auto both = ss::words_from_board(b, ss::search_direction::both);

        using vec = decltype(hor);



        #if 0
        std::cout << "{\n";
        boost::for_each( both, [](auto&& _){ std::cout << _ << ",";});
        std::cout << "}\n";
        #endif


        EXPECT_TRUE( detail::equal(hor, vec{"HELLO", "WORLD"}));
        EXPECT_TRUE( detail::equal(vert, vec{"LW", "LO", "OR"}));
        EXPECT_TRUE( detail::equal(both, vec{"HELLO", "WORLD", "LW", "LO", "OR"}));
}

TEST(words_from_board, test)
{
	ss::board b(10,10);
        ss::read_board_from_string(b,
           //                   1234567890
                               "the       "
                               "  quick   "
                               "brown     "
                               "   fox    "
                               "jumped    "
                               "     brown"
                               "over the  "
                               " red fence"
                               "          "
                               "a fox    a"
                              );
        auto words = ss::words_from_board(b, ss::search_direction::horizontal);

        std::vector<std::string> proto = {
                "THE", "QUICK", "BROWN", "FOX",
                      "JUMPED", "OVER",
                      "RED", "FENCE" };
        EXPECT_TRUE( detail::equal( words, proto));
        #if 0
        std::cout << "{\n";
        boost::for_each( words, [](auto&& _){ std::cout << _ << ",";});
        std::cout << "}\n";
        std::cout << "{\n";
        boost::for_each( proto, [](auto&& _){ std::cout << _ << ",";});
        std::cout << "}\n";
        #endif
}
