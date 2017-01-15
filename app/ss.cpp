#include "ss.h"
#include "ss_util.h"

namespace ss{


	namespace tests{
		void render_test(){
			board board(15,15,' ');

			auto r = renderer_factory::get_inst()->make("cout_renderer");

                        auto strat = strategy_factory::get_inst()
                                ->make("fast_solver");


                        std::string board_proto = 
                        //       123456789012345
                           /*1*/"               "
                           /*2*/"               "
                           /*3*/"               "
                           /*4*/"               "
                           /*5*/"               "
                           /*6*/"               "
                           /*7*/"               "
                           /*8*/"               "
                           /*9*/"               "
                          /*10*/"               "
                          /*11*/"               "
                          /*12*/"               "
                          /*13*/"               "
                          /*14*/"               "
                          /*15*/"               "
                        ;
                        std::string board_str = 
                        //       123456789012345
                           /*1*/"               "
                           /*2*/"               "
                           /*3*/"               "
                           /*4*/"               "
                           /*5*/"               "
                           /*6*/"        COMP   "
                           /*7*/"        L      "
                           /*8*/"       PATHS   "
                           /*9*/"        M  E   "
                          /*10*/"         BELIE "
                          /*11*/"           L   "
                          /*12*/"           E   "
                          /*13*/"           R   "
                          /*14*/"               "
                          /*15*/"               "
                        ;

                        std::string rck_str = "AUEVNWA";
                        rack rck(rck_str);
                        read_board_from_string(board, board_str);
                        r->render(board);
                        std::cout << "Solving for |" << rck_str << "|\n";

                        auto dict = dictionary_factory::get_inst()->make("regular");
                        auto bad_words = do_validate_board(*dict, board);
                        if( bad_words.size()){
                                std::cout << "board not valid\n";
                                boost::for_each(bad_words, [](auto&& _){
                                        std::cout << "    " << _ << "\n";
                                });
                        } else {
                                auto move = strat->solve( board, rck);
                        }



		}
	}
}

int main(){
  ss::tests::render_test();
}

// vim: sw=8 ts=8
