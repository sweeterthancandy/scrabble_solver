#pragma once

#include "ss_board.h"
#include "ss_rack.h"
#include "ss_dict.h"

#include <memory>
#include <functional>

namespace ss{

        struct strategy{
                // (orientation, x, y, word)
                using callback_t = std::function<void(array_orientation, size_t, size_t, std::string const&)>;
                virtual ~strategy()=default;

                virtual void yeild(board const& board, rack const& r, dictionary_t const& dict, callback_t callback)=0;
                virtual std::shared_ptr<strategy> clone()=0;
        };

        using strategy_factory = generic_factory<strategy>;
}
