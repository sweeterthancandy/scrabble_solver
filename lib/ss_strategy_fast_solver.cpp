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
                explicit fast_solver(std::shared_ptr<dictionary_t> dict):
                        dict_(std::move(dict))
                {}
                void solve_(board const& b, std::vector<std::string> const& lines, rack const& rck){
                        assert( ! lines.size() && "precondition failed");
                        using std::get;
                        size_t width = lines.front().size();
                        for(size_t i=0;i!=lines.size();++i){

                                std::string line = lines[i];

                                std::vector<std::tuple<int, int, std::string, std::string> > moves;

                                std::vector<int> sum;
                                enum{
                                        Ele_Idx,
                                        Ele_Start,
                                        Ele_Left,
                                        Ele_Right
                                };
                                int sigma = 0;

                                PRINT(line);

                                for(size_t j=0;j!=width;++j){
                                        if( line[j] != '\0')
                                                continue;
                                        // this is a possible move
                                        std::string left;
                                        std::string right;
                                        for(size_t k=i;k!=0;){
                                                --k;
                                                if( lines[k][j] == '\0'){
                                                        break;
                                                }
                                                left += lines[k][j];
                                        }
                                        for(size_t k=i+1;k < width;++k){
                                                if( lines[k][j] == '\0'){
                                                        break;
                                                }
                                                right += lines[k][j];
                                        }
                                        int is_start = ( j     != 0           && line[j-1] != '\0' ) ||
                                                       ( j + 1 != line.size() && line[j+1] != '\0' ) ||
                                                       left.size() || 
                                                       right.size();

                                        left = std::string(left.rbegin(),left.rend());
                                        moves.emplace_back( std::forward_as_tuple( j, is_start, std::move(left), std::move(right)));
                                        sum.emplace_back(sigma);
                                        sigma += is_start;
                                }
                                sum.emplace_back(sigma);
                                        
                                PRINT(moves.size());

                                std::vector<std::vector<int> > start_stack;
                                for(size_t n=1;n <= std::max/**/(rck.size(),3ul);++n){
                                        auto cpy{b};
                                        int count{0};
                                        for(auto&& _ : moves){
                                                cpy( get<Ele_Idx>(_), 0 ) = boost::lexical_cast<char>(count % 10 );
                                                cpy( get<Ele_Idx>(_), 1 ) = boost::lexical_cast<char>(sum[count] % 10);
                                                ++count;
                                        }
                                        std::vector<int> start;

                                        PRINT(n);
                                        PRINT(moves.size());
                                        for(size_t j=0;j + n <= moves.size(); ++j){
                                                auto diff = sum[j+n] - sum[j];
                                                cpy( get<Ele_Idx>(moves[j]), 2 ) = boost::lexical_cast<char>(diff % 10);
                                                if(diff){
                                                        start.push_back(j);
                                                        //for(size_t k=0;k!=n;++k){
                                                        size_t m = get<Ele_Idx>(moves[j]);
                                                        cpy( m, i) = 'X';
                                                        //}
                                                }
                                        }
                                        if( start.empty() )
                                                break;
                                        dump(std::cout, start);
                                        cpy.dump();

                                        start_stack.emplace_back(std::move(start));
                                }

                                size_t n=1;
                                for( auto&& vec : start_stack ){
                                }

                        }
                }
                player_move solve(board const& b, rack const& rck){
                        auto hor = detail::string_cache_lines(array_orientation::horizontal, b);
                        auto vert = detail::string_cache_lines(array_orientation::vertical, b);

                        this->solve_(b, hor, rck);
                        board vb{b, array_orientation::vertical};
                        this->solve_(vb, vert, rck);

                        #if 0
                        std::cout << "BEGIN\n";
                        boost::copy(hor, std::ostream_iterator<std::string>(std::cout, "\n"));
                        std::cout << "END\n";
                        std::cout << "BEGIN\n";
                        boost::copy(vert, std::ostream_iterator<std::string>(std::cout, "\n"));
                        std::cout << "END\n";
                        #endif

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
