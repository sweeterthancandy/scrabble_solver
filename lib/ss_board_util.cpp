#include "ss_board_util.h"
#include "ss_board.h"
#include "ss_word_placement.h"
#include "ss_tile_traits.h"

#include <boost/exception/all.hpp>

namespace ss{
        void apply_placement(board& brd, word_placement const& p){
                apply_placement(brd, std::vector<word_placement>{p});
        }
        void apply_placement(board& brd, std::vector<word_placement> const& placements){
                #if 0
                for( auto const& p : placements){
                        auto const& word{p.get_word()};
                        auto const& mask{p.get_mask()};
                        switch(p.get_orientation()){
                        case array_orientation::horizontal:
                                for(size_t i=0;i!=word.size();++i){
                                        if( tile_traits::empty(mask[i]))
                                                continue;
                                        if( ! tile_traits::empty( brd(p.get_x() + i,p.get_y()) ) ){
                                                BOOST_THROW_EXCEPTION( std::domain_error("unable to apply"));
                                        }
                                }
                                break;
                        case array_orientation::vertical:
                                for(size_t i=0;i!=word.size();++i){
                                        if( tile_traits::empty(mask[i]))
                                                continue;
                                        if( ! tile_traits::empty( brd(p.get_x(),i+p.get_y()) ) ){
                                                BOOST_THROW_EXCEPTION( std::domain_error("unable to apply"));
                                        }
                                }
                                break;
                        }
                }
                #endif
                for( auto const& p : placements){
                        auto const& word{p.get_word()};
                        auto const& mask{p.get_mask()};
                        switch(p.get_orientation()){
                        case array_orientation::horizontal:
                                for(size_t i=0;i!=word.size();++i){
                                        if( tile_traits::empty(mask[i]))
                                                continue;
                                        brd(p.get_x()+i,p.get_y()) = word[i];
                                }
                                break;
                        case array_orientation::vertical:
                                for(size_t i=0;i!=word.size();++i){
                                        if( tile_traits::empty(mask[i]))
                                                continue;
                                        brd(p.get_x(),i+p.get_y()) = word[i];
                                }
                                break;
                        }
                }
        }
}
