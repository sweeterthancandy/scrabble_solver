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
                        /*
                        ostr << "          SCRABBLE\n";
                        ostr << "\n";
                        ostr << "     +---------------+\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     |               |\n";
                        ostr << "     +---------------+\n";
                        */
                        std::vector<std::string> lines;
                        std::ifstream scr("scrabble.scratch");
                        for(;;){
                                std::string line;
                                std::getline(scr, line);
                                lines.push_back(std::move(line));
                                if( scr.eof())
                                        break;
                        }

                        std::cout << "line=\n";
                        boost::copy( lines, std::ostream_iterator<std::string>(std::cout,"\n"));

                        ss::board next(ctx.width, ctx.height);
                        size_t y_offset{4};
                        size_t x_offset{5};
                        for(size_t y=0;y!=ctx.height;++y){
                                for(size_t x=0;x!=ctx.height;++x){
                                        next(x,y) = lines[y+y_offset][x+x_offset];
                                }
                        }
                        std::cout << "next=\n";
                        next.dump();
                        std::cout << "board=\n";
                        ctx.board.dump();

                        size_t rack_offset{21};
                        std::string rack_s{lines[rack_offset]};
                        do{
                                auto first{ rack_s.find_first_of('|') };
                                auto last{ rack_s.find_last_of('|') };
                                // should never happen
                                if( first == std::string::npos )
                                        break;
                                std::string rs{ rack_s.substr(first+1, last - first -1 ) };
                                std::string rs_masked;
                                for( char c : rs ){
                                        if( c == ' ' )
                                                continue;
                                        rs_masked += c;
                                }
                                ss::rack orig_r{ ctx.get_active()->rack };
                                std::string orig_rs{ orig_r.to_string() };
                                
                                PRINT_SEQ((rack_s)(rs)(rs_masked)(orig_r));
                                boost::sort(orig_rs);
                                boost::sort(rs_masked);
                                PRINT_SEQ((orig_rs)(rs_masked));

                                if( ! boost::includes( orig_rs, rs_masked ) ){
                                        std::cerr << "bad exchange\n";
                                        return meta_rewrite{};
                                }
                                std::string d;
                                boost::set_difference( orig_rs, rs_masked, std::back_inserter(d) );
                                
                                PRINT_SEQ((rack_s)(rs)(rs_masked)(orig_r)(d));

                                if( d.size() ){
                                        return exchange_t{ d };
                                }
                                // need 

                        }while(0);

                        

                        // XXX can't place a single tile on first go

                        auto placements{ ss::algorithm::compile_move( ctx.board, next) };

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
