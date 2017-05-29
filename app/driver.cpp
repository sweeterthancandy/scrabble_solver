
#include "ss.h"
#include "ss_util.h"
#include "ss_dict.h"
#include "ss_board_util.h"

#include <tuple>
#include <random>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>

namespace bpt = boost::property_tree;

struct player_t{
        std::string backend;
        std::string rack;

};

struct command_context{
        command_context()
                : gen{rd()}
        {}

        void write(std::ostream& ostr)const;
        void read(std::istream& ostr);
        void render(std::ostream& ostr)const;
        void apply_placements(std::vector<ss::word_placement> const& placements);

        std::random_device rd;
        std::mt19937 gen;

        size_t width;
        size_t height;
        ss::board board;

        std::string bag;
        std::vector<player_t> players;
        size_t active_player;
        std::string scratch = "scrabble.scratch";

        std::vector<std::string> log;
        std::vector<std::string> moves;

        std::string dict;
        std::shared_ptr<ss::dictionary_t> dict_ptr;

        std::string metric;
        std::shared_ptr<ss::metric> metric_ptr;

};
        
void command_context::write(std::ostream& ostr)const{
        bpt::ptree root;
        root.put("bag", bag);
        root.put("active_player", active_player);
        root.put("width", width);
        root.put("height", height);
        root.put("scratch", scratch);
        root.put("dict", dict);
        root.put("metric", metric);
        for( auto const& item : log )
                root.add("logs.log", item);
        for( auto const& item : moves )
                root.add("moves.move", item);
        for( auto const& p : players){
                bpt::ptree aux;
                aux.put("backend", p.backend);
                aux.put("rack", p.rack);
                root.add_child("players.player", aux);
        }
        for( auto const& l : board.to_string_vec()){
                root.add("board.line", l);
        }

        bpt::write_json(ostr, root);
}
void command_context::read(std::istream& ostr){
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
        for( auto const& p : root.get_child("players")){
                players.emplace_back();
                players.back().backend = p.second.get<std::string>("backend");
                players.back().rack    = p.second.get<std::string>("rack");
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
void command_context::render(std::ostream& ostr)const{
        ostr << "          SCRABBLE\n";
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
        ostr << "        |" << players[active_player].rack << "|\n";

        ostr << "\nlogs:\n    ";
        boost::copy( log, std::ostream_iterator<std::string>(ostr, "\n    "));
        ostr << "\nmoves:\n    ";
        boost::copy( moves, std::ostream_iterator<std::string>(ostr, "\n    "));
}
void command_context::apply_placements(std::vector<ss::word_placement> const& placements){
        auto score{ metric_ptr->calculate(placements) };
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
        ++active_player;
        active_player = active_player % players.size();

}

struct sub_command{
        using handle = std::shared_ptr<sub_command>;
        virtual ~sub_command(){}
        virtual int run(command_context& ctx, std::vector<std::string> const& args)=0;
};

struct sub_command_factory{

        using factory_t = std::function<sub_command::handle()>;

        static sub_command_factory* get(){
                static sub_command_factory* self{0};
                if( self == nullptr )
                        self = new sub_command_factory;
                return self;
        }
        void register_(std::string const& name, factory_t fac){
                fm_[name] = fac;
        }
        auto make(std::string const& name)const{
                if( ! fm_.count(name))
                        BOOST_THROW_EXCEPTION(std::domain_error("no subcommand " + name));
                return fm_.find(name)->second();
        }
        void print_help(std::string const& prog_name){
                std::cout << prog_name << " <sub-command>\n";
                for( auto const& p : fm_ )
                        std::cout << "    " << p.first << "\n";
        }
private:
        std::map<std::string, factory_t> fm_;
};

static std::map<char, size_t> tile_dist ={
        {'A', 9},
        {'B', 2},
        {'C', 2},
        {'D', 4},
        {'E',12},
        {'F', 2},
        {'G', 3},
        {'H', 2},
        {'I', 9},
        {'J', 1},
        {'K', 1},
        {'L', 4},
        {'M', 2},
        {'N', 6},
        {'O', 8},
        {'P', 2},
        {'Q', 1},
        {'R', 6},
        {'S', 4},
        {'T', 6},
        {'U', 4},
        {'V', 2},
        {'W', 2},
        {'X', 1},
        {'Y', 2},
        {'Z', 1},
};

struct init : sub_command{
        virtual int run(command_context& ctx, std::vector<std::string> const& args){
                for(auto const& p : tile_dist )
                        ctx.bag  += std::string(p.second, p.first);
                std::shuffle( ctx.bag.begin(), ctx.bag.end(), ctx.gen);

                ctx.width = 15;
                ctx.height = 15;
                ctx.board = ss::board(ctx.width, ctx.height);
                ctx.players.emplace_back();
                ctx.players.back().backend = "player";
                ctx.players.back().rack = ctx.bag.substr(ctx.bag.size()-7);
                ctx.bag.resize( ctx.bag.size() - 7 );
                ctx.players.emplace_back();
                ctx.players.back().backend = "ai";
                ctx.players.back().rack = ctx.bag.substr(ctx.bag.size()-7);
                ctx.bag.resize( ctx.bag.size() - 7 );
                ctx.active_player = 0;
                ctx.dict = "regular";
                ctx.metric = "scrabble_metric";
                
                ctx.log.push_back( "create new game");
                ctx.moves.push_back( "start");



                return EXIT_SUCCESS;
        }
};

int init_reg = (sub_command_factory::get()->register_("init", [](){ return std::make_shared<init>(); }), 0);


struct move : sub_command{
        virtual int run(command_context& ctx, std::vector<std::string> const& args){
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
                // get diff, and make sure that we havn't deleted any times,
                // ie
                //              ctx.board \setminus minus = {}
                //
                std::vector< std::pair< size_t, size_t> > diff;
                bool board_empty{true};
                for(size_t y=0;y!=ctx.height;++y){
                        for(size_t x=0;x!=ctx.height;++x){

                                if( ctx.board(x,y) == ' '){
                                        if( next(x,y) != ctx.board(x,y) ){
                                                diff.emplace_back(x,y);
                                        }
                                } else{
                                        board_empty = false;
                                        if( ctx.board(x,y) != next(x,y) ){
                                                // bad board
                                                // just rerender 
                                                std::stringstream sstr;
                                                sstr << "bad board, re-rendering <" << x << "," << y << ">";
                                                ctx.log.push_back(sstr.str());
                                                return EXIT_SUCCESS;
                                        }
                                }
                        }
                }

                // now validate move

                if( diff.empty() ){
                        ctx.log.push_back("nothing to do");
                        return EXIT_SUCCESS;
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
                        ctx.log.push_back("invalid move");
                        return EXIT_SUCCESS;
                }

                std::vector<ss::word_placement> placements;

                for( auto const& p : diff){
                        std::cout << "{" << p.first << "," << p.second << "}\n";
                }

                if( board_empty ){
                        if( boost::find_if( diff, [](auto const& p){ return p.first == 7 && p.second == 7; } ) == diff.end()){
                                ctx.log.push_back("first move must cross the middle <8,8>");
                                return EXIT_SUCCESS;
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

                        for(; y < ctx.height && next(x,y) != ' '; ++y)
                                word+=next(x,y);

                        if( y <= diff.back().second ){
                                ctx.log.push_back("tile placement not sequential");
                                return EXIT_SUCCESS;
                        }

                        // special case, can place one tile as in
                        //
                        //              HAT
                        //               T   <------ place this tile
                        //               
                        if( word.size() > 1){
                                placements.emplace_back( diff.front().first,
                                                         diff.front().second,
                                                         orientation,
                                                         word,
                                                         word);
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
                        

                        for(; x < ctx.height && next(x,y) != ' '; ++x){
                                word+=next(x,y);
                        }

                        if( x <= diff.back().first ){
                                ctx.log.push_back("tile placement not sequential");
                                return EXIT_SUCCESS;
                        }

                        if( word.size() > 1){
                                placements.emplace_back( diff.front().first,
                                                         diff.front().second,
                                                         orientation,
                                                         word,
                                                         word);
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
                                for(size_t x{d.first+1}; x < ctx.width; ++x){
                                        if( next(x,y) == ' ')
                                                break;
                                        right += next(x,y);
                                }
                                std::string left{rev_left.rbegin(), rev_left.rend()};
                                std::string word{ left + next(d.first, y ) + right };

                                PRINT_SEQ((left)(right)(word));

                                if( word.size() > 1){
                                        placements.emplace_back( d.first - left.size(),
                                                                 y,
                                                                 ss::array_orientation::horizontal,
                                                                 word,
                                                                 word);
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
                                for(size_t y{d.second+1}; y < ctx.width; ++y){
                                        if( next(x, y) == ' ')
                                                break;
                                        right += next(x, y);
                                }
                                std::string left{rev_left.rbegin(), rev_left.rend()};
                                std::string word{ left + next(x, d.second ) + right };

                                PRINT_SEQ((left)(right)(word));

                                if( word.size() > 1){
                                        placements.emplace_back( x,
                                                                 d.second - left.size(),
                                                                 ss::array_orientation::horizontal,
                                                                 word,
                                                                 word);
                                }
                        }
                }

                // XXX can't place a single tile on first go


                for( auto const& p : placements ){
                        if( ! ctx.dict_ptr->contains(p.get_word() ) ){
                                std::stringstream sstr;
                                sstr << "invalid word [" << p.get_word() << "]";
                                ctx.log.push_back(sstr.str());
                                return EXIT_SUCCESS;
                        }
                }

                ctx.apply_placements( placements );


                #if 0
                std::stringstream sstr;
                sstr << "player " << ctx.active_player << " placed " << placements.front().get_word() << " at <" << diff.front().first << "," << diff.front().second << ">";
                ctx.moves.emplace_back(sstr.str());
                        
                ctx.board = next;
                #endif

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
                                                std::cout << "yeild\n";
                                                all_placements.push_back( placements );
                                       });
                        PRINT(all_placements.size());
                        if( all_placements.size()){
                                boost::sort( all_placements, [&](auto const& l, auto const& r){
                                        return ctx.metric_ptr->calculate(l) < ctx.metric_ptr->calculate(r);
                                });

                                ctx.apply_placements( all_placements.back() );
                                #if 0
                                for( auto const& x : all_placements.back())
                                        x.dump();
                                #endif
                        }

                }



                return EXIT_SUCCESS;
        }
};

int move_reg = (sub_command_factory::get()->register_("move", [](){ return std::make_shared<move>(); }), 0);


int driver_main(int argc, char** argv){
        if( argc < 2 || std::strcmp(argv[1], "--help") == 0 ){
                sub_command_factory::get()->print_help(argv[0]);
                return EXIT_SUCCESS;
        }
        command_context ctx;
        try{
                auto sub{ sub_command_factory::get()->make(argv[1]) };
                std::vector<std::string> args( argv, argv+argc);
                sub->run(ctx, args);
        } catch(std::exception const& e){
                std::cerr << e.what() << "\n";
                sub_command_factory::get()->print_help(argv[0]);
                return EXIT_FAILURE;
        }
        #if 1
        std::ofstream of("scrabble.json");
        ctx.write(of);
        std::ofstream scof(ctx.scratch);
        ctx.render(scof);
        #endif
        ctx.write(std::cout);
        ctx.render(std::cout);
        return EXIT_SUCCESS;
}


int main(int argc, char** argv){
        const char* args[] = {
                "__main__",
                "init",
                0
        };
        //return driver_main(sizeof(args)/sizeof(*args)-1, args);
                
        return driver_main(argc, argv);
}
