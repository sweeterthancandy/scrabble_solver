#include "driver_sub_command.h"
#include "game_context.h"
#include "ss_algorithm.h"
#include "ss_strategy.h"
#include "ss_print.h"

#include <fstream>

namespace{


struct move : sub_command{
        virtual int run(game_context& ctx, std::vector<std::string> const& args){
                std::ifstream ifs("scrabble.json");
                game_context_io{}.read(ctx, ifs);
                ifs.close();



                // we can't have everyone skip there go 3 times in a row

                try{
                        for(; ctx.state != State_Finished;){
                                PRINT_SEQ((ctx.active_player));
                                auto ap{ ctx.get_active() };
                                auto m{ ap->vp->exec( ctx ) };

                                if( boost::get<skip_go_t>(&m) ){
                                        ctx.skip_go();
                                } else if ( auto ptr = boost::get<exchange_t>(&m) ){
                                        ctx.exchange( ptr->get() );
                                } else if ( auto ptr = boost::get<std::vector<ss::word_placement> >(&m) ){
                                        ctx.apply_placements( *ptr );
                                } else if ( boost::get<meta_rewrite>(&m)){
                                        game_context_io{}.write_all(ctx);
                                        break;
                                }

                                ap->vp->post_exec(ctx);
                        }
                        game_context_io{}.write_all(ctx);
                } catch(...){
                        game_context_io{}.write_all(ctx);
                        throw;
                }
                return EXIT_SUCCESS;
        }
};

int move_reg = (sub_command_factory::get()->register_("move", [](){ return std::make_shared<move>(); }), 0);

} // anon
