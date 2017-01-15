#include "ss_util.h"

namespace ss{
        
        std::vector<std::string> words_from_board(board const& b, search_direction direction){
                std::vector<std::string> result;

                auto tpl = [&result](auto&& aux){
                        enum class state_t{
                                looking_for_word,
                                matching_word
                        };

                        for(size_t x=0;x!=aux.x_len();++x){
                                state_t state = state_t::looking_for_word;
                                std::string buffer;
                                for(size_t y=0; y!=aux.y_len();++y){

                                        switch(state){
                                        case state_t::looking_for_word:
                                                assert( buffer.empty() );
                                                if( aux(x,y) == '\0')
                                                        continue;
                                                // start of start '\<\w'
                                                buffer += aux(x,y);
                                                state = state_t::matching_word;
                                                break;
                                        case state_t::matching_word:
                                                if( aux(x,y) == '\0'){
                                                        // end of word '\w\>'
                                                        assert( buffer.size() );
                                                        if( 2 <= buffer.size() && ! boost::binary_search(result, buffer)){
                                                                result.push_back(buffer);
                                                                boost::sort(result);
                                                        }
                                                        buffer.clear();
                                                        state = state_t::looking_for_word;
                                                } else{
                                                        buffer += aux(x,y);
                                                }
                                                break;
                                        }
                                }
                                if( 2 <= buffer.size() && ! boost::binary_search(result, buffer)){
                                        result.push_back(buffer);
                                        boost::sort(result);
                                }
                                buffer.clear();
                        }
                };

                if( static_cast<long>(direction) & static_cast<long>(search_direction::vertical)){
                        tpl(b);
                }
                if( static_cast<long>(direction) & static_cast<long>(search_direction::horizontal)){
                        tpl(make_const_rotate_view(b));
                }
                        
                return std::move(result);
        }

        std::vector<std::string> do_validate_board(dictionary_t const& dict, board b){
                std::vector<std::string> bad_words;
                auto words = words_from_board(b, search_direction::vertical);
                auto other = words_from_board(b, search_direction::horizontal);
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
