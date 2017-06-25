#include "game_context.h" 
#include "ss_print.h"

#include <algorithm>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>


#include "ss.h"


void game_context::skip_go(){
        if( ++skips == players.size() * 3 ){
                on_finish_();
        }
        next_();
}
void game_context::exchange(std::string const& s){
        // can't exchange when less than 7 tiles
        if( bag.size() < 7 )
                BOOST_THROW_EXCEPTION((std::domain_error("not enough tiles for exchange")));

        std::string to_remove(s);
        // want to preservce order of get_active()->rack

        for(char& c : get_active()->rack ){
                auto iter = boost::find(to_remove, c );
                if( iter == to_remove.end() )
                        continue;
                c = bag.back();
                bag.pop_back();
                *iter = ' '; // zero it out
        }
        std::stringstream sstr;
        sstr << "Ex ";
        boost::copy( s, std::ostream_iterator<char>(sstr,","));
        get_active()->score.emplace_back(0, sstr.str());

        bag += s;
        std::shuffle( bag.begin(), bag.end(), gen);

        next_();

}
// first placement is the real one, ie the one where the mask represents the tiles placed, the others are perps
void game_context::apply_placements(std::vector<ss::word_placement> const& placements){
        auto score{ metric_ptr->calculate(placements) };

        auto& p{ players[active_player] };
        for( auto const& p : placements ){
                std::cout << p;
        }

        std::vector<char> to_remove;
        PRINT(placements.front().get_mask() );
        for( char c : placements.front().get_mask() ){
                if( ss::tile_traits::not_empty(c) ){
                        PRINT( p.rack );
                        auto iter{ p.rack.find(c) };
                        assert( iter != std::string::npos && "unexpected");
                        p.rack.erase(iter, 1);
                }
        }
        PRINT( p.rack );
                        
        for(;p.rack.size() < 7 && bag.size();){
                p.rack += bag.back();
                bag.pop_back();
        }


        // need to calculate tiles used without double counting

        for( auto const& p : placements ){
                size_t y{ p.get_y()};
                size_t x{ p.get_x()};
                auto word{ p.get_word() };
                if( p.get_orientation() == ss::array_orientation::horizontal ){
                        for(size_t i{0};i!=word.size();++i){
                                board(x +i, y) = word[i];
                        }
                } else{
                        for(size_t i{0};i!=word.size();++i){
                                board(x, y+i) = word[i];
                        }
                }
        }
        std::stringstream sstr;
        sstr << "player " << active_player << " placed " << placements.front().get_word() << " at <" << placements.front().get_x() << "," << placements.front().get_y() << "> for " << score << " points";
        moves.emplace_back(sstr.str());
        p.score.emplace_back(score, placements.front().get_word());

        if( p.rack.size() == 0 ){
                on_finish_();


        } else{
                state = State_Running; // bacause this could of been first move

                next_();

        }

}
void game_context::next_(){
        ++active_player;
        active_player = active_player % players.size();
}
void game_context::on_finish_(){
        state = State_Finished;

        unsigned max_score{0};
        size_t   max_idx{0};
        for(size_t idx{0};idx!=players.size();++idx){
                auto s{ players[idx].sigma() };
                if( s > max_score ){
                        max_score = s;
                        max_idx   = idx;
                }
        }
        std::stringstream sstr;
        sstr << "player " << max_idx << " wins with " << max_score;
        log.push_back(sstr.str());
}
