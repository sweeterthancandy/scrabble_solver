#ifndef TEXT_COMPOSER_H
#define TEXT_COMPOSER_H

#include <cassert>
#include "ss_print.h"

namespace tc{

        constexpr const size_t dynamic{static_cast<size_t>(-1)};

        /*
         * NOTE is default constructed with an empty line for 
         * sugar
         */
        struct text_object{
                text_object(){
                }
                auto begin()     { return vec_.begin(); }
                auto begin()const{ return vec_.begin(); }
                auto end()       { return vec_.end(); }
                auto end()const  { return vec_.end(); }

                auto size()const { return vec_.size(); }
                auto height()const { return size(); }
                auto width()const {
                        size_t w{0};
                        for(auto const& l : vec_ ){
                                w = std::max(l.size(), w);
                        }
                        return w;
                }

                void push_back(std::string line){ vec_.push_back(line); }
                void emplace_back(){ vec_.emplace_back(); }



                void pad_square(){
                        size_t w{0};
                        for(auto const& l : vec_ ){
                                w = std::max(l.size(), w);
                        }
                        pad(w);
                }
                void pad(size_t w){
                        for(auto& l : vec_ ){
                                if( l.size() < w ){
                                        size_t padding{ w - l.size()};
                                        l += std::string(padding, ' ');
                                }
                        }

                }

                /*
                   x_size == dynamic => all
                   y_size == dynamic => all
                 */
                text_object make_static_view(size_t x_offset,
                                             size_t y_offset,
                                             size_t x_size,
                                             size_t y_size)const
                {
                        size_t y_last{0};
                        if( y_last == dynamic ){
                                y_last = vec_.size();
                        } else {
                                y_last = std::min(y_offset + y_size, vec_.size());
                        }
                        text_object result;
                        for(size_t y{y_offset}; y < y_last; ++y){
                                if( vec_[y].size() == 0 ){
                                        result.emplace_back();
                                } else if( x_size == dynamic ||
                                           x_offset + x_size > vec_[y].size() ){
                                        result.push_back( vec_[y].substr(x_offset) );
                                } else {
                                        result.push_back( vec_[y].substr(x_offset, x_size) );
                                }
                        }
                        assert( result.width()  <= x_size && "post condition failed");
                        assert( result.height() <= y_size && "post condition failed");

                        #if 0
                        std::cout << "BEGIN\n";
                        display(std::cout);
                        PRINT_SEQ((x_offset)(y_offset)(x_size)(y_size)(y_last));
                        result.display(std::cout);
                        std::cout << "END\n";
                        #endif

                        return std::move(result);
                }
                template<class T>
                text_object& operator<<(T const& val){
                        vec_.back() += boost::lexical_cast<std::string>(val);
                        return *this;
                }
                void display(std::ostream& ostr)const{
                        for( auto const& l : vec_){
                                ostr << l << "\n";
                        }
                }
                static std::unique_ptr<text_object> from_string(std::string const& str){
                        std::stringstream sstr;
                        sstr << str;
                        return from_istream(sstr);
                }
                static std::unique_ptr<text_object> from_istream(std::istream& istr){
                        auto obj{ std::make_unique<text_object>() };
                        for(;;){
                                std::string line;
                                std::getline(istr, line);
                                if( line.empty() && istr.eof())
                                        break;
                                obj->push_back(std::move(line));
                                if( istr.eof() )
                                        break;
                        }
                        return std::move(obj);
                }
        private:
                std::vector<std::string> vec_;
        };

        struct decl{
                virtual ~decl()=default;
                virtual size_t x_len()const{ return tc::dynamic; }
                virtual size_t y_len()const{ return tc::dynamic; }
                virtual void accept(text_object const& obj)=0;
                virtual std::unique_ptr<text_object> to_object()const=0;
        };


        
        using text_handle = std::shared_ptr<decl>;

        struct text : decl{
                explicit text(std::string str):str_(std::move(str)){}
                size_t x_len()const override{ return str_.size(); }
                size_t y_len()const override{ return 1; }
                std::unique_ptr<text_object> to_object()const override{
                        auto obj{ std::make_unique<text_object>() };
                        obj->emplace_back();
                        *obj << str_;
                        return std::move(obj);
                }
                void accept(text_object const& obj)override{
                        std::cout << "text got\n";
                        obj.display(std::cout);
                }
        private:
                std::string str_;
        };
        
        struct text_object_view : decl{
                explicit text_object_view(text_object const& obj):obj_(obj){}
                size_t x_len()const override{ return obj_.width(); }
                size_t y_len()const override{ return obj_.height(); }
                std::unique_ptr<text_object> to_object()const override{
                        return std::make_unique<text_object>(obj_);
                }
                void accept(text_object const& obj)override{
                }
        private:
                text_object obj_;
        };

