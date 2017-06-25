#include "game_context_io.h"

#include <algorithm>
#include <functional>

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
        root.put("winner", ctx.winner);
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
        ctx.winner        = root.get<int>("winner");
        
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

void game_context_io::write_all(game_context const& ctx)const{
        std::ofstream of("scrabble.json");
        std::ofstream scof(ctx.scratch);
        write(ctx, of);
        render(ctx, scof);
}


#include "text_composer.h"

namespace{
namespace detail{
        struct board_view : tc::decl{
                board_view(game_context const& ctx,
                           std::function<void(ss::board const&)> const& on_parse )
                        : ctx_(&ctx)
                        , on_parse_(on_parse)
                {}
                std::unique_ptr<tc::text_object> to_object()const override{
                        std::stringstream ostr;
                        ostr << ' ';
                        for(size_t i=0;i!=ctx_->width;++i)
                                ostr << boost::lexical_cast<std::string>(i%10);
                        ostr << "\n";
                        auto sv{ ctx_->board.to_string_vec() };
                        int i{0};
                        std::string top{ " +" + std::string(ctx_->width,'-') + "+"};

                        ostr << top << "\n";
                        for( auto const& line : sv ){
                                ostr  << (i%10) << "|" << line << "|\n";
                                ++i;
                        }
                        ostr << top << "\n";
                        ostr << "\n";
                        return tc::text_object::from_string(ostr.str());
                }
                void accept(tc::text_object const& obj)override{
                        if( obj.height() < 3 + 15 )
                                return;

                        auto board_view{ obj.make_static_view( 2, 2, ctx_->width, ctx_->height) };

                        ss::board next(ctx_->width, ctx_->height);
                        size_t y{0};
                        for( auto const& l : board_view ){
                                for(size_t x{0};x!=ctx_->width;++x){
                                        next(x,y) = l[x];
                                }
                                ++y;
                        }
                        on_parse_(next);
                }
        private:
                game_context const* ctx_;
                std::function<void(ss::board const&)> on_parse_;
        };
        
        struct rack_view : tc::decl{
                rack_view(game_context const& ctx,
                           std::function<void(std::string const&)> const& on_parse )
                        : ctx_(&ctx)
                        , on_parse_(on_parse)
                {}
                std::unique_ptr<tc::text_object> to_object()const override{
                        auto const& r{ ctx_->get_active()->rack };

                        std::string header{"+" + std::string(r.size(), '-') + "+"};
                        std::string middle{"|" + r + "|"};

                        auto obj{ std::make_unique<tc::text_object>() };
                        obj->push_back(header);
                        obj->push_back(middle);
                        obj->push_back(header);
                        return std::move(obj);
                }
                void accept(tc::text_object const& obj)override{
                        if( obj.height() < 3 )
                                return;
                        std::string rack_s{ *std::next(obj.begin()) };
                        auto first{ rack_s.find_first_of('|') };
                        auto last{ rack_s.find_last_of('|') };
                        // should never happen
                        if( first == std::string::npos )
                                return;
                        std::string rs{ rack_s.substr(first+1, last - first -1 ) };
                        on_parse_(rs);
                }
        private:
                game_context const* ctx_;
                std::function<void(std::string const&)> on_parse_;
        };
        
        struct score_view : tc::decl{
                explicit score_view(game_context const& ctx):ctx_(&ctx){}
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
                void accept(tc::text_object const& obj)override{
                        #if 0
                        std::cout << "score view got\n";
                        obj.display(std::cout);
                        #endif
                }
        private:
                game_context const* ctx_;
        };

