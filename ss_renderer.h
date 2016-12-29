namespace ss{

	struct renderer{
                virtual ~renderer()=default;
                virtual void render(board const& board, score_board const& sboard)=0;
                virtual std::shared_ptr<renderer> clone()=0;
	};


        using renderer_factory = generic_factory<renderer>;

        namespace auto_reg{
                std::shared_ptr<renderer> make_cout_renderer(){
                        struct stream_renderer : renderer{
                                explicit stream_renderer(std::ostream& ostr):ostr_(&ostr){}
                                void render(board const& board, score_board const& sboard){
                                        *ostr_ << std::string(board.x_len() * 3 + 2, '-') << "\n";
                                        for(size_t y= board.y_len(); y!= 0;){
                                                --y;
                                                *ostr_ << "|";
                                                for(size_t x=0;x!=board.x_len();++x){
                                                        auto d = board(x,y);
                                                        auto t = sboard(x,y);

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
                        return std::make_shared<stream_renderer>(std::cout);
                }
                int cout_render = (
                       renderer_factory::get_inst()->register_(
                               "cout_renderer",
                              make_cout_renderer()),0);
        }

}
