#pragma once

#include "ss_board.h"
#include "ss_dict.h"

namespace ss{


        enum class search_direction{
                horizontal = 1,
                vertical   = 2,

                both   = horizontal | vertical,
        };

        std::vector<std::string> words_from_board(board const& b,
                              search_direction direction = search_direction::both);

        std::vector<std::string> do_validate_board(dictionary_t const& dict, board b);
        bool validate_board(dictionary_t const& dict, board b);
        
        void read_board_from_string(board& b, std::string const& str);

        std::vector<std::tuple<size_t, size_t> > find_initial_moves(board const& b);

}




