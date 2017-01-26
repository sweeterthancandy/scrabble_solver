#include "ss.h"
#include "ss_board.h"
#include "ss_driver.h"
#include "ss_dict.h"
#include "ss_util.h"

#include <iostream>
#include <fstream>
#include <functional>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>


// this represents the possible

namespace{
        using std::get;
        template<class T, class = std::__void_t< decltype( std::declval<T>().begin() ) > >
        inline std::ostream& dump(std::ostream& ostr, T const& con){
                ostr << "{";
                boost::for_each( con, [&](auto&& _){ ostr << _ << ", ";});
                ostr << "}\n";
                return ostr;
        }

        using namespace ss;

        namespace detail{

                std::vector<std::string> string_cache_lines(array_orientation orientation, board const& b){
                        std::vector<std::string> result;

                        for(size_t j=0;j!=b.y_len(orientation);++j){
                                result.emplace_back();
                                for(size_t i=0;i!=b.x_len(orientation);++i){
                                        result.back() += b(orientation, i,j);
                                }
                        }

                        return result;

                }
        }

        struct fast_solver : strategy{
                typedef std::tuple<
                        size_t, 
                        size_t,
                        std::string
                > candidate_move;
                enum{
                        Cand_X,
                        Cand_Y,
                        Cand_Word
                };

