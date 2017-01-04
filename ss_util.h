#pragma once

#include "ss_board.h"
#include "ss_dict.h"

namespace ss{
        template<class Board>
        auto words_from_board(Board const& b);

        std::vector<std::string> do_validate_board(dictionary_t const& dict, board b);
        bool validate_board(dictionary_t const& dict, board b);
}




namespace ss{
        template<class Board>
        auto words_from_board(Board const& b){
                std::vector<std::string> result;
                enum class state_t{
                        looking_for_word,
                        matching_word
                };

                state_t state = state_t::looking_for_word;

                for(size_t x=0;x!=b.x_len();++x){
                        for(size_t y=0; y!=b.y_len();++y){

                                switch(state){
                                case state_t::looking_for_word:
                                        if( b(x,y) == '\0')
                                                continue;
                                        result.emplace_back();
                                        result.back() += b(x,y);
                                        state = state_t::matching_word;
                                        break;
                                case state_t::matching_word:
                                        if( b(x,y) == '\0'){
                                                if( result.back().size() < 2 )
                                                        result.pop_back();
                                                state = state_t::looking_for_word;
                                                continue;
                                        }
                                        result.back() += b(x,y);
                                        break;
                                }
                        }
                }
                return std::move(result);
        }
}
