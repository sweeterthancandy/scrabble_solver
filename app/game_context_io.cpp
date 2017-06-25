#include "game_context_io.h"

#include <algorithm>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>

namespace bpt = boost::property_tree;

void game_context_io::write(game_context const& ctx, std::ostream& ostr)const{
        bpt::ptree root;
        root.put("bag", ctx.bag);
        root.put("active_player", ctx.active_player);
        root.put("width", ctx.width);
        root.put("height", ctx.height);
        root.put("scratch", ctx.scratch);
        root.put("dict", ctx.dict);
        root.put("metric", ctx.metric);
        root.put("state", ctx.state);
        root.put("skips", ctx.skips);
        root.put("is_rotated", ctx.is_rotated);
        for( auto const& item : ctx.log )
                root.add("logs.log", item);
        for( auto const& item : ctx.moves )
                root.add("moves.move", item);
        for( auto const& item : ctx.debug )
                root.add("debug.log", item);
        for( auto const& p : ctx.players){
                bpt::ptree aux;
                aux.put("backend", p.backend);
                aux.put("rack", p.rack);
                for( auto s : p.score ){
                        using std::get;
                        bpt::ptree p;
                        p.put("value", std::get<0>(s));
                        p.put("word" , std::get<1>(s));
                        aux.add_child("scores.score", p);
                }
                root.add_child("players.player", aux);
        }
        for( auto const& l : ctx.board.to_string_vec()){
                root.add("board.line", l);
        }

        bpt::write_json(ostr, root);
}
void game_context_io::read(game_context& ctx, std::istream& ostr)const{
        bpt::ptree root;
        bpt::read_json(ostr, root);
        ctx.bag           = root.get<std::string>("bag");
        ctx.scratch       = root.get<std::string>("scratch");
        ctx.dict          = root.get<std::string>("dict");
        ctx.dict_ptr      = ss::dictionary_factory::get_inst()->make(ctx.dict);
        ctx.metric        = root.get<std::string>("metric");
        ctx.metric_ptr    = ss::metric_factory::get_inst()->make(ctx.metric);
        ctx.active_player = root.get<size_t>("active_player");
        ctx.width         = root.get<size_t>("width");
        ctx.height        = root.get<size_t>("height");
        ctx.state         = static_cast<game_state>(root.get<int>("state"));
        ctx.skips         = root.get<size_t>("skips");
        ctx.is_rotated    = root.get<bool>("is_rotated");
        
        ctx.board         = ss::board(ctx.width, ctx.height);

        size_t y=0;
        for( auto const& c : root.get_child("board") ){
                //std::string l{ c.second.get<std::string>("line") };
                std::string l{ c.second.data() };
                for( size_t x=0;x!=l.size();++x){
                        ctx.board(x,y) = l[x];
                }
                ++y;
        }

        for( auto const& p : root.get_child("players")){
                ctx.players.emplace_back();
                ctx.players.back().backend = p.second.get<std::string>("backend");
                ctx.players.back().rack    = p.second.get<std::string>("rack");
                ctx.players.back().vp      = vplayer_factory::get_inst()->make( ctx.players.back().backend );
                auto opt{ p.second.get_child_optional("scores") };
                if( opt ){
                        for( auto const& s : *opt){
                             ctx.players.back().score.emplace_back(
                                     s.second.get<size_t>("value"),
                                     s.second.get<std::string>("word"));
                        }
                }
        }
        for( auto const& p : root.get_child("moves")){
                ctx.moves.push_back(p.second.data());
        }
        for( auto const& p : root.get_child("logs")){
                ctx.log.push_back(p.second.data());
        }
        for( auto const& p : root.get_child("debug")){
                ctx.debug.push_back(p.second.data());
        }

}
void game_context_io::render(game_context const& ctx, std::ostream& ostr)const{
        using std::get;
        ostr << "          SCRABBLE                                                                                  \n";
        ostr << "\n";
        ostr << std::string(4, ' ');
        for(size_t i=0;i!=ctx.width;++i)
                ostr << boost::lexical_cast<std::string>(i%10);
        ostr << "\n";
        auto sv{ ctx.board.to_string_vec() };
        int i{0};
        std::string top{ std::string(4,' ') + "+" + std::string(ctx.width,'-') + "+"};

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
        ostr << "        |" << ctx.get_active()->rack << "|\n"
             << "\n";
        for(auto const& p : ctx.players )
                ostr << std::setw(22) << std::internal << p.backend << "|";
        ostr << "\n";
        ostr << std::string(22 * ctx.players.size(),'-') << "\n";
        std::vector<unsigned> sigma( ctx.players.size(), 0 );
        for( size_t i=0;;++i){
                bool end{false};
                for( size_t j=0;j!=ctx.players.size();++j){
                        auto const& p{ ctx.players[j] };
                        if( ! ( i < p.score.size() )) {
                                end = true;
                                ostr << std::setw(10) << "" << std::setw(0) << "|"
                                     << std::setw(5)  << "" << std::setw(0) << "|"
                                     << std::setw(5)  << "" << std::setw(0) << "|";
                        } else {
                                sigma[j] += get<0>(p.score[i]);
                                ostr << std::setw(10) << std::left << get<1>(p.score[i]) << std::setw(0) << "|"
                                     << std::setw(5)  << get<0>(p.score[i]) << std::setw(0) << "|"
                                     << std::setw(5)  << sigma[j]   << std::setw(0) << "|";
                        }
                }
                ostr << "\n";
                if( end ) break;
        }

        /////////////////////////////////////////////////////////////////////////
        // logs
        /////////////////////////////////////////////////////////////////////////

        ostr << "\nmoves:\n    ";
        boost::copy( ctx.moves, std::ostream_iterator<std::string>(ostr, "\n    "));
        ostr << "\nlogs:\n    ";
        boost::copy( ctx.log, std::ostream_iterator<std::string>(ostr, "\n    "));
        ostr << "\ndebug:\n    ";
        boost::copy( ctx.debug, std::ostream_iterator<std::string>(ostr, "\n    "));
}

