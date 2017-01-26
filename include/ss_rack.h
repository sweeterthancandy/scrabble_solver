#pragma once

#include "ss_tile.h"

#include <string>
#include <vector>
#include <set>

namespace ss{


        /*
         * This is basically just a std::string,
         * but this helps keeping the algorithms less
         * <LeftMouse>
         *
         */
        struct rack{
		using value_type = tile_t;
		using iterator = decltype( std::vector<tile_t>().begin() );
		using const_iterator = decltype( std::vector<tile_t>().cbegin() );

                rack()=default;
                explicit rack(std::vector<tile_t>& tiles);
                explicit rack(std::string const& s);
                rack clone_remove_tile(tile_t t)const;
                std::set<tile_t> make_tile_set()const;
                size_t size()const;
		decltype(auto) begin()const{return tiles_.begin(); }
		decltype(auto) end()const{return tiles_.end(); }
		decltype(auto) begin(){return tiles_.begin(); }
		decltype(auto) end(){return tiles_.end(); }
		decltype(auto) cbegin()const{return tiles_.begin(); }
		decltype(auto) cend()const{return tiles_.end(); }
        private:
                std::vector<tile_t> tiles_;
        };
}
