#include "ss_word_placement.h"
#include "ss_board.h"

namespace ss{
        word_placement make_word_placement(board const& b, size_t x, size_t y, array_orientation orientation, std::string word){
                std::string mask;
                if( orientation == ss::array_orientation::horizontal ){
                        for(size_t i=0;i!=word.size();++i){
                                mask += ( tile_traits::empty(b(x+i,y)) ? word[i] : ' ');
                        }
                } else{
                        for(size_t i=0;i!=word.size();++i){
                                mask += ( tile_traits::empty(b(x,y+i)) ? word[i] : ' ');
                        }
                }
                return word_placement(x,y,orientation, std::move(word), std::move(mask));
        }
}
