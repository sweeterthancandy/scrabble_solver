#pragma once

#include "ss_tile.h"

#include <string>
#include <vector>
#include <set>
#include <iostream>

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
                std::string to_string()const;
                rack clone_remove_tile(tile_t t)const;
                rack & remove(tile_t t){
                        *this = this->clone_remove_tile(t);
                        return *this;
                }
                std::set<tile_t> make_tile_set()const;
                size_t size()const;
		decltype(auto) begin()const{return tiles_.begin(); }
		decltype(auto) end()const{return tiles_.end(); }
		decltype(auto) begin(){return tiles_.begin(); }
		decltype(auto) end(){return tiles_.end(); }
		decltype(auto) cbegin()const{return tiles_.begin(); }
		decltype(auto) cend()const{return tiles_.end(); }
                friend std::ostream& operator<<(std::ostream& ostr, rack const& self);
                void dump(std::ostream& ostr = std::cout)const{ ostr << *this; }
        private:
                std::vector<tile_t> tiles_;
        };
}
