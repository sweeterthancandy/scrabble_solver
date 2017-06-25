#include "driver_sub_command.h"
#include "game_context.h"

namespace{

struct rotate : sub_command{
        virtual int run(game_context& ctx, std::vector<std::string> const& args){
                std::ifstream ifs("scrabble.json");
                game_context_io{}.read(ctx, ifs);
                ifs.close();

                if( ctx.is_rotated ){
                        ctx.board.rotate_90();
                        ctx.board.rotate_90();
                        ctx.board.rotate_90();
                } else {
                        ctx.board.rotate_90();
                }
                ctx.is_rotated = ! ctx.is_rotated;

                game_context_io{}.write_all(ctx);

                return EXIT_SUCCESS;
        }
};

int rotate_reg = (sub_command_factory::get()->register_("rotate", [](){ return std::make_shared<rotate>(); }), 0);

} // anon
