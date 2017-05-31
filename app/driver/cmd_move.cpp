#include "sub_command.h"
#include "game_context.h"
#include "ss.h"

#include <fstream>

namespace{

std::vector<ss::word_placement> compile_move(ss::board const& original, ss::board const& next){
        // get diff, and make sure that we havn't deleted any times,
        // ie
        //              original \setminus minus = {}
        //
        std::vector< std::pair< size_t, size_t> > diff;
        bool board_empty{true};
        for(size_t x=0;x!=original.x_len();++x){
                for(size_t y=0;y!=original.y_len();++y){

                        if( original(x,y) == ' '){
                                if( next(x,y) != original(x,y) ){
                                        diff.emplace_back(x,y);
                                }
                        } else{
                                board_empty = false;
                                if( original(x,y) != next(x,y) ){
                                        // bad board
                                        // just rerender 
                                        std::stringstream sstr;
                                        sstr << "bad board, re-rendering <" << x << "," << y << ">";
                                        BOOST_THROW_EXCEPTION(std::domain_error(sstr.str()));
                                }
                        }
                }
        }

        // now validate move

        if( diff.empty() ){
                BOOST_THROW_EXCEPTION(std::domain_error("nothing to do"));
        }

        ss::array_orientation orientation{ ss::array_orientation::horizontal };

        if( 
            std::min_element( diff.begin(), diff.end(), [](auto const& l, auto const& r){ return l.first < r.first; } )->first ==
            std::max_element( diff.begin(), diff.end(), [](auto const& l, auto const& r){ return l.first < r.first; } )->first ){
                orientation = ss::array_orientation::vertical;
        } else if(
            std::min_element( diff.begin(), diff.end(), [](auto const& l, auto const& r){ return l.second < r.second; } )->second ==
            std::max_element( diff.begin(), diff.end(), [](auto const& l, auto const& r){ return l.second < r.second; } )->second ){
                orientation = ss::array_orientation::horizontal;
        } else{
                BOOST_THROW_EXCEPTION(std::domain_error("invalid move"));
        }

        std::vector<ss::word_placement> placements;

        for( auto const& p : diff){
                std::cout << "{" << p.first << "," << p.second << "}\n";
        }

        if( board_empty ){
                if( boost::find_if( diff, [](auto const& p){ return p.first == 7 && p.second == 7; } ) == diff.end()){
                        BOOST_THROW_EXCEPTION(std::domain_error("first move must cross the middle <8,8>"));
                }
        }

        // check tiles are seqiemntial, ie
        //
        //         X   X
        //       --X---X
        //         X   X
        //         XXXXX
        //
        // is valid, whilst 
        //
        //         X   X
        //       --X- -X-
        //         X   X
        //         XXXXX
        //
        // isn't, as they arn't equential
        //
        // TODO mask
        PRINT(orientation);
        if( orientation == ss::array_orientation::vertical ){
                boost::sort( diff, [](auto const& l, auto const& r){ return l.second < r.second; } );
                size_t x{ diff.front().first };
                size_t y{ diff.front().second  };
                std::string rev_left;
                        
                for( size_t z{ y }; z != 0; ){
                        --z;
                        if( next(x,z) == ' ')
                                break;
                        rev_left += next(x,z);
                }
                std::string left{ rev_left.rbegin(), rev_left.rend() };
                
                std::string word{ left };

                for(; y < original.y_len() && next(x,y) != ' '; ++y)
                        word+=next(x,y);

                if( y <= diff.back().second ){
                        BOOST_THROW_EXCEPTION(std::domain_error("tile placement not sequential"));
                }

                // special case, can place one tile as in
                //
                //              HAT
                //               T   <------ place this tile
                //               
                if( word.size() > 1){
                        placements.emplace_back( 
                                make_word_placement(
                                        original,
                                        diff.front().first,
                                        diff.front().second - left.size(),
                                        orientation,
                                        word) );
                }
                
        } else{
                boost::sort( diff, [](auto const& l, auto const& r){ return l.first < r.first; } );
                size_t x{ diff.front().first };
                size_t y{ diff.front().second };
                
                std::string rev_left;
                        
                for( size_t z{ x }; z != 0; ){
                        --z;
                        if( next(z,y) == ' ')
                                break;
                        rev_left += next(z,y);
                }
                std::string left{ rev_left.rbegin(), rev_left.rend() };
                
                std::string word{ left };
                

                for(; x < original.x_len() && next(x,y) != ' '; ++x){
                        word+=next(x,y);
                }

                if( x <= diff.back().first ){
                        BOOST_THROW_EXCEPTION(std::domain_error("tile placement not sequential"));
                }

                if( word.size() > 1){
                        placements.emplace_back( 
                                make_word_placement(
                                        original,
                                        diff.front().first - left.size(),
                                        diff.front().second,
                                        orientation,
                                        word));
                }
        }

        // now check perpendicular ones
        for( auto const& d : diff){
                if( orientation == ss::array_orientation::vertical ){
                        // horizontal perpindicular
                        size_t y{d.second};
                        std::string rev_left; 
                        std::string right;

                        for( size_t x{ d.first }; x != 0; ){
                                --x;
                                if( next(x,y) == ' ')
                                        break;
                                rev_left += next(x,y);
                        }
                        for(size_t x{d.first+1}; x < original.x_len(); ++x){
                                if( next(x,y) == ' ')
                                        break;
                                right += next(x,y);
                        }
                        std::string left{rev_left.rbegin(), rev_left.rend()};
                        std::string word{ left + next(d.first, y ) + right };

                        //PRINT_SEQ((left)(right)(word));

                        if( word.size() > 1){
                                placements.emplace_back(
                                        make_word_placement(
                                                original,
                                                d.first - left.size(),
                                                y,
                                                ss::array_orientation::horizontal,
                                                word));
                        }
                } else {
                        // vertial perpindicular
                        size_t x{d.first};
                        std::string rev_left; 
                        std::string right;

                        for( size_t y{ d.second }; y != 0; ){
                                --y;
                                if( next(x,y) == ' ')
                                        break;
                                rev_left += next(x,y);
                        }
                        for(size_t y{d.second+1}; y < original.y_len(); ++y){
                                if( next(x, y) == ' ')
                                        break;
                                right += next(x, y);
                        }
                        std::string left{rev_left.rbegin(), rev_left.rend()};
                        std::string word{ left + next(x, d.second ) + right };

                        //PRINT_SEQ((left)(right)(word));

                        if( word.size() > 1){
                                placements.emplace_back(
                                        make_word_placement(
                                                original,
                                                x,
                                                d.second - left.size(),
                                                ss::array_orientation::horizontal,
                                                word));
                        }
                }
        }
        return std::move(placements);
}


struct move : sub_command{
        virtual int run(game_context& ctx, std::vector<std::string> const& args){
                do{
                        std::ifstream ifs("scrabble.json");
                        ctx.read(ifs);
                }while(0);

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

                auto placements{ compile_move( ctx.board, next) };

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
                        PRINT(all_placements.size());
                        if( all_placements.size()){
                                boost::sort( all_placements, [&](auto const& l, auto const& r){
                                        return ctx.metric_ptr->calculate(l) < ctx.metric_ptr->calculate(r);
                                });

                                ctx.apply_placements( all_placements.back() );
                        }

                }



                return EXIT_SUCCESS;
        }
};

int move_reg = (sub_command_factory::get()->register_("move", [](){ return std::make_shared<move>(); }), 0);

} // anon
