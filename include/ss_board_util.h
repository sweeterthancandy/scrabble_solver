#pragma once

#include <vector>

namespace ss{

        struct word_placement;
        struct board;

        // throws on error
        void apply_placement(board& brd, word_placement const&);
        void apply_placement(board& brd, std::vector<word_placement> const&);

}
