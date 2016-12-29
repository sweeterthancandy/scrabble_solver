#include "ss.h"
#include "ss_driver.h"

#include <fstream>

namespace ss{

        struct solve_context{
                std::vector<tile_t> task;
        };

        struct strategy{
                virtual ~strategy()=default;

                virtual player_move solve(board const& board, std::vector<tile_t> const& rack)=0;
                virtual std::shared_ptr<strategy> clone()=0;
        };

        using strategy_factory = generic_factory<strategy>;
        
        namespace auto_reg{

                std::shared_ptr<strategy> make_dict_solver(){
                        struct brute_force_strategy : strategy{
                                explicit brute_force_strategy(std::vector<std::string> const& dict):
                                        dict_(dict)
                                {}
                                player_move solve(board const& board, std::vector<tile_t> const& rack){
                                        return skip_go{};
                                }
                                std::shared_ptr<strategy> clone(){
                                        return std::make_shared<brute_force_strategy>(dict_);
                                }
                        private:
                                std::vector<std::string> dict_;
                        };
                        std::ifstream fstr("dictionary.txt");
                        std::vector<std::string> proto_dict;
                        for(;;){
                                std::string line;
                                std::getline(fstr, line);
                                if(line.size()){
                                        proto_dict.emplace_back(std::move(line));
                                }
                                if( fstr.eof() )
                                        break;
                        }
                        std::cout << proto_dict.size() << "\n";
                        return std::make_shared<brute_force_strategy>(proto_dict);
                }
                int reg_brute_force = ( strategy_factory::get_inst()
                                        ->register_( "brute_force", make_dict_solver()), 0 );
        }

}
