
#include "ss.h"
#include "ss_util.h"
#include "ss_board_util.h"

#include <tuple>
#include <random>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace bpt = boost::property_tree;

struct player_t{
        std::string backend;
        std::string rack;
};

struct command_context{
        command_context()
                : gen{rd()}
        {}

        void write(std::ostream& ostr)const{
                bpt::ptree root;
                root.put("bag", bag);
                root.put("active_player", active_player);
                root.put("width", width);
                root.put("height", height);
                root.put("scratch", scratch);
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
        void read(std::istream& ostr){
                bpt::ptree root;
                bpt::read_json(ostr, root);
                bag = root.get<std::string>("bag");
                scratch = root.get<std::string>("scratch");
                active_player = root.get<size_t>("active_player");
                width = root.get<size_t>("width");
                height = root.get<size_t>("height");
                for( auto const& p : root.get_child("players")){
                        players.emplace_back();
                        players.back().backend = p.second.get<std::string>("backend");
                        players.back().rack    = p.second.get<std::string>("rack");
                }
                size_t y=0;
                board = ss::board(width, height);
                for( auto const& c : root.get_child("board") ){
                        //std::string l{ c.second.get<std::string>("line") };
                        std::string l{ c.first };
                        for( size_t x=0;x!=l.size();++x){
                                board(x,y) = l[x];
                        }
                        ++y;
                }
        }
        void render(std::ostream& ostr)const{
                ostr << "          SCRABBLE\n";
                ostr << "\n";
                auto sv{ board.to_string_vec() };
                std::string top{ std::string(4,' ') + "+" + std::string(width,'-') + "+"};

                ostr << top << "\n";
                for( auto const& line : sv ){
                        ostr << "    |" << line << "|\n";
                }
                ostr << top << "\n";
                ostr << "\n";
                ostr << "        |" << players[active_player].rack << "|\n";
        }

        std::random_device rd;
        std::mt19937 gen;

        std::string bag;
        std::vector<player_t> players;
        size_t active_player;
        std::string scratch = "scrabble.scratch";
        size_t width;
        size_t height;
        ss::board board;
};

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

                std::ofstream of("scrabble.json");
                ctx.write(of);
                std::ofstream scof(ctx.scratch);
                ctx.render(scof);

                return EXIT_SUCCESS;
        }
};

int init_reg = (sub_command_factory::get()->register_("init", [](){ return std::make_shared<init>(); }), 0);


struct move : sub_command{
        virtual int run(command_context& ctx, std::vector<std::string> const& args){
                std::ifstream ifs("scrabble.json");
                ctx.read(ifs);

                std::ifstream scr("scrabble.scratch");
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
                for(;;){
                        std::string line;
                        std::getline(scr, line);
                        lines.push_back(std::move(line));
                        if( scr.eof())
                                break;
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
                return sub->run(ctx, args);
        } catch(std::exception const& e){
                std::cerr << e.what() << "\n";
                sub_command_factory::get()->print_help(argv[0]);
                return EXIT_SUCCESS;
        }
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
