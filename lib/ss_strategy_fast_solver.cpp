#include "ss.h"
#include "ss_board.h"
#include "ss_driver.h"
#include "ss_dict.h"
#include "ss_util.h"

#include <fstream>
#include <functional>
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

        struct placeholder_t{};
        struct constant_t{
                explicit constant_t(char c):char_(c){}
                char char_;
        };
        struct perp_t{
                explicit perp_t(std::string left, std::string right):
                        left_(std::move(left)),
                        right_(std::move(right))
                {}
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
                        decl_.emplace_back(perpt_{std::move(left), std::move(right)});
                }
                auto make(){
                        // XXX maybe clear at the end
                        return move_template{std::move(decl_)};
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
                                                for(size_t k=j;k!=0;){
                                                        --k;
                                                        if( lines[i][k] == '\0'){
                                                                break;
                                                        }
                                                        left += lines[i][k];
                                                }
                                                for(size_t k=j+1;k < width;++k){
                                                        if( lines[i][k] == '\0'){
                                                                break;
                                                        }
                                                        right += lines[i][k];
                                                }
                                                if( left.size() + right.size() == 0){
                                                        maker.placeholder();
                                                } else{
                                                        maker.perp(left,right);
                                                }
                                        }
                                }
                                auto result = maker.make();
                        }
                }
                player_move solve(board const& b, rack const& rck){
                        auto hor = detail::string_cache_lines(b);
                        auto vert = detail::string_cache_lines(b);

                        this->solve_(candidates, hor);

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
