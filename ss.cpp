#include "ss.h"
#include "ss_solver.h"


namespace ss{

        inline void read_board_from_string(board& b, std::string const& str){
                if( b.x_len() * b.y_len() != str.size() ){
                        std::stringstream msg;
                        msg << "parsing string as board but wrong size x_len * y_len = " 
                                << b.x_len() * b.y_len() << ", str.size = " << str.size();
                        BOOST_THROW_EXCEPTION(std::domain_error(msg.str()));
                }
                b.fill('\0');
                auto iter = str.begin();
                //for( size_t y = b.y_len();y!=0;){
                //      --y;
                for(size_t y=0;y!=b.y_len();++y){
                        for(size_t x=0;x!=b.x_len();++x){
                                if( *iter == ' '){
                                } else if( std::isalpha( *iter ) ){
                                        b(x,y) = std::toupper(*iter);
                                } else{
                                        std::stringstream msg;
                                        msg << "unknown char " << (int)(*iter);
                                        BOOST_THROW_EXCEPTION(std::domain_error(msg.str()));
                                }
                                ++iter;
                        }
                }
        }

	namespace tests{
		void render_test(){
			auto board = board_factory::get_inst()->make("plain");
			auto sboard = score_board_factory::get_inst()->make("plain");

			auto r = renderer_factory::get_inst()->make("cout_renderer");

                        #if 0
                        board->operator()(3,7) = 'J';
                        board->operator()(4,7) = 'O';
                        board->operator()(5,7) = 'K';
                        board->operator()(6,7) = 'E';
                        board->operator()(7,7) = 'R';
                        #endif

			r->render(*board);

                        auto strat = strategy_factory::get_inst()
                                ->make("brute_force");

                        //std::vector<tile_t> aux = {'T', 'H', 'A', 'P', 'A', 'U', 'A'};
                        //rack rck(aux);
                        rack rck("RETAINS");
                        //auto move = strat->solve( *board, rck, *sboard);
                        //std::cout << player_move_to_string(move) << "\n";

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
                           /*6*/"               "
                           /*7*/"       W       "
                           /*8*/"   HELLO       "
                           /*9*/"       R       "
                          /*10*/"       L       "
                          /*11*/"       D       "
                          /*12*/"               "
                          /*13*/"               "
                          /*14*/"               "
                          /*15*/"               "
                        ;
                        read_board_from_string(*board, board_str);
                        r->render(*board);
                        
                        auto move = strat->solve( *board, rck, *sboard);
                        std::cout << player_move_to_string(move) << "\n";


		}
	}
}

int main(){
  ss::tests::render_test();
}

// vim: sw=8 ts=8