        struct hud_view : tc::decl{
                explicit hud_view(game_context const& ctx):ctx_(&ctx){
			m_[0] = 
R"_(
  _____  _                         __  __          ___           
 |  __ \| |                       /_ | \ \        / (_)          
 | |__) | | __ _ _   _  ___ _ __   | |  \ \  /\  / / _ _ __  ___ 
 |  ___/| |/ _` | | | |/ _ \ '__|  | |   \ \/  \/ / | | '_ \/ __|
 | |    | | (_| | |_| |  __/ |     | |    \  /\  /  | | | | \__ \
 |_|    |_|\__,_|\__, |\___|_|     |_|     \/  \/   |_|_| |_|___/
                  __/ |                                          
                 |___/                                           
)_";
			m_[1] = 
R"_(
  _____  _                         ___   __          ___           
 |  __ \| |                       |__ \  \ \        / (_)          
 | |__) | | __ _ _   _  ___ _ __     ) |  \ \  /\  / / _ _ __  ___ 
 |  ___/| |/ _` | | | |/ _ \ '__|   / /    \ \/  \/ / | | '_ \/ __|
 | |    | | (_| | |_| |  __/ |     / /_     \  /\  /  | | | | \__ \
 |_|    |_|\__,_|\__, |\___|_|    |____|     \/  \/   |_|_| |_|___/
                  __/ |                                            
                 |___/                                             
)_";
			draw_ = 
R"_(
  _____                     
 |  __ \                    
 | |  | |_ __ __ ___      __
 | |  | | '__/ _` \ \ /\ / /
 | |__| | | | (_| |\ V  V / 
 |_____/|_|  \__,_| \_/\_/  
)_";
                            
                            
			title_ = 
R"_(
   _____                _          _____                _     _     _      
  / ____|              | |        / ____|              | |   | |   | |     
 | |     __ _ _ __   __| |_   _  | (___   ___ _ __ __ _| |__ | |__ | | ___ 
 | |    / _` | '_ \ / _` | | | |  \___ \ / __| '__/ _` | '_ \| '_ \| |/ _ \
 | |___| (_| | | | | (_| | |_| |  ____) | (__| | | (_| | |_) | |_) | |  __/
  \_____\__,_|_| |_|\__,_|\__, | |_____/ \___|_|  \__,_|_.__/|_.__/|_|\___|
                           __/ |                                           
                          |___/                                            
)_";
                }
                std::unique_ptr<tc::text_object> to_object()const override{
			std::string str{title_};
                        switch(ctx_->state){
                        case State_Finished:
				if( m_.count( ctx_->winner ) ){
					str = m_.find(ctx_->winner)->second;
				}
                                break;
                        default:
                                break;
                        }
                        return tc::text_object::from_string(str);
                }
        private:
                game_context const* ctx_;
		std::string title_;
		std::string draw_;
                std::map<int, std::string> m_;
        };

} // detail
} // anon
game_context_io::game_context_io(){
	//                                                  width      | height
        title_ = std::make_shared<tc::placeholder>("title", 80         , 9     );
        board_ = std::make_shared<tc::placeholder>("board", 20         ,20     );
        rack_  = std::make_shared<tc::placeholder>("rack" , 20         , 3     );
        score_ = std::make_shared<tc::placeholder>("score", 50         , 50    );
        //hud_   = std::make_shared<tc::placeholder>("hud"  , 20         , 9    );

        auto left { std::make_shared<tc::above_below_composite>() };
        left->push(board_);
        left->push(rack_);
	//left->push(hud_);

        auto second { std::make_shared<tc::side_by_side_composite>() };
        second->push(left);
        second->push(score_);

        root_ = std::make_shared<tc::above_below_composite>();
        root_->push(title_);
        root_->push(second);
}
void game_context_io::render(game_context const& ctx, std::ostream& ostr)const{

        title_->set(std::make_shared<detail::hud_view>(ctx));
        rack_ ->set(std::make_shared<detail::rack_view>(ctx, [](auto const& r){}));
        board_->set(std::make_shared<detail::board_view>(ctx, [](auto const& b){}));
        score_->set(std::make_shared<detail::score_view>(ctx));

        auto obj{ root_->to_object() };
        
        obj->display(ostr);

}


boost::optional<game_context_io::parse_result_t> game_context_io::parse(game_context const& ctx, std::istream& istr)const{
        boost::optional<std::string> rack;
        boost::optional<ss::board> brd;

        rack_ ->set(std::make_shared<detail::rack_view>(ctx, [&](auto const& r){ 
                                                        rack = r; }));
        board_->set(std::make_shared<detail::board_view>(ctx, [&](auto const& b){
                                                         brd = b; }));
        auto meta{ tc::text_object::from_istream(istr) };
        root_->accept(*meta);
        
        if( rack && brd ){
                game_context_io::parse_result_t result{ brd.get(), rack.get() };
                return std::move(result);
        }
        std::cerr << "bad parse\n";
        return boost::none;
}

















