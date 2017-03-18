#pragma once

#include "ss_board.h"
#include "ss_rack.h"
#include "ss_dict.h"

#include <memory>
#include <functional>

namespace ss{

        struct word_placement;

        struct strategy{
                // first word is the primary move
                using callback_t = 
                        std::function<
                                void(std::vector<word_placement> const&)
                        >;

                virtual ~strategy()=default;

                virtual void yeild(board const& board, rack const& r, dictionary_t const& dict, callback_t callback)=0;
                virtual std::shared_ptr<strategy> clone()=0;
        };

        using strategy_factory = generic_factory<strategy>;
}
