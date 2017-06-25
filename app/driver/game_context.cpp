#include "game_context.h" 
#include "ss_print.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>

#include "ss.h"

namespace bpt = boost::property_tree;

void game_context::write(std::ostream& ostr)const{
        bpt::ptree root;
        root.put("bag", bag);
        root.put("active_player", active_player);
        root.put("width", width);
        root.put("height", height);
        root.put("scratch", scratch);
        root.put("dict", dict);
        root.put("metric", metric);
        root.put("state", state);
        for( auto const& item : log )
                root.add("logs.log", item);
        for( auto const& item : moves )
                root.add("moves.move", item);
        for( auto const& p : players){
                bpt::ptree aux;
                aux.put("backend", p.backend);
                aux.put("rack", p.rack);
                for( auto s : p.score )
                        aux.add("scores.score", s);
                root.add_child("players.player", aux);
        }
        for( auto const& l : board.to_string_vec()){
                root.add("board.line", l);
        }

        bpt::write_json(ostr, root);
}
void game_context::read(std::istream& ostr){
        bpt::ptree root;
        bpt::read_json(ostr, root);
        bag = root.get<std::string>("bag");
        scratch = root.get<std::string>("scratch");
        dict = root.get<std::string>("dict");
        dict_ptr =  ss::dictionary_factory::get_inst()->make(dict);
        metric = root.get<std::string>("metric");
        metric_ptr =  ss::metric_factory::get_inst()->make(metric);
        active_player = root.get<size_t>("active_player");
        width = root.get<size_t>("width");
        height = root.get<size_t>("height");
        state = static_cast<game_state>(root.get<int>("state"));
        for( auto const& p : root.get_child("players")){
                players.emplace_back();
                players.back().backend = p.second.get<std::string>("backend");
                players.back().rack    = p.second.get<std::string>("rack");
                players.back().vp      = vplayer_factory::get_inst()->make( players.back().backend );
                auto opt{ p.second.get_child_optional("scores") };
                if( opt ){
                        for( auto const& s : *opt)
                             players.back().score.emplace_back( boost::lexical_cast<unsigned>(s.second.data()) );
                }
        }
        for( auto const& p : root.get_child("moves")){
                moves.push_back(p.second.data());
        }
        for( auto const& p : root.get_child("logs")){
                log.push_back(p.second.data());
        }
        size_t y=0;
        board = ss::board(width, height);
        for( auto const& c : root.get_child("board") ){
                //std::string l{ c.second.get<std::string>("line") };
                std::string l{ c.second.data() };
                for( size_t x=0;x!=l.size();++x){
                        board(x,y) = l[x];
                }
                ++y;
        }
}
void game_context::render(std::ostream& ostr)const{
        ostr << "          SCRABBLE                                                                                  \n";
        ostr << "\n";
        ostr << std::string(5, ' ');
        for(size_t i=0;i!=width;++i)
                ostr << boost::lexical_cast<std::string>(i%10);
        ostr << "\n";
        auto sv{ board.to_string_vec() };
        int i{0};
        std::string top{ std::string(4,' ') + "+" + std::string(width,'-') + "+"};

        ostr << top << "\n";
        for( auto const& line : sv ){
                ostr << "   " << (i%10) << "|" << line << "|\n";
                ++i;
        }
        ostr << top << "\n";
        ostr << "\n";
        /////////////////////////////////////////////////////////////////////////
        // Scores
        /////////////////////////////////////////////////////////////////////////
        ostr << "        |" << players[active_player].rack << "|\n";
        ostr << "    Score\n"
             << "\n";
        for(auto const& p : players )
                ostr << std::setw(11) << p.backend << "|";
        ostr << "\n------------------------\n";
        std::vector<unsigned> sigma( players.size(), 0 );
        for( size_t i=0;;++i){
                bool end{false};
                for( size_t j=0;j!=players.size();++j){
                        auto const& p{ players[j] };
                        if( ! ( i < p.score.size() )) {
                                end = true;
                                ostr << std::setw(5) << "" << std::setw(0) << "|"
                                     << std::setw(5) << "" << std::setw(0) << "|";
                        } else {
                                sigma[j] += p.score[i];
                                ostr << std::setw(5) << p.score[i] << std::setw(0) << "|"
                                     << std::setw(5) << sigma[j]   << std::setw(0) << "|";
                        }
                }
                ostr << "\n";
                if( end ) break;
        }

        /////////////////////////////////////////////////////////////////////////
        // logs
        /////////////////////////////////////////////////////////////////////////

        ostr << "\nmoves:\n    ";
        boost::copy( moves, std::ostream_iterator<std::string>(ostr, "\n    "));
        ostr << "\nlogs:\n    ";
        boost::copy( log, std::ostream_iterator<std::string>(ostr, "\n    "));
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
        p.score.push_back(score);

        if( p.rack.size() == 0 ){
                state = State_Finished;
        } else{
                state = State_Running; // bacause this could of been first move

                ++active_player;
                active_player = active_player % players.size();
        }

}
