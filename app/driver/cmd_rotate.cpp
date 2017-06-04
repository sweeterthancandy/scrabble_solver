#include "sub_command.h"
#include "game_context.h"

namespace{

struct rotate : sub_command{
        virtual int run(game_context& ctx, std::vector<std::string> const& args){
                do{
                        std::ifstream ifs("scrabble.json");
                        ctx.read(ifs);
                }while(0);

                ss::board tmp{ ctx.board, ss::array_orientation::vertical };
                ctx.board = std::move(tmp);

                std::ofstream of("scrabble.json");
                ctx.write(of);
                std::ofstream scof(ctx.scratch);
                ctx.render(scof);

                return EXIT_SUCCESS;
        }
};

int rotate_reg = (sub_command_factory::get()->register_("rotate", [](){ return std::make_shared<rotate>(); }), 0);

} // anon
