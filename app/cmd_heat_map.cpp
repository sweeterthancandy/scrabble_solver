#include "driver_sub_command.h"
#include "game_context.h"
#include "ss.h"
#include "ss_orientation.h"

#include <fstream>

#include <boost/lexical_cast.hpp>

namespace{


struct heat_map : sub_command{
        virtual int run(game_context& ctx, std::vector<std::string> const& args){
                std::ifstream ifs("scrabble.json");
                game_context_io{}.read(ctx, ifs);
                ifs.close();

                enum Mode{
                        Mode_StartingPosition
                };
                boost::optional<ss::array_orientation> orien;

                auto printer = [](std::ostream& ostr, auto const& _){
                        ostr
                                << "{ "
                                << "\"word\":\"" << _.get_word() << "\""
                                << ", \"x\":" << _.get_x()
                                << ", \"y\":" << _.get_y()
                                << ", \"orientation\":" << "\"" << _.get_orientation() << "\""
                                << " }";
                };


                for(size_t i=2;i<args.size();){
                        size_t d{ args.size() - i };
                        switch(d){
                        default:
                                // fall
                        case 1:
                                if( args[i] == "--orientation"){
                                        if( args[i+1] == "vertical" )
                                                orien = ss::array_orientation::vertical;
                                        else 
                                                orien = ss::array_orientation::horizontal;
                                        i+=2;
                                        continue;
                                }
                                BOOST_THROW_EXCEPTION(std::domain_error("unknown argument"));
                        }
                }

                std::map<size_t, std::map< size_t, size_t> > heat;
                size_t sigma{0};


                auto strat{ ss::strategy_factory::get_inst()->make("fast_solver") };
                ss::rack rack{ ctx.players[ctx.active_player].rack };
                std::vector<std::vector<ss::word_placement> > all_placements;
                strat->yeild( ctx.board, rack, *ctx.dict_ptr, 
                               [&](std::vector<ss::word_placement> const& placements)mutable
                               {
                                        if( orien && orien != placements.front().get_orientation() )
                                                return;
                                        auto x{placements.front().get_x()};
                                        auto y{placements.front().get_y()};
                                        ++heat[x][y];
                                        ++sigma;
                               });
                size_t m{0};
                for(size_t x=0;x!=ctx.board.x_len(); ++x){
                        for(size_t y=0;y!=ctx.board.y_len(); ++y){
                                m = std::max(heat[x][y], m);
                        }
                }
                for(size_t x=0;x!=ctx.board.x_len(); ++x){
                        for(size_t y=0;y!=ctx.board.y_len(); ++y){
                                if( heat[x][y] == 0 )
                                        continue;
                                std::cout
                                        << "{ "
                                        <<   "\"metric\":" << heat[x][y]
                                        << ", \"sigma\":\"" << sigma << "\""
                                        << ", \"max\":\"" << m << "\""
                                        << ", \"x\":" << x
                                        << ", \"y\":" << y
                                        << " }\n";
                        }
                }


                return EXIT_SUCCESS;
        }
};

int heat_map_reg = (sub_command_factory::get()->register_("heat-map", [](){ return std::make_shared<heat_map>(); }), 0);

} // anon
