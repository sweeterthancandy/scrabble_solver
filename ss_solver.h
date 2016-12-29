#pragma once
#include "ss.h"
#include "ss_board.h"
#include "ss_driver.h"

#include <fstream>
#include <functional>

namespace ss{

        template<class Board>
        inline auto words_from_board(Board const& b){
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

        inline bool validate_move(std::vector<std::string> const& dict, board b, board const& moves){
                std::vector<std::string> bad_words;
                b.paste(moves, '\0');
                auto words = words_from_board(b);
                auto other = words_from_board(make_const_rotate_view(b));
                boost::copy(other, std::back_inserter(words));

                for(auto& word : words){
                        bool ret = std::binary_search(dict.begin(), dict.end(), word);
                        std::cout << word << ( ret ? " Is a word" : " Is not at word") << "\n";
                        if( ! ret ){
                                bad_words.emplace_back(word);
                        }
                }
                return true;
        }

        struct strategy{
                virtual ~strategy()=default;

                virtual player_move solve(board const& board, rack const& r, score_board const& sboard)=0;
                virtual std::shared_ptr<strategy> clone()=0;
        };

        using strategy_factory = generic_factory<strategy>;
        
        namespace auto_reg{

                std::shared_ptr<strategy> make_dict_solver(){
                        struct brute_force_strategy : strategy{
                                explicit brute_force_strategy(std::vector<std::string> const& dict):
                                        dict_(dict)
                                {}
                                player_move solve(board const& b, rack const& rck, score_board const& sboard){
                                        using std::get;
                                        /*
                                         * Strategy here is to using the perimeter of the board
                                         * as the starting point to every branch
                                         *
                                         * for every starting point, try putting a tile down, and then go thought every
                                         * second move
                                         *
                                         * NOTE that the first move has no direction, but after the second move every
                                         * subsequent action must be in the same direction
                                         *
                                         * def:
                                         *      starting point - square (x,y), with either 
                                         *              (x+d,y  ) is placed, or
                                         *              (x  ,y+d) is placed [where d \in {-1,+1}
                                         *
                                         *      valid move - placement of tiles T, such that
                                         *              the set of all words now on the board
                                         *              are all valid in the dictionary
                                         *
                                         *
                                         *
                                         */
                                        std::vector<std::tuple<size_t, size_t> > initial_moves;
			
                                        auto r = renderer_factory::get_inst()->make("cout_renderer");

                                        if( b( b.x_len()/2, b.y_len()/2) == '\0'){
                                                initial_moves.emplace_back( b.x_len()/2, b.y_len()/2 );
                                        } else {
                                                for(size_t x=0;x!=b.x_len();++x){
                                                        for(size_t y=0;y!=b.y_len();++y){
                                                                if( b(x,y) == '\0' ){
                                                                        if( (x != b.x_len() -1 && b(x+1,y) != '\0') ||
                                                                            (x != 0            && b(x-1,y) != '\0') ||
                                                                            (y != b.y_len() -1 && b(x,y+1) != '\0') ||
                                                                            (y != 0 && b(x,y-1) != '\0' ) )
                                                                        {
                                                                                initial_moves.emplace_back(x,y);
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                        boost::for_each( initial_moves, [](auto&& _){ std::cout << "\t(" << get<0>(_) << "," << get<1>(_) << ")\n";});

                                        enum class direction{
                                                x = 1,
                                                y = 2
                                        };

                                        struct context{
                                                size_t x;                  // first tile placement
                                                size_t y;                  // last tile placement
                                                direction dir;             // move direction
                                                board moves;               // board with placed tiles on 
                                                rack left;                 // tiles left to play

                                        };

                                        std::vector<context> output_stack;
                                        std::vector<context> stack;

                                        // XXX handle a rack of the form "CANDYAA"
                                        for( auto&& pos : initial_moves ){

                                                for( auto t : rck.make_tile_set() ){
                                                        // TODO blanks

                                                        context ctx = { 
                                                                get<0>(pos),
                                                                get<1>(pos),
                                                                direction::x,
                                                                b,
                                                                rck.clone_remove_tile(t)
                                                        };
                                                        ctx.moves(ctx.x, ctx.y) = t;
                                                        stack.emplace_back(ctx);

                                                        ctx.dir = direction::y;
                                                        stack.emplace_back(ctx);

                                                        auto tmp = b;
                                                        tmp.paste( ctx.moves, '\0');

                                                        r->render(tmp, sboard);

                                                        validate_move( dict_,b, ctx.moves );



                                                }
                                        }

                                        std::cout << "stack.size() = " << stack.size() << "\n";

                                        return skip_go{};
                                }
                                std::shared_ptr<strategy> clone(){
                                        return std::make_shared<brute_force_strategy>(dict_);
                                }
                        private:
                                std::vector<std::string> dict_;
                        };
                        std::ifstream fstr("dictionary.txt");
                        std::vector<std::string> proto_dict;
                        boost::sort( proto_dict);
                        for(;;){
                                std::string line;
                                std::getline(fstr, line);
                                if(line.size()){
                                        proto_dict.emplace_back(std::move(line));
                                }
                                if( fstr.eof() )
                                        break;
                        }
                        std::cout << proto_dict.size() << "\n";
                        return std::make_shared<brute_force_strategy>(proto_dict);
                }
                int reg_brute_force = ( strategy_factory::get_inst()
                                        ->register_( "brute_force", make_dict_solver()), 0 );
        }

}