void game_context_io::write_all(game_context const& ctx)const{
        std::ofstream of("scrabble.json");
        std::ofstream scof(ctx.scratch);
        write(ctx, of);
        render(ctx, scof);
        render_better(ctx, std::cout);
}


#include "text_composer.h"

namespace{
namespace detail{
        struct board_view : tc::decl{
                explicit board_view(game_context const& ctx):ctx_(&ctx){}
                size_t x_len()const override{ return tc::dynamic; }
                size_t y_len()const override{ return tc::dynamic; }
                std::unique_ptr<tc::text_object> to_object()const override{
                        std::stringstream ostr;
                        ostr << ' ';
                        for(size_t i=0;i!=ctx_->width;++i)
                                ostr << boost::lexical_cast<std::string>(i%10);
                        ostr << "\n";
                        auto sv{ ctx_->board.to_string_vec() };
                        int i{0};
                        std::string top{ "+" + std::string(ctx_->width,'-') + "+"};

                        ostr << top << "\n";
                        for( auto const& line : sv ){
                                ostr  << (i%10) << "|" << line << "|\n";
                                ++i;
                        }
                        ostr << top << "\n";
                        ostr << "\n";
                        return tc::text_object::from_string(ostr.str());
                }
        private:
                game_context const* ctx_;
        };
        
        struct score_view : tc::decl{
                explicit score_view(game_context const& ctx):ctx_(&ctx){}
                size_t x_len()const override{ return tc::dynamic; }
                size_t y_len()const override{ return tc::dynamic; }
                std::unique_ptr<tc::text_object> to_object()const override{
                        using std::get;
                        std::stringstream ostr;
                        for(auto const& p : ctx_->players )
                                ostr << std::setw(22) << std::internal << p.backend << "|";
                        ostr << "\n";
                        ostr << std::string(22 * ctx_->players.size(),'-') << "\n";
                        std::vector<unsigned> sigma( ctx_->players.size(), 0 );
                        for( size_t i=0;;++i){
                                bool end{false};
                                for( size_t j=0;j!=ctx_->players.size();++j){
                                        auto const& p{ ctx_->players[j] };
                                        if( ! ( i < p.score.size() )) {
                                                end = true;
                                                ostr << std::setw(10) << "" << std::setw(0) << "|"
                                                     << std::setw(5)  << "" << std::setw(0) << "|"
                                                     << std::setw(5)  << "" << std::setw(0) << "|";
                                        } else {
                                                sigma[j] += get<0>(p.score[i]);
                                                ostr << std::setw(10) << std::left << get<1>(p.score[i]) << std::setw(0) << "|"
                                                     << std::setw(5)  << get<0>(p.score[i]) << std::setw(0) << "|"
                                                     << std::setw(5)  << sigma[j]   << std::setw(0) << "|";
                                        }
                                }
                                ostr << "\n";
                                if( end ) break;
                        }
                        return tc::text_object::from_string(ostr.str());
                }
        private:
                game_context const* ctx_;
        };
} // detail
} // anon

void game_context_io::render_better(game_context const& ctx, std::ostream& ostr)const{
        auto title = std::make_shared<tc::placeholder>("title", tc::dynamic, 1);
        auto rack  = std::make_shared<tc::placeholder>("rack", tc::dynamic, 1);
        auto board = std::make_shared<tc::placeholder>("board", 20,20);
        auto score = std::make_shared<tc::placeholder>("score", 20,20);

        auto root = std::make_shared<tc::above_below_composite>();
        root->push(title);
        auto first { std::make_shared<tc::above_below_composite>() };
        first->push(board);
        first->push(rack);
        auto second { std::make_shared<tc::side_by_side_composite>() };
        second->push(first);
        second->push(score);

        root->push(second);

        title->set( std::make_shared<tc::text>("          SCRABBLE"));
        rack ->set( std::make_shared<tc::text>("        |" + ctx.get_active()->rack + "|" ));
        board->set( std::make_shared<detail::board_view>(ctx));
        score->set( std::make_shared<detail::score_view>(ctx));

        auto obj{ root->to_object() };
        
        obj->display(ostr);

}



















