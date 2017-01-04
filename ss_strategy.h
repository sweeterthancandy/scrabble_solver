#pragma once

#include "ss_board.h"
#include "ss_rack.h"

#include <memory>

namespace ss{
        struct strategy{
                virtual ~strategy()=default;

                virtual player_move solve(board const& board, rack const& r)=0;
                virtual std::shared_ptr<strategy> clone()=0;
        };

        using strategy_factory = generic_factory<strategy>;
}
