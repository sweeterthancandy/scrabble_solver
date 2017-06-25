#include "game_context.h"

#include "driver_sub_command.h"
#include "game_context.h"
#include "ss_algorithm.h"
#include "ss_strategy.h"
#include "ss_print.h"

#include <fstream>

namespace{
        struct vplayer_text : vplayer{
                void post_exec(game_context& ctx){
                        game_context_io{}.write_all(ctx);
                }
                player_move exec(game_context& ctx){
                        std::ifstream scr(ctx.scratch);
                        auto presult{ game_context_io{}.parse(ctx, scr ) };
                        if( ! presult )
                                return meta_rewrite{};

                        do{
                                std::string rs_masked;
                                for( char c : presult->r ){
                                        if( c == ' ' )
                                                continue;
                                        rs_masked += c;
                                }
                                ss::rack orig_r{ ctx.get_active()->rack };
                                std::string orig_rs{ orig_r.to_string() };
                                
                                PRINT_SEQ((presult->r)(rs_masked)(orig_r));
                                boost::sort(orig_rs);
                                boost::sort(rs_masked);
                                PRINT_SEQ((orig_rs)(rs_masked));

                                if( ! boost::includes( orig_rs, rs_masked ) ){
                                        std::cerr << "bad exchange\n";
                                        return meta_rewrite{};
                                }
                                std::string d;
                                boost::set_difference( orig_rs, rs_masked, std::back_inserter(d) );
                                
                                PRINT_SEQ((presult->r)(rs_masked)(orig_r)(d));

                                if( d.size() ){
                                        return exchange_t{ d };
                                }
                                // need 

                        }while(0);

                        

                        // XXX can't place a single tile on first go

                        auto placements{ ss::algorithm::compile_move( ctx.board, presult->b) };

                        if( placements.empty() )
                                return skip_go_t{};

                        for( auto const& p : placements ){
                                if( ! ctx.dict_ptr->contains(p.get_word() ) ){
                                        std::stringstream sstr;
                                        sstr << "invalid word [" << p.get_word() << "]";
                                        ctx.log.push_back(sstr.str());
                                        return meta_rewrite{};
                                }
                        }


                        return placements;
                }
        };
        int reg= ( vplayer_factory::get_inst() ->register_( "text", 
                                                                          [](){
                                                                                return std::make_unique<vplayer_text>();
                                                                          }
                                                                          ), 0 );
}
