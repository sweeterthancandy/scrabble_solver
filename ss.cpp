#include "ss.h"
#include "ss_solver.h"

namespace ss{
	namespace tests{
		void render_test(){
			auto board = board_factory::get_inst()->make("plain");

			auto r = renderer_factory::get_inst()->make("cout_renderer");

                        board->tile_at(3,3) = 'g';
                        board->tile_at(4,3) = 'e';
                        board->tile_at(5,3) = 'r';
                        board->tile_at(6,3) = 'r';
                        board->tile_at(7,3) = 'y';

			r->render(*board);

                        auto strat = strategy_factory::get_inst()
                                ->make("brute_force");

                        std::vector<tile_t> rack = {'t', 'h', 'a', 'p', 'a', 'u', 'a'};
                        auto move = strat->solve( *board, rack);

                        std::cout << player_move_to_string(move) << "\n";


		}
	}
}

int main(){
  ss::tests::render_test();
}

// vim: sw=8 ts=8
