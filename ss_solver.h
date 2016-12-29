#pragma once

#include "ss.h"
#include "ss_board.h"
#include "ss_driver.h"

#include <fstream>

namespace ss{

        struct solve_context{
                std::vector<tile_t> task;
        };

        struct strategy{
                virtual ~strategy()=default;

                virtual player_move solve(board const& board, std::vector<tile_t> const& rack, score_board const& sboard)=0;
                virtual std::shared_ptr<strategy> clone()=0;
        };

        using strategy_factory = generic_factory<strategy>;
        
        namespace auto_reg{

                std::shared_ptr<strategy> make_dict_solver(){
                        struct brute_force_strategy : strategy{
                                explicit brute_force_strategy(std::vector<std::string> const& dict):
                                        dict_(dict)
                                {}
                                player_move solve(board const& board, std::vector<tile_t> const& rack, score_board const& sboard){
                                        /*
                                         * Strategy here is to using the perimeter of the board
                                         * as the starting point to every iteration, then for every starting
                                         * point, see if any length permutation of the letters are a valid word.
                                         * For any valid move, push on to the candidate stack
                                         *
                                         * return the candidate move with the hightest score
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

                                        if( board( board.x_len()/2, board.y_len()/2) == '\0'){
                                                initial_moves.emplace_back( board.x_len()/2, board.y_len()/2 );
                                        } else {
                                                for(size_t x=0;x!=board.x_len();++x){
                                                        for(size_t y=0;y!=board.y_len();++y){
                                                                if( board(x,y) == '\0' ){
                                                                        if( x != board.x_len() -1 && board(x+1,y) != '\0' ){
                                                                                initial_moves.emplace_back(x,y);
                                                                                continue;
                                                                        }
                                                                        if( x != 0 && board(x-1,y) != '\0' ){
                                                                                initial_moves.emplace_back(x,y);
                                                                                continue;
                                                                        }
                                                                        if( y != board.y_len() -1 && board(x,y+1) != '\0' ){
                                                                                initial_moves.emplace_back(x,y);
                                                                                continue;
                                                                        }
                                                                        if( y != 0 && board(x,y-1) != '\0' ){
                                                                                initial_moves.emplace_back(x,y);
                                                                                continue;
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                        using std::get;
                                        boost::for_each( initial_moves, [](auto&& _){ std::cout << "\t(" << get<0>(_) << "," << get<1>(_) << ")\n";});
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
