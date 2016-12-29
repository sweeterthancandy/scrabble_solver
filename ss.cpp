#include "ss.h"
#include "ss_solver.h"

namespace ss{
	namespace tests{
		void render_test(){
			auto board = board_factory::get_inst()->make("plain");
			auto sboard = score_board_factory::get_inst()->make("plain");

			auto r = renderer_factory::get_inst()->make("cout_renderer");

                        board->operator()(3,7) = 'g';
                        board->operator()(4,7) = 'e';
                        board->operator()(5,7) = 'r';
                        board->operator()(6,7) = 'r';
                        board->operator()(7,7) = 'y';

			r->render(*board, *sboard);

                        auto strat = strategy_factory::get_inst()
                                ->make("brute_force");

                        std::vector<tile_t> rack = {'t', 'h', 'a', 'p', 'a', 'u', 'a'};
                        auto move = strat->solve( *board, rack, *sboard);

                        std::cout << player_move_to_string(move) << "\n";


		}
	}
}

int main(){
  ss::tests::render_test();
}

// vim: sw=8 ts=8
