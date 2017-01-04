#include "ss_util.h"

namespace ss{

        std::vector<std::string> do_validate_board(dictionary_t const& dict, board b){
                std::vector<std::string> bad_words;
                auto words = words_from_board(b);
                auto other = words_from_board(make_const_rotate_view(b));
                boost::copy(other, std::back_inserter(words));

                for(auto& word : words){
                        bool ret = std::binary_search(dict.begin(), dict.end(), word);
                        //std::cout << word << ( ret ? " Is a word" : " Is not at word") << "\n";
                        if( ! ret ){
                                bad_words.emplace_back(word);
                        }
                }
                return std::move(bad_words);
        }
        bool validate_board(dictionary_t const& dict, board b){
                return do_validate_board(dict, b).size() == 0;
        }
}
