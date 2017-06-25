#include "game_context.h"

#include "sub_command.h"
#include "game_context.h"
#include "ss_algorithm.h"
#include "ss_strategy.h"

#include <fstream>

namespace{
        struct vplayer_text : vplayer{
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
