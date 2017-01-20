#include "ss.h"
#include "ss_board.h"
#include "ss_driver.h"
#include "ss_dict.h"
#include "ss_util.h"

#include <fstream>
#include <functional>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
/*
 * algorithm:
 *
 *      1 - starting points X
 *
 *      find the set X = {x_0,x_1,...x_n} for each every move M = {m_0, m_1,..m_m}
 *      \in P such that,
 *              \forall M in P \ldot M \union \X \ne \nullset.
 *      that is fine the smallest set X so that every possible move M \in P
 *      has a tile from X.
 *
 *      2 - cache possible moves
 *      
 *      find the set T of all templated moves, so that, for any set t \in T, 
 *      there exits an x in X such that t = x. Where t \in T is of the form
 *              t = {t_0,t_1,t_2}
 *
 *
 *              
 *
 *
 */

#include <boost/variant.hpp>


// this represents the possible

namespace{

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

        struct placeholder_t{
                friend std::ostream& operator<<(std::ostream& ostr, placeholder_t const&){
                        return ostr << "_";
                }
                void dump(){
                        std::cout << "placeholder\n";
                }
        };
        struct constant_t{
                explicit constant_t(char c):char_(c){}
                void dump(){
                        std::cout << "constant{" << char_ << "}\n";
                }
                friend std::ostream& operator<<(std::ostream& ostr, constant_t const& arg){
                        return ostr << "const{" << arg.char_ << "}";
                }
                char char_;
        };
        struct perp_t{
                explicit perp_t(std::string left, std::string right):
                        left_(std::move(left)),
                        right_(std::move(right))
                {}
                void dump(){
                        std::cout << "perp{" << left_ << "_" << right_ << "}\n";
                }
                friend std::ostream& operator<<(std::ostream& ostr, perp_t const& arg){
                        return ostr << "perp";
                }
                std::string left_;
                std::string right_;
        };
        using meta_tile_t = boost::variant<
                placeholder_t,
                constant_t,
                perp_t
        >;

        struct move_template{
                explicit move_template(std::vector<meta_tile_t> decl):
                        decl_(std::move(decl))
                {}
                void dump(){
                        #if 1
                        std::cout << "{";
                        boost::for_each( decl_, [](auto&& _){
                                boost::apply_visitor([](auto&& tile){
                                        std::cout << tile << ",";
                                        //tile.dump();
                                }, _);
                        });
                        std::cout << "}\n";
                        #endif
                        namespace ba = boost::accumulators;
                        using acc_t = ba::accumulator_set<size_t, ba::features<ba::tag::max>>;
                        acc_t left, right;
                        left(0);
                        right(0);
                        boost::for_each( decl_, [&](auto&& _){
                                if( perp_t* p = boost::get<perp_t>(&_) ){
                                        left( p->left_.size() );
                                        right( p->right_.size() );
                                }
                        });

                        struct printer : boost::static_visitor<>{

                                explicit printer(size_t offset, board& b)
                                        :offset_(offset), b_(&b)
                                {}

                                void operator()(placeholder_t const&){
                                        (*b_)(idx_, offset_) = '_';
                                        ++idx_;
                                }
                                void operator()(perp_t const& arg){
                                        (*b_)(idx_, offset_) = '_';
                                        for(size_t j=0; j!= arg.left_.size();++j){
                                                (*b_)(idx_, j + offset_ - arg.left_.size()) = arg.left_[j];
                                        }
                                        for(size_t j=0; j!= arg.right_.size();++j){
                                                (*b_)(idx_, j + offset_ + 1) = arg.right_[j];
                                        }
                                        ++idx_;
                                }
                                void operator()(constant_t const& arg){
                                        (*b_)(idx_, offset_) = arg.char_;
                                        ++idx_;
                                }
                        private:
                                board* b_;
                                size_t offset_;
                                size_t idx_ = 0;
                        };


                        auto offset = ba::max(left);
                        board aux( decl_.size(), ba::max(left) + ba::max(right) + 1);
                        printer p{offset, aux};
                        boost::for_each( decl_, boost::apply_visitor(p) );
                        aux.dump();
                }
                void solve(rack const& start_rck){
                        /*
                         * Algorithm
                         *
                         * first starting point x \in [0,n], try all combinations
                         * of words for word in the tiles [y,x]
                         *
                         */

                        //std::vector<size_t> start_points;
                        //for(size_t x{0};x!=decl_.size();++x){
                                //std::vector< std::tuple<std::string, rack> > stack;
                                //stack.emplace_back( std::forward_as_tuple( "", start_rck) );

                                //for(; stack.size(); ){
                                        //std::string tiles;
                                        //rack rck;
                                        //std::tie(tiles,rck) = std::move(stack.back());
                                        //stack.pop_back();


                                //}
                        //}

                }
        private:
                std::vector<meta_tile_t> decl_;
        };
        struct template_maker{
                void constant(char c){
                        decl_.emplace_back(constant_t{c});
                }
                void placeholder(){
                        decl_.emplace_back(placeholder_t{});
                }
                void perp(std::string left, std::string right){
                        decl_.emplace_back(perp_t{std::move(left), std::move(right)});
                }
                auto make(){
                        return move_template{std::move(decl_)};
                        // XXX maybe clear at the end
                }
        private:
                std::vector<meta_tile_t> decl_;
        };

        struct fast_solver : strategy{
                explicit fast_solver(std::shared_ptr<dictionary_t> dict):
                        dict_(std::move(dict))
                {}
                void solve_(std::vector<std::string> const& lines, rack const& rck){
                        assert( ! lines.size() && "precondition failed");
                        size_t width = lines.front().size();
                        for(size_t i=0;i!=lines.size();++i){
                                template_maker maker;

                                for(size_t j=0;j!=width;++j){
                                        if( lines[i][j] != '\0'){
                                                maker.constant(lines[i][j]);
                                        } else{
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
                                                if( left.size() + right.size() == 0){
                                                        maker.placeholder();
                                                } else{
                                                        left = std::string(left.rbegin(),left.rend());
                                                        PRINT(left);
                                                        PRINT(right);
                                                        maker.perp(left,right);
                                                }
                                        }
                                }
                                auto result = maker.make();
                                result.dump();
                        }
                }
                player_move solve(board const& b, rack const& rck){
                        auto hor = detail::string_cache_lines(array_orientation::horizontal, b);
                        auto vert = detail::string_cache_lines(array_orientation::verital, b);

                        this->solve_(hor, rck);

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
