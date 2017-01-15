#include "ss_renderer.h"
#include "ss_board.h"

#include <string>
#include <memory>

namespace {
        using namespace ss;

        struct stream_renderer : renderer{
                explicit stream_renderer(std::ostream& ostr):ostr_(&ostr){}
                void render(board const& board){
                        *ostr_ << std::string(board.x_len() * 3 + 2, '-') << "\n";
                        for(size_t y=0;y!=board.y_len();++y){
                                *ostr_ << "|";
                                for(size_t x=0;x!=board.x_len();++x){
                                        auto d = board(x,y);

                                        switch(d){
                                        case '\0':
                                                *ostr_ << "   ";
                                                break;
                                        default:
                                                if( std::isgraph(d)){
                                                        *ostr_ << ' ' << d << ' ';
                                                }else{
                                                        *ostr_ << ' ' << '?' << ' ';
                                                }
                                        }
                                }
                                *ostr_ << "|\n";
                        }
                        *ostr_ << std::string(board.x_len() * 3 + 2, '-') << "\n";
                        *ostr_ << std::flush;
                }
                std::shared_ptr<renderer> clone(){
                        return std::make_shared<stream_renderer>(*ostr_);
                }
        private:
                std::ostream* ostr_;
        };

        int cout_render = (
               renderer_factory::get_inst()->register_(
                       "cout_renderer",
                      [](){ return std::make_unique<stream_renderer>(std::cout); }
                      ),0);
}
