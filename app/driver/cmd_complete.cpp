#include "sub_command.h"
#include "game_context.h"
#include "ss.h"

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
                std::function<bool(std::vector<ss::word_placement> const&)> f{
                        [](std::vector<ss::word_placement> const& _){ return true; }};

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
                if( ( ! x ) != ( ! y ) ){
                        BOOST_THROW_EXCEPTION(std::domain_error("need --x <x> --y <y>"));
                } else if( x && y ){
                        f = [&](std::vector<ss::word_placement> const& p){
                                return ( p.front().get_x() == *x && p.front().get_y() == *y );
                        };
                }



                auto strat{ ss::strategy_factory::get_inst()->make("fast_solver") };
                ss::rack rack{ ctx.players[ctx.active_player].rack };
                std::vector<std::vector<ss::word_placement> > all_placements;
                strat->yeild( ctx.board, rack, *ctx.dict_ptr, 
                               [&](std::vector<ss::word_placement> const& placements)mutable
                               {
                                        if( f(placements) ) {
                                                consumer(placements);
                                        }
                               });

                return EXIT_SUCCESS;
        }
};

int complete_reg = (sub_command_factory::get()->register_("complete", [](){ return std::make_shared<complete>(); }), 0);

} // anon
