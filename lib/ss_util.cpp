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
        
        void read_board_from_string(board& b, std::string const& str){
                if( b.x_len() * b.y_len() != str.size() ){
                        std::stringstream msg;
                        msg << "parsing string as board but wrong size x_len * y_len = " 
                                << b.x_len() * b.y_len() << ", str.size = " << str.size();
                        BOOST_THROW_EXCEPTION(std::domain_error(msg.str()));
                }
                b.fill('\0');
                auto iter = str.begin();
                //for( size_t y = b.y_len();y!=0;){
                //      --y;
                for(size_t y=0;y!=b.y_len();++y){
                        for(size_t x=0;x!=b.x_len();++x){
                                if( *iter == ' '){
                                } else if( std::isalpha( *iter ) ){
                                        b(x,y) = std::toupper(*iter);
                                } else{
                                        std::stringstream msg;
                                        msg << "unknown char " << (int)(*iter);
                                        BOOST_THROW_EXCEPTION(std::domain_error(msg.str()));
                                }
                                ++iter;
                        }
                }
        }
}
