#include "sub_command.h"
#include "game_context.h"
#include "ss.h"
#include "ss_orientation.h"

#include <fstream>

#include <boost/lexical_cast.hpp>

namespace{


struct complete : sub_command{
        virtual int run(game_context& ctx, std::vector<std::string> const& args){
                do{
                        std::ifstream ifs("scrabble.json");
                        ctx.read(ifs);
                }while(0);

                boost::optional<int> x;
                boost::optional<int> y;
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

                std::function<void(std::vector<ss::word_placement> const&)> consumer{
                        [&](std::vector<ss::word_placement> const& _){
                                printer(std::cout, _.front());
                                std::cout << "\n";
                        }
                };

                for(size_t i=2;i<args.size();){
                        size_t d{ args.size() - i };
                        switch(d){
                        default:
                                if( args[i] == "--x" ){
                                        x = boost::lexical_cast<int>(args[i+1]);
                                        i+=2;
                                        continue;
                                }
                                if( args[i] == "--y" ){
                                        y = boost::lexical_cast<int>(args[i+1]);
                                        i+=2;
                                        continue;
                                }
                                if( args[i] == "--orientation"){
                                        if( args[i+1] == "vertical" )
                                                orien = ss::array_orientation::vertical;
                                        else 
                                                orien = ss::array_orientation::horizontal;
                                        i+=2;
                                        continue;
                                }
                                // fall
                        case 1:
                                if( args[i] == "--all" ){
                                        consumer =  
                                                [&](std::vector<ss::word_placement> const& _){
                                                        for(auto const& p : _ ){
                                                                printer(std::cout, p);
                                                                std::cout << ",";
                                                        }
                                                        std::cout << "\n";
                                                };
                                        i+=1;
                                        continue;
                                }
                                BOOST_THROW_EXCEPTION(std::domain_error("expected --x <x> --y <y>"));
                        }
                }



                
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
                                        if( x && *x != placements.front().get_x() )
                                                return;
                                        if( y && *y != placements.front().get_y() )
                                                return;
                                        if( orien && orien != placements.front().get_orientation() )
                                                return;
                                        consumer(placements);
                               });

                return EXIT_SUCCESS;
        }
};

int complete_reg = (sub_command_factory::get()->register_("complete", [](){ return std::make_shared<complete>(); }), 0);

} // anon
