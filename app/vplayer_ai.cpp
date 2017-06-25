#include "game_context.h"

#include "sub_command.h"
#include "game_context.h"
#include "ss_algorithm.h"
#include "ss_strategy.h"

#include <fstream>

namespace{
        struct vplayer_ai : vplayer{
                player_move exec(game_context& ctx)override{
                        // player the au
                        
                        std::shared_ptr<ss::strategy> strat;
                        switch(ctx.state){
                        case State_FirstPlacement:
                                strat = ss::strategy_factory::get_inst()->make("first");
                                break;
                        default:
                                strat = ss::strategy_factory::get_inst()->make("fast_solver");
                                break;
                        }
                        ss::rack rack{ ctx.players[ctx.active_player].rack };
                        std::vector<std::vector<ss::word_placement> > all_placements;
                        strat->yeild( ctx.board, rack, *ctx.dict_ptr, 
                                       [&](std::vector<ss::word_placement> const& placements)mutable
                                       {
                                        std::cout << "[";
                                        for( auto const& p : placements ){
                                                std::cout << p << ", ";
                                        }
                                        std::cout << "]\n";
                                                all_placements.push_back( placements );
                                       });
                        #if 0
                        for( auto const& m : all_placements ){
                                        std::cout << "[";
                                        for( auto const& p : m ){
                                                std::cout << p << ", ";
                                        }
                                        std::cout << "]\n";
                        }
                        #endif
                        if( all_placements.empty())
                                return skip_go_t{};

                        boost::sort( all_placements, [&](auto const& l, auto const& r){
                                return ctx.metric_ptr->calculate(l) < ctx.metric_ptr->calculate(r);
                        });

                        return all_placements.back();

                }
        };
        int reg= ( vplayer_factory::get_inst() ->register_( "ai", 
                                                                                  [](){
                                                                                        return std::make_unique<vplayer_ai>();
                                                                                  }
                                                                                  ), 0 );
}
