#ifndef TEXT_COMPOSER_H
#define TEXT_COMPOSER_H

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

                void push_back(std::string line){ vec_.push_back(line); }
                void emplace_back(){ vec_.emplace_back(); }

                text_object make_static_view(size_t x_offset,
                                             size_t y_offset,
                                             size_t x_size,
                                             size_t y_size)
                {
                        text_object result;
                        for(size_t y{y_offset}; y - y_offset < y_size; ++y){
                                result.push_back( vec_[y].substr(x_offset, x_size) );
                        }
                        return std::move(result);
                }
                template<class T>
                text_object& operator<<(T const& val){
                        vec_.back() += boost::lexical_cast<std::string>(val);
                        return *this;
                }
                void display(std::ostream& ostr){
                        for( auto const& l : vec_){
                                ostr << l << "\n";
                        }
                }
                static std::unique_ptr<text_object> from_string(std::string const& str){
                        auto obj{ std::make_unique<text_object>() };
                        std::stringstream sstr;
                        sstr << str;
                        for(;;){
                                std::string line;
                                std::getline(sstr, line);
                                if( line.empty() && sstr.eof())
                                        break;
                                obj->push_back(std::move(line));
                                if( sstr.eof() )
                                        break;
                        }
                        return std::move(obj);
                }
        private:
                std::vector<std::string> vec_;
        };

        struct decl{
                virtual ~decl()=default;
                virtual size_t x_len()const=0;
                virtual size_t y_len()const=0;
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
        private:
                std::string str_;
        };

        struct placeholder : decl{
                explicit placeholder(std::string name, size_t width, size_t height)
                        :name_(std::move(name))
                        ,width_(width)
                        ,height_(height)
                {}
                size_t x_len()const override{ 
                        check_();
                        return handle_->x_len();
                }
                size_t y_len()const override{
                        check_();
                        return handle_->y_len();
                }
                std::unique_ptr<text_object> to_object()const override{
                        check_();
                        return handle_->to_object();
                }
                void set(text_handle h){ handle_ = h; }
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


        #if 0
        struct text_object_view : text_item{
                size_t x()const override{
                        return x_;
                }
                size_t y()const override{
                        return y_;
                }
                size_t x_len()const override{
                        return obj_.x_len();
                }
                size_t y_len()const override{
                        return obj_.y_len();
                }
                std::unique_ptr<text_object> to_object()const override{
                        auto ret{ std::make_unique<text_object>(obj_) };
                        return std::move(ret);
                }
        private:
                size_t x_;
                size_t y_;
                text_object obj_;
        };
        #endif

        struct above_below_composite : decl{
                size_t x_len()const override{
                        size_t ret{0};
                        for( auto const& h : vec_ )
                                ret = std::max( ret, h->x_len() );
                        return ret;
                }
                size_t y_len()const override{
                        size_t ret{0};
                        for( auto const& h : vec_ )
                                ret += h->y_len();
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
        private:
                std::vector<text_handle> vec_;
        };


}

#endif // TEXT_COMPOSER_H 
