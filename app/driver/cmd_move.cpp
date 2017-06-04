#include "sub_command.h"
#include "game_context.h"
#include "ss_algorithm.h"
#include "ss_strategy.h"

#include <fstream>

namespace{

struct move : sub_command{
        virtual int run(game_context& ctx, std::vector<std::string> const& args){
                do{
                        std::ifstream ifs("scrabble.json");
                        ctx.read(ifs);
                }while(0);

                try{

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

                        for( auto const& p : placements ){
                                if( ! ctx.dict_ptr->contains(p.get_word() ) ){
                                        std::stringstream sstr;
                                        sstr << "invalid word [" << p.get_word() << "]";
                                        ctx.log.push_back(sstr.str());
                                        return EXIT_SUCCESS;
                                }
                        }


                        ctx.apply_placements( placements );

                        for(;;){

                                if( ctx.players[ctx.active_player].backend == "player")
                                        break;


                                // player the au
                                auto strat{ ss::strategy_factory::get_inst()->make("fast_solver") };
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
                                if( all_placements.size()){
                                        boost::sort( all_placements, [&](auto const& l, auto const& r){
                                                return ctx.metric_ptr->calculate(l) < ctx.metric_ptr->calculate(r);
                                        });

                                        ctx.apply_placements( all_placements.back() );
                                }

                        }
                        std::ofstream of("scrabble.json");
                        ctx.write(of);
                        std::ofstream scof(ctx.scratch);
                        ctx.render(scof);
                } catch(...){
                        game_context other;
                        std::ifstream ifs("scrabble.json");
                        other.read(ifs);
                        std::ofstream of("scrabble.json");
                        other.write(of);
                        std::ofstream scof(other.scratch);
                        other.render(scof);
                        throw;
                }

                return EXIT_SUCCESS;
        }
};

int move_reg = (sub_command_factory::get()->register_("move", [](){ return std::make_shared<move>(); }), 0);

} // anon
