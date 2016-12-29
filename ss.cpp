#include "ss.h"
#include "ss_solver.h"

namespace ss{
	namespace tests{
		void render_test(){
			auto board = board_factory::get_inst()->make("plain");
			auto sboard = score_board_factory::get_inst()->make("plain");

			auto r = renderer_factory::get_inst()->make("cout_renderer");

                        board->operator()(3,7) = 'G';
                        board->operator()(4,7) = 'E';
                        board->operator()(5,7) = 'R';
                        board->operator()(6,7) = 'R';
                        board->operator()(7,7) = 'Y';

			r->render(*board, *sboard);

                        auto strat = strategy_factory::get_inst()
                                ->make("brute_force");

                        std::vector<tile_t> aux = {'T', 'H', 'A', 'P', 'A', 'U', 'A'};
                        rack rck(aux);
                        auto move = strat->solve( *board, rck, *sboard);

                        std::cout << player_move_to_string(move) << "\n";


		}
	}
}

int main(){
  ss::tests::render_test();
}

// vim: sw=8 ts=8