                explicit fast_solver(std::shared_ptr<dictionary_t> dict):
                        dict_(std::move(dict))
                {}
                template<class F>
                void solve_(board const& b, std::vector<std::string> const& board_lines, rack const& rck, F f){
                        assert( board_lines.size() && "precondition failed");
                        size_t width = board_lines.front().size();
                        auto dict = dictionary_factory::get_inst()->make("regular");
                        for(size_t i=0;i!=board_lines.size();++i){
                        //for(size_t i=4; i!=5; ++i){

                                std::cout << "\n\n";

                                std::string current_line = board_lines[i];

                                do{
                                        std::string aux;
                                        boost::for_each( current_line, [&aux](char c){ 
                                                switch(c){
                                                case '\0':
                                                       c = ' ';
                                                       break;
                                                }
                                                aux += c;
                                        });
                                        std::cout << "current_line = " << aux << "\n";
                                }while(0);


                                std::vector<std::tuple<int, int, std::string, std::string> > moves;

                                std::vector<int> sum;
                                enum{
                                        Ele_Idx,
                                        Ele_Start,
                                        Ele_Left,
                                        Ele_Right
                                };
                                int sigma = 0;

                                for(size_t j=0;j!=width;++j){
                                        if( current_line[j] != '\0')
                                                continue;
                                        // this is a possible move
                                        std::string left;
                                        std::string right;
                                        for(size_t k=i;k!=0;){
                                                --k;
                                                if( board_lines[k][j] == '\0'){
                                                        break;
                                                }
                                                left += board_lines[k][j];
                                        }
                                        for(size_t k=i+1;k < width;++k){
                                                if( board_lines[k][j] == '\0'){
                                                        break;
                                                }
                                                right += board_lines[k][j];
                                        }
                                        int is_start = ( j     != 0           && current_line[j-1] != '\0' ) ||
                                                       ( j + 1 != current_line.size() && current_line[j+1] != '\0' ) ||
                                                       left.size() || 
                                                       right.size();

                                        left = std::string(left.rbegin(),left.rend());
                                        moves.emplace_back( std::forward_as_tuple( j, is_start, std::move(left), std::move(right)));
                                        sum.emplace_back(sigma);
                                        sigma += is_start;
                                }
                                sum.emplace_back(sigma);
                                        
                                std::vector<std::vector<int> > start_stack;
                                for(size_t n=1;n <= std::max/**/(rck.size(),3ul);++n){

                                        std::cout << "XXX\n";
                                        //PRINT(n);
                                        //auto cpy{b};
                                        //int count{0};
                                        //for(auto&& _ : moves){
                                                //cpy( get<Ele_Idx>(_), 0 ) = boost::lexical_cast<char>(count % 10 );
                                                //cpy( get<Ele_Idx>(_), 1 ) = boost::lexical_cast<char>(sum[count] % 10);
                                                //++count;
                                        //}
                                        std::vector<int> start_vec;

                                        for(size_t j=0;j + n <= moves.size(); ++j){
                                                auto diff = sum[j+n] - sum[j];
                                                //cpy( get<Ele_Idx>(moves[j]), 2 ) = boost::lexical_cast<char>(diff % 10);
                                                if(diff){
                                                        start_vec.push_back(j);
                                                        //for(size_t k=0;k!=n;++k){
                                                        size_t m = get<Ele_Idx>(moves[j]);
                                                        //cpy( m, i) = 'X';
                                                        //}
                                                }
                                        }
                                        dump(std::cout, start_vec);
                                        if( start_vec.empty() )
                                                break;
                                        //dump(std::cout, start_vec);
                                        //cpy.dump();


                                        for( int start : start_vec ){

                                                auto const& start_move(moves[start]);
                                                std::string prefix;
                                                for( size_t j= get<Ele_Idx>(start_move); j != 0; ){
                                                        --j;
                                                        if( current_line[j] == '\0')
                                                                break;
                                                        prefix += current_line[j];
                                                }
                                                prefix = std::string(prefix.rbegin(), prefix.rend());


                                                std::vector<std::tuple<std::string, size_t, rack> > stack;
                                                stack.emplace_back( std::move(prefix), start, rck);
                                                enum{
                                                        Item_Word,
                                                        Item_MoveIdx,
                                                        Item_Rack
                                                };
                                                for(; stack.size();){
                                                        auto item = stack.back();
                                                        stack.pop_back();
                                                                

                                                        //PRINT_SEQ((current_move_suffix)(current_move_prefix));

                                                        auto current_idx {get<Item_MoveIdx>(item)};
                                                        if( current_idx - start == n ){
                                                                // terminal
                                                                auto word = get<Item_Word>(item);

                                                                bool ret = boost::binary_search( *dict, word );

                                                                PRINT_SEQ((ret)(i)(n)(start)(word));

                                                                if( ret ){
                                                                        f(start, i, word);
                                                                }
                                                                
                                                        } else{
                                                                // else, yeild all other possible states
                                                                
                                                                auto current_rack{get<Item_Rack>(item)};
                                                                auto const& current_move{moves.at(current_idx)};
                                                                std::string current_move_suffix{get<Ele_Left>(current_move)};
                                                                std::string current_move_prefix{get<Ele_Right>(current_move)};

                                                                if( current_idx + 1 < moves.size() ){

                                                                        auto cpy_start{get<Ele_Idx>(moves[current_idx]) +1},
                                                                             cpy_end  {get<Ele_Idx>(moves[current_idx +1])};
                                                                        //PRINT_SEQ((cpy_start)(cpy_end));
                                                                        for(;
                                                                            cpy_start!=cpy_end;
                                                                            ++cpy_start)
                                                                        {
                                                                                get<Item_Word>(item) += current_line[cpy_start];
                                                                        }
                                                                }

                                                                for( auto t : current_rack.make_tile_set() ){

                                                                        if( current_move_suffix.size() || current_move_prefix.size() ){
                                                                                auto perp_word{current_move_prefix};
                                                                                perp_word += t;
                                                                                perp_word += current_move_suffix;
                                                                                bool ret = boost::binary_search( *dict, perp_word );
                                                                                PRINT_SEQ((ret)(perp_word));
                                                                                if( ! ret ){
                                                                                        continue;
                                                                                }
                                                                        }

                                                                        stack.emplace_back(
                                                                                get<Item_Word>(item) + t,
                                                                                get<Item_MoveIdx>(item)+1,
                                                                                current_rack.clone_remove_tile(t));
                                                                }
                                                        }
                                                }
                                        }

                                        //if( n == 2 )
                                                //return;


                                        start_stack.emplace_back(std::move(start_vec));
                                }

                                //PRINT(start_stack.size());

                                size_t n=1;
                                for( auto&& vec : start_stack ){
                                }

                                //// break after first non trival loop
                                //if( start_stack.size() )
                                        //break;
                        }
                }
                player_move solve(board const& b, rack const& rck){
                        auto hor = detail::string_cache_lines(array_orientation::horizontal, b);
                        auto vert = detail::string_cache_lines(array_orientation::vertical, b);


                        std::vector<candidate_move> hor_cand;
                        this->solve_(b, hor, rck, [&hor_cand](size_t x, size_t y, std::string word){
                                hor_cand.emplace_back(x,y,std::move(word));
                        });
                        boost::sort( hor_cand, [](auto&& l, auto&& r){
                                return get<Cand_Word>(l).size() < get<Cand_Word>(r).size();
                        });
                        for( auto best : hor_cand ){
                                auto cpy{b};
                                cpy.dump();
                                auto x = get<Cand_X>(best);
                                auto y = get<Cand_Y>(best);
                                for( auto c : get<Cand_Word>(best)){
                                        #if 0
                                        for(;;++x){
                                                if( cpy(x,y) == '\0')
                                                        break;
                                        }
                                        #endif
                                        cpy(x,y) = c;
                                        ++x;
                                }
                                cpy.dump();
                                PRINT_SEQ((get<Cand_X>(best))(get<Cand_Y>(best))(get<Cand_Word>(best)));
                        }
                        //board vb{cpy, array_orientation::vertical};
                        //this->solve_(vb, vert, rck);

                        //std::cout << "BEGIN\n";
                        //boost::copy(hor, std::ostream_iterator<std::string>(std::cout, "\n"));
                        //std::cout << "END\n";
                        //std::cout << "BEGIN\n";
                        //boost::copy(vert, std::ostream_iterator<std::string>(std::cout, "\n"));
                        //std::cout << "END\n";

                        return skip_go{};
                }
                std::shared_ptr<strategy> clone(){
                        return std::make_shared<fast_solver>(dict_);
                }
        private:
                std::shared_ptr<dictionary_t> dict_;
        };
        
        
        int reg_brute_force = ( strategy_factory::get_inst() ->register_( "fast_solver", 
                                                                          [](){
                                                                                return std::make_unique<fast_solver>(
                                                                                        dictionary_factory::get_inst()->make("regular"));
                                                                          }
                                                                          ), 0 );

}
