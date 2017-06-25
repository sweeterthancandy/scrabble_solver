#include "sub_command.h"
#include "game_context.h"
#include "ss_algorithm.h"
#include "ss_strategy.h"

#include <fstream>

namespace{


struct move : sub_command{
        virtual int run(game_context& ctx, std::vector<std::string> const& args){
                std::ifstream ifs("scrabble.json");
                ctx.read(ifs);
                ifs.close();

                auto write = [&](){
                        std::ofstream of("scrabble.json");
                        ctx.write(of);
                        std::ofstream scof(ctx.scratch);
                        ctx.render(scof);
                };


                try{
                        for(; ctx.state != State_finished;){
                                auto m{ ctx.players[ctx.active_player].vp->exec( ctx ) };

                                if( boost::get<skip_go_t>(&m) ){
                                        std::cout << "skipping\n";
                                        continue;
                                } else if ( boost::get<meta_rewrite>(&m)){
                                        std::cout << "rewriting\n";
                                        break;
                                } else if ( auto ptr = boost::get<std::vector<ss::word_placement> >(&m) ){
                                        std::cout << "moving\n";
                                        ctx.apply_placements( *ptr );
                                }
                        }
                        write();
                } catch(...){
                        write();
                        throw;
                }
                return EXIT_SUCCESS;
        }
};

int move_reg = (sub_command_factory::get()->register_("move", [](){ return std::make_shared<move>(); }), 0);

} // anon
