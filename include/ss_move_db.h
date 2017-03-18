#pragma once

#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <set>

#include "ss_board.h"
#include "ss_word_placement.h"

namespace ss{

        /*
         * This is so that I can run unit tests, but buffering every word 
         * shouldn't really matter anyway
         */
        struct move_db{
                void push(array_orientation orientation, size_t x, size_t y, std::string word){
                        map_[std::make_tuple(orientation,x,y)].insert(std::move(word));
                }
                decltype(auto) lookup(array_orientation orientation, size_t x, size_t y){
                        return map_[std::make_tuple(orientation,x,y)];
                }
                auto accepter(){
                        return [this](std::vector<word_placement> const& placements){
                                for( auto const& p : placements ){
                                        this->push(p.get_orientation(), p.get_x(), p.get_y(), p.get_word());
                                }
                        };
                }
                void dump()const{
                        using std::get;
                        boost::for_each( map_, [](auto&& p){
                                std::cout << "{" << get<0>(p.first) 
                                                 << "," << get<1>(p.first)
                                                 << "," << get<2>(p.first)
                                          << "}\n";
                                boost::for_each( p.second, [](auto&& word){
                                        std::cout << "    " << word << "\n";
                                });
                        });
                }
                size_t size()const{
                        size_t result{0};
                        for( auto const& p : map_ ){
                                result += p.second.size();
                        }
                        return result;
                }
        private:
                std::map<std::tuple<array_orientation, size_t,size_t>, std::set<std::string> > map_;
        };
}
