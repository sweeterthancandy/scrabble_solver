#include "driver_sub_command.h"
#include "game_context.h"

namespace{

std::map<char, size_t> tile_dist ={
        {'A', 9},
        {'B', 2},
        {'C', 2},
        {'D', 4},
        {'E',12},
        {'F', 2},
        {'G', 3},
        {'H', 2},
        {'I', 9},
        {'J', 1},
        {'K', 1},
        {'L', 4},
        {'M', 2},
        {'N', 6},
        {'O', 8},
        {'P', 2},
        {'Q', 1},
        {'R', 6},
        {'S', 4},
        {'T', 6},
        {'U', 4},
        {'V', 2},
        {'W', 2},
        {'X', 1},
        {'Y', 2},
        {'Z', 1},
};

struct init : sub_command{
        virtual int run(game_context& ctx, std::vector<std::string> const& args){
                for(auto const& p : tile_dist )
                        ctx.bag  += std::string(p.second, p.first);
                std::shuffle( ctx.bag.begin(), ctx.bag.end(), ctx.gen);

                ctx.width = 15;
                ctx.height = 15;
                ctx.board = ss::board(ctx.width, ctx.height);
                ctx.players.emplace_back();
                ctx.players.back().backend = "ai";
                ctx.players.back().rack = ctx.bag.substr(ctx.bag.size()-7);
                ctx.bag.resize( ctx.bag.size() - 7 );
                ctx.players.emplace_back();
                ctx.players.back().backend = "ai";
                ctx.players.back().rack = ctx.bag.substr(ctx.bag.size()-7);
                ctx.bag.resize( ctx.bag.size() - 7 );
                ctx.active_player = 0;
                ctx.dict = "regular";
                ctx.metric = "scrabble_metric";
                ctx.state = State_FirstPlacement;
                ctx.skips = 0;
                ctx.winner = -1;
                
                ctx.log.push_back( "create new game");
                ctx.moves.push_back( "start");
                ctx.debug.push_back( "start");


                game_context_io().write_all(ctx);

                return EXIT_SUCCESS;
        }
};

int init_reg = (sub_command_factory::get()->register_("init", [](){ return std::make_shared<init>(); }), 0);

} // anon
