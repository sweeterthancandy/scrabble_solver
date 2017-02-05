


/*
 * I really need to be able to do some pretty printing for easy
 * debugging, and I think I can write a very nice text render
 * with little effort
 *
 *      - To be able to debug algorithms
 *      - Render actualy output for playing versus computer
 *
 * idea
 *      - compose the screen
 *      - have arbitary text boxes/lists
 *
 */

#include "picture_compiler.h"
#include "ss_board.h"

namespace ss{
        namespace io{
                struct board_renderer{
                        board_renderer(board const& b,
                                       array_orientation orientation = array_orientation::horizontal)
                                :b_(&b)
                                ,orientation_(orientation)
                        {}
                        template<class T>
                        void put_tag(std::string key, T const& value){
                                keys_.emplace_back(std::move(key));
                                values_.emplace_back(boost::lexical_cast<std::string>(value));
                        }
                        void display(){
                                using tag_reg0 = picture_compiler::tag_reg0;
                                using tag_reg1 = picture_compiler::tag_reg1;
                                using tag_reg2 = picture_compiler::tag_reg2;
                                using tag_reg3 = picture_compiler::tag_reg3;

                                std::stringstream sstr;
                                b_->dump(sstr);
                                std::vector<std::string> sv;
                                sv.emplace_back();
                                std::string aux(sstr.str());
                                boost::for_each( aux, [&sv](char c){
                                        switch(c){
                                        case '\n':
                                                sv.emplace_back();
                                                break;
                                        default:
                                                sv.back() += c;
                                        }
                                });

                                picture_compiler compiler;

                                #if 0
                                compiler.make_text_picture_from_sv_left<tag_reg0>(sv);
                                auto pic = compiler.compile<tag_reg0>();
                                #else
                                compiler.make_text_picture_from_sv_left<tag_reg0>(keys_);
                                compiler.make_framed<tag_reg0>();
                                compiler.make_text_picture_from_sv_left<tag_reg1>(values_);
                                compiler.make_framed<tag_reg1>();
                                compiler.make_side_by_side_center<tag_reg0, tag_reg1, tag_reg0>();

                                compiler.make_text_picture_from_sv_left<tag_reg1>(sv);
                                compiler.make_framed<tag_reg1>();
                                compiler.make_side_by_side_center<tag_reg0, tag_reg1, tag_reg0>();
                                
                                auto pic = compiler.compile< tag_reg0 >();
                                #endif
                                pic.display();
                        }
                private:
                        board const* b_;
                        array_orientation orientation_;
                        std::vector<std::string> keys_;
                        std::vector<std::string> values_;
                };
        }
}
