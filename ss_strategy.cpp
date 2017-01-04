#include "ss.h"
#include "ss_board.h"
#include "ss_driver.h"
#include "ss_dict.h"
#include "ss_util.h"

#include <fstream>
#include <functional>

namespace{
        using namespace ss;

        struct brute_force_strategy : strategy{
                explicit brute_force_strategy(std::shared_ptr<dictionary_t> dict):
                        dict_(std::move(dict))
                {}
                player_move solve(board const& b, rack const& rck){
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
                         *              are all valid in the dictionary_t
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
                        //boost::for_each( initial_moves, [](auto&& _){ std::cout << "\t(" << get<0>(_) << "," << get<1>(_) << ")\n";});

                        enum class direction{
                                left,
                                right,
                                up,
                                down
                        };

                        struct context{
                                size_t x;                  // first tile placement
                                size_t y;                  // last tile placement
                                direction dir;             // move direction
                                board b;                   // board with placed tiles on 
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
                                                direction::left,
                                                b,
                                                rck.clone_remove_tile(t)
                                        };
                                        ctx.b(ctx.x, ctx.y) = t;

                                        ctx.dir = direction::left;
                                        stack.emplace_back(ctx);
                                        ctx.dir = direction::right;
                                        stack.emplace_back(ctx);
                                        ctx.dir = direction::up;
                                        stack.emplace_back(ctx);
                                        ctx.dir = direction::down;
                                        stack.emplace_back(ctx);

                                        if( validate_board( *dict_, ctx.b) ){
                                                output_stack.push_back( ctx );
                                        }
                                }
                        }


                        for(; stack.size(); ){
                                auto top = stack.back();
                                stack.pop_back();

                                long x = top.x;
                                long y = top.y;

                                // There will be at most 2 moves possible
                                switch(top.dir){
                                case direction::left:
                                        --x;
                                        for(;0 <= x;--x){
                                                if( b(x,y) == '\0' ){
                                                        break;
                                                }
                                        }
                                        break;
                                case direction::right:
                                        ++x;
                                        for(;x != b.x_len();++x){
                                                if( b(x,y) == '\0' ){
                                                        break;
                                                }
                                        }
                                        break;
                                case direction::up:
                                        ++y;
                                        for(;y != b.y_len();++y){
                                                if( b(x,y) == '\0' ){
                                                        break;
                                                }
                                        }
                                        break;
                                case direction::down:
                                        --y;
                                        for(;0 <= y;--y){
                                                if( b(x,y) == '\0' ){
                                                        break;
                                                }
                                        }
                                        break;
                                }

                                if( ( 0 <= x && x < b.x_len() ) &&
                                    ( 0 <= y && y < b.y_len() ) ){

                                        for( auto t : top.left.make_tile_set() ){
                                                // TODO blanks
                                                
                                                context ctx = { 
                                                        static_cast<size_t>(x),
                                                        static_cast<size_t>(y),
                                                        top.dir,
                                                        top.b,
                                                        top.left.clone_remove_tile(t)
                                                };
                                                ctx.b(x, y) = t;

                                                //r->render(ctx.b);
                                        
                                                stack.emplace_back(ctx);

                                                if( validate_board( *dict_, ctx.b) ){
                                                        output_stack.push_back( ctx );
                                                }
                                        }
                                }

                        }

                        boost::sort( output_stack, [](auto&& left, auto&& right){
                                return left.left.size() < right.left.size();
                        });

                        if( output_stack.size() ){
                                r->render(output_stack.front().b);
                        } else {
                                std::cout << "no results :(\n";
                        }

                        std::cout << "output_stack.size() = " << output_stack.size() << "\n";

                        return skip_go{};
                }
                std::shared_ptr<strategy> clone(){
                        return std::make_shared<brute_force_strategy>(dict_);
                }
        private:
                std::shared_ptr<dictionary_t> dict_;
        };

        auto make_dict_solver(){
                return std::make_unique<brute_force_strategy>(
                        dictionary_factory::get_inst()->make("regular"));
        }
        int reg_brute_force = ( strategy_factory::get_inst() ->register_( "brute_force", 
                                                                          [](){
                                                                                return std::make_unique<brute_force_strategy>(
                                                                                        dictionary_factory::get_inst()->make("regular"));
                                                                          }
                                                                          ), 0 );
}
