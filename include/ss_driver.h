#pragma once

#include <vector>
#include <boost/variant.hpp>
#include <tuple>

#include "ss.h"

namespace ss{

                
        struct skip_go{};
        
        struct place_tiles{
                std::vector<std::tuple<size_t, size_t, tile_t, tile_t> > vec;
        };
        
        using player_move = boost::variant<skip_go, place_tiles>;

        namespace detail{
                struct player_move_to_string : boost::static_visitor<std::string>{
                        std::string operator()(skip_go const&)const{
                                return "skip_go";
                        }
                        std::string operator()(place_tiles const&)const{
                                return "place_tiles";
                        }
                };
        }
        inline
        std::string player_move_to_string( player_move const& move ){
                return boost::apply_visitor( detail::player_move_to_string(), move);

        }



}