        struct placeholder : decl{
                explicit placeholder(std::string name, size_t width, size_t height)
                        :name_(std::move(name))
                        ,width_(width)
                        ,height_(height)
                {}
                size_t x_len()const override{ 
                        return width_;
                }
                size_t y_len()const override{
                        return height_;
                }
                std::unique_ptr<text_object> to_object()const override{
                        check_();
                        auto obj{ handle_->to_object() };
                        // now pad it
                        for(; height_ != dynamic && obj->size() < height_;)
                                obj->emplace_back();
                        if( width_ != dynamic )
                                obj->pad(width_);


                        if( obj->width() > width_ ||
                            obj->height() > height_ ){
                                std::cerr << "Warning, text object is too large\n";
                        }

                        return std::move(obj);
                }
                void set(text_handle h){ handle_ = h; }
                void accept(text_object const& obj)override{
                        if( handle_ )
                                handle_->accept(obj);
                }
        private:
                void check_()const{
                        if( ! handle_ )
                                throw std::domain_error("handle " + name_ + " not set");
                }
                std::string name_;
                size_t width_;
                size_t height_;
                text_handle handle_;
        };


        /*
             +------+
             +      |
             +------+
             +      |
             +------+
             +      |
             +------+
              
         */
        struct above_below_composite : decl{
                size_t x_len()const override{
                        size_t ret{0};
                        for( auto const& h : vec_ ){
                                if( h->x_len() == dynamic )
                                        return dynamic;
                                ret = std::max( ret, h->x_len() );
                        }
                        return ret;
                }
                size_t y_len()const override{
                        size_t ret{0};
                        for( auto const& h : vec_ ){
                                if( h->y_len() == dynamic )
                                        return dynamic;
                                ret += h->y_len();
                        }
                        return ret;
                }
                std::unique_ptr<text_object> to_object()const override{
                        auto ret{ std::make_unique<text_object>() };
                        for( auto const& item : vec_){
                                auto child{ item->to_object() };
                                for( auto const& l : *child ){
                                        ret->push_back(l);
                                }
                        }
                        return std::move(ret);
                }
                void push(text_handle handle){
                        vec_.push_back(std::move(handle));
                }
                void accept(text_object const& obj)override{
                        /*
                          
                           |
                           |y
                           |
                           |
                          \/

                         */
                        size_t offset{0};
                        for( auto const& item : vec_){
                                PRINT_SEQ((item->x_len())(item->y_len()));
                                if( item->y_len() == dynamic ){
                                        /*
                                            (offset,0)------+
                                             +              |
                                             +              |
                                             +              |
                                             +              |
                                             +-----------(end,end)
                                        */
                                        item->accept(obj.make_static_view(0, offset, dynamic, dynamic ));
                                        return;
                                }
                                /*
                                    (offset,0)--------------------+
                                     +                            |
                                     +                            |
                                     +                            |
                                     +                            |
                                     +--------------(end,offset+item->y_len())
                                     +                            |
                                     +                            |
                                     +         (rest)             |
                                     +                            |
                                     +----------------------(end,end)

                                */
                                item->accept(obj.make_static_view(0, offset, dynamic, item->y_len()));
                                offset += item->y_len();
                        }
                }
        private:
                std::vector<text_handle> vec_;
        };
        
        /*
         
             +------+------+------+
             +      |      |      |
             +------+------+------+
              
         */
        struct side_by_side_composite : decl{
                size_t y_len()const override{
                        size_t ret{0};
                        for( auto const& h : vec_ ){
                                if( h->y_len() == dynamic )
                                        return dynamic;
                                ret = std::max( ret, h->y_len() );
                        }
                        return ret;
                }
                size_t x_len()const override{
                        size_t ret{0};
                        for( auto const& h : vec_ ){
                                if( h->x_len() == dynamic )
                                        return dynamic;
                                ret += h->x_len();
                        }
                        return ret;
                }
                std::unique_ptr<text_object> to_object()const override{
                        auto ret{ std::make_unique<text_object>() };
                        for( auto const& item : vec_){
                                auto child{ item->to_object() };

                                for(; ret->size() < child->size() ;)
                                        ret->emplace_back();
                                ret->pad_square();


                                auto out{ ret->begin() };
                                for( auto const& l : *child ){
                                        *out += l;
                                        ++out;
                                }
                        }
                        return std::move(ret);
                }
                side_by_side_composite& push(text_handle handle){
                        vec_.push_back(std::move(handle));
                        return *this;
                }
                
                void accept(text_object const& obj)override{
                        /*
                          
                           ------------>
                              x

                         */
                        size_t offset{0};
                        for( auto const& item : vec_){
                                PRINT_SEQ((item->x_len())(item->y_len()));
                                if( item->x_len() == dynamic ){
                                        /*
                                            (0,offset)------+
                                             +              |
                                             +              |
                                             +              |
                                             +              |
                                             +-----------(end,end)
                                        */
                                        item->accept(obj.make_static_view(offset, 0, dynamic, dynamic ));
                                        return;
                                }
                                /*
                                    (0,offset)--------------------+-------------------+
                                     +                            |                   |
                                     +                            |      rest         |
                                     +                            |                   |
                                     +                            |                   |
                                     +-----------(end,offset+item->y_len())--------(end,end)
                                */
                                item->accept(obj.make_static_view(offset, 0, item->x_len(), dynamic));
                                offset += item->x_len();
                        }
                }
        private:
                std::vector<text_handle> vec_;
        };


}

#endif // TEXT_COMPOSER_H 
