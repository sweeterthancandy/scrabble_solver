#include <cstring>
#include <fstream>

#include "sub_command.h"
#include "game_context.h"


int driver_main(int argc, char** argv){
        if( argc < 2 || std::strcmp(argv[1], "--help") == 0 ){
                sub_command_factory::get()->print_help(argv[0]);
                return EXIT_SUCCESS;
        }
        game_context ctx;
        try{
                auto sub{ sub_command_factory::get()->make(argv[1]) };
                std::vector<std::string> args( argv, argv+argc);
                sub->run(ctx, args);
        } catch(std::exception const& e){
                std::cerr << e.what() << "\n";
                sub_command_factory::get()->print_help(argv[0]);
        }
        #if 1
        std::ofstream of("scrabble.json");
        ctx.write(of);
        std::ofstream scof(ctx.scratch);
        ctx.render(scof);
        #endif
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
