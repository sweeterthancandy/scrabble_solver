#pragma once

namespace ss{
        namespace tile_traits{
                bool empty(char c);
                inline bool not_empty(char c){
                        return ! ::ss::tile_traits::empty(c);
                }
        }
}
