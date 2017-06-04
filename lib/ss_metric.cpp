#include "ss.h"
#include "ss_metric.h"
#include "ss_word_placement.h"

#include <map>


namespace{
        enum tile_decl{
                __,

                dl,
                tl,

                dw,
                tw,

                st
                
        };

        //       1  2  3  4  5  6  7  8  9  10  11 12 13 14 15
        tile_decl decl[15][15] = {
                {tw,__,__,dl,__,__,__,tw,__,__,__,dl,__,__,tw},
                {__,dw,__,__,__,tl,__,__,__,tl,__,__,__,dw,__},
                {__,__,dw,__,__,__,dl,__,dl,__,__,__,dw,__,__},
                {dl,__,__,dw,__,__,__,dl,__,__,__,dw,__,__,dl},
                {__,__,__,__,dw,__,__,__,__,__,dw,__,__,__,__},
                {__,tl,__,__,__,tl,__,__,__,tl,__,__,__,tl,__},
                {__,__,dl,__,__,__,dl,__,dl,__,__,__,dl,__,__},
                {tw,__,__,dl,__,__,__,st,__,__,__,dl,__,__,tw},
                {__,__,dl,__,__,__,dl,__,dl,__,__,__,dl,__,__},
                {__,tl,__,__,__,tl,__,__,__,tl,__,__,__,tl,__},
                {__,__,__,__,dw,__,__,__,__,__,dw,__,__,__,__},
                {dl,__,__,dw,__,__,__,dl,__,__,__,dw,__,__,dl},
                {__,__,dw,__,__,__,dl,__,dl,__,__,__,dw,__,__},
                {__,dw,__,__,__,tl,__,__,__,tl,__,__,__,dw,__},
                {tw,__,__,dl,__,__,__,tw,__,__,__,dl,__,__,tw}};

        std::map<char, size_t> tile_value={
                {'A', 1},
                {'B', 3},
                {'C', 3},
                {'D', 2},
                {'E', 1},
                {'F', 4},
                {'G', 2},
                {'H', 4},
                {'I', 1},
                {'J', 8},
                {'K', 5},
                {'L', 1},
                {'M', 3},
                {'N', 1},
                {'O', 1},
                {'P', 3},
                {'Q',10},
                {'R', 1},
                {'S', 1},
                {'T', 1},
                {'U', 1},
                {'V', 4},
                {'W', 4},
                {'X', 8},
                {'Y', 4},
                {'Z',10},
        };
}




namespace ss{



        /* 
                Scrabble scoring takes the form of the following
                        sigma <- 0
                        for all words w
                                sigma <- sigma + metric(w)



                        metric: word -> Z
        */ 
                                
        struct scrabble_metric : metric{
                size_t calculate(std::vector<word_placement> const& placements)override{
                        size_t sigma{0};
                        for( auto const& p : placements )
                        {
                                switch( p.get_orientation()){
                                case array_orientation::horizontal:
                                        sigma += sub_calculate(p.get_x(), p.get_y(), p);
                                        break;
                                case array_orientation::vertical:
                                        sigma += sub_calculate(p.get_y(), p.get_x(), p);
                                        break;
                                }
                        }
                        size_t count{0};
                        for( char c : placements.front().get_mask() ){
                                if( c != ' ' )
                                        ++count;
                        }
                        if( count == 7 ){
                                sigma += 50;
                        }
                        return sigma;
                }
        private:
                size_t sub_calculate(size_t x, size_t y, word_placement const& p)const{
                        auto start{x};
                        size_t underlying{0};
                        size_t multiplyer{1};

                        auto const& word{p.get_word()};
                        auto const& mask{p.get_mask()};

                        // first sum all letters
                        for(auto i{0}; i!=p.get_word().size();++i){
                                auto idx{start+i};
                        
                                if( mask[i] == ' '){
                                        underlying += tile_value[word[i]];
                                } else{
                                        // first add up all the letters, only apply
                                        // dw and tw to placed tiles
                                        switch( decl[idx][y] ){
                                        case tile_decl::dl:
                                                underlying += tile_value[word[i]] * 2;
                                                break;
                                        case tile_decl::tl:
                                                underlying += tile_value[word[i]] * 3;
                                                break;
                                        case tile_decl::dw:
                                                underlying += tile_value[word[i]] * 1;
                                                multiplyer *= 2;
                                                break;
                                        case tile_decl::tw:
                                                underlying += tile_value[word[i]] * 1;
                                                multiplyer *= 3;
                                                break;
                                        case tile_decl::st:
                                                underlying += tile_value[word[i]] * 1;
                                                multiplyer *= 2;
                                                break;
                                        case tile_decl::__:
                                                underlying += tile_value[word[i]] * 1;
                                                break;
                                        }

                                }
                        }
                        // PRINT_SEQ((underlying)(multiplyer));
                        return underlying * multiplyer;
                }
        };

        int scarabble_metric_reg = ( metric_factory::get_inst() ->register_( "scrabble_metric", 
                                                                          [](){
                                                                                return std::make_unique<scrabble_metric>();
                                                                          }
                                                                          ), 0 );
}
