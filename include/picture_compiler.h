#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <boost/exception/all.hpp>
#include <boost/mpl/char.hpp>
#include <boost/format.hpp>
#include <boost/fusion/container/map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/preprocessor.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/lexical_cast.hpp>

namespace picture_compiler_detail{


        enum class centering_policy{
                left   = 1 << 0, 
                top    = 1 << 1,
                right  = 1 << 2,
                bottom = 1 << 3,
                center = 1 << 4
        };


        struct picture{
                struct visitor;

                using handle = std::shared_ptr<picture>;

                virtual ~picture()=default;
                virtual void put(visitor& /*v*/)const=0;

               friend std::ostream& operator<<(std::ostream& ostr, const picture& pic);
        };

        struct picture::visitor{
                // way to decalr the number of lines BEFORE iterating
                virtual void attr_width(size_t /*n*/){}
                virtual void attr_height(size_t /*n*/){}
                virtual void put_line(const std::string& /*line*/){}
                virtual void begin(){}
                virtual void end(){}
        };
        
        struct debug_visitor : public picture::visitor{
                // way to decalr the number of lines BEFORE iterating
                void attr_width(size_t n){
                        std::cout << boost::format("%s(%s)\n") % __FUNCTION__ % n;
                }
                void attr_height(size_t n){
                        std::cout << boost::format("%s(%s)\n") % __FUNCTION__ % n;
                }
                void put_line(const std::string& line){
                        std::cout << boost::format("%s(%s)\n") % __FUNCTION__ % line;
                }
                void begin(){
                        std::cout << boost::format("%s()\n") % __FUNCTION__;
                }
                void end(){
                        std::cout << boost::format("%s()\n") % __FUNCTION__;
                }
        };
        
        struct cout_visitor : public picture::visitor{
                void put_line(const std::string& line)final{
                        std::cout << line << "\n";
                }
        };
        struct stream_visitor : public picture::visitor{
                explicit stream_visitor(std::ostream& ostr):ostr_(&ostr){}
                void put_line(const std::string& line)final{
                        (*ostr_) << line << "\n";
                }
        private:
                std::ostream* ostr_;
        };

        inline std::ostream& operator<<(std::ostream& ostr, const picture& pic){
                stream_visitor v(ostr);
                //debug_visitor v;
                pic.put( v );
                return ostr;
        }


        struct buffer_visitor : public picture::visitor{
                // way to decalr the number of lines BEFORE iterating
                void attr_width(size_t n){
                        width_ = n;
                }
                void attr_height(size_t n){
                        height_ = n;
                }
                void put_line(const std::string& line){
                        lines_.emplace_back( line );
                }
                size_t width()const{return width_;}
                size_t height()const{return height_;}
                std::vector<std::string> lines()const{return lines_;}
        private:
                size_t width_, height_;
                std::vector<std::string> lines_;
        };
        
        template<centering_policy Centering_Policy> 
        struct basic_text_picture
                : public picture
        {
                using vs_type = std::vector<std::string>;
                explicit basic_text_picture(vs_type vs):
                        vs_(std::move(vs))
                {
                        this->size_ = 0;
                        for( const auto& l : vs_ ){
                                using std::max;
                                this->size_ = std::max/**/(this->size_, l.size() ); 
                                using std::isgraph;
                                using std::isspace;
                                for( auto c : l ){
                                        if( !( isgraph(c ) || isspace(c) ) ){
                                                std::cout << "c = " << (int)c << ", " << c << "\n";
                                                BOOST_THROW_EXCEPTION(std::domain_error("bad graphic"));
                                        }
                                }
                        }
                }
                void put(visitor& v)const final{
                        v.attr_height( vs_.size() );
                        v.attr_width( size_ );
                        v.begin();
                        for( const auto& l : vs_ ){
                                size_t line_size = l.size();
                                assert( line_size <= size_ );
                                size_t delta = size_ - line_size;

                                size_t lpad;
                                switch(Centering_Policy){
                                        case centering_policy::left: 
                                                lpad = 0; 
                                                break;
                                        case centering_policy::center: 
                                                lpad = delta / 2;
                                                break;
                                        case centering_policy::right: 
                                                lpad = delta;
                                                break;
                                        default:
                                                assert( 0 && "unexpected");
                                }
                                size_t rpad = delta - lpad;
                                std::string buf;
                                buf += std::string(lpad,' ');
                                buf += l;
                                buf += std::string(rpad,' ');
                                assert( buf.size() == size_ );

                                v.put_line( buf );
                        }
                        v.end();
                }
        private:
                std::vector<std::string> vs_;
                size_t size_;
        };


        template<class Policy_Type>
        struct basic_picture_composer : public picture
        {
                using policy = Policy_Type;
                basic_picture_composer(handle lp, handle rp)
                        :lp_(std::move(lp))
                        ,rp_(std::move(rp))
                {}
                void put(visitor& v)const final{
                        buffer_visitor left, right;
                        lp_->put( left );
                        rp_->put( right );
                        policy::put_as_composed(v, left, right );
                }
        private:
                handle lp_;
                handle rp_;
        };

        template<centering_policy Centering_Policy>
        struct overlay_{
                using visitor = picture::visitor;
                
                /// \param left is the parent
                /// \param right is the child
                //
                // an example of this is if left is a graph, and right is the legend
                static void put_as_composed( visitor& v, buffer_visitor& left, buffer_visitor& right){
                        if( left.height()  < right.height() || left.width() < right.width() ){
                                BOOST_THROW_EXCEPTION(std::domain_error("not suitable"));
                        } 
                        std::cout << "putting as composed\n";

                        size_t width = left.width();
                        size_t height = left.height();

                        size_t x_start = 0;
                        size_t x_end = x_start + right.width();

                        size_t y_start = 0;
                        size_t y_end = y_start + right.height();


                        v.attr_width( width );
                        v.attr_height( height );
                        v.begin();

                        auto make_inside = [](auto&& min, auto&& max)->auto{
                                return [min,max](auto&& value)->auto{
                                        return min <= value && value < max;
                                };
                        };

                        auto inside_x = make_inside(x_start,x_end);
                        auto inside_y = make_inside(y_start,y_end);

                        for(size_t y=0;y!=height;++y){  
                                std::string buf;
                                buf.reserve( width );
                                for(size_t x=0;x!=width;++x){
                                        if( inside_x(x) && inside_y(y) ){
                                                buf += right.lines()[y - y_start][x - x_start];
                                        } else{
                                                buf += left.lines()[y][x];
                                        }
                                }
                                v.put_line( buf );
                        }

                        v.end();


                }
        };

        
        template<centering_policy Centering_Policy>
        struct above_below_{
                using visitor = picture::visitor;
                static void put_as_composed( visitor& v, buffer_visitor& left, buffer_visitor& right){

                        auto const height = left.height() + right.height();
                        auto const width = std::max/**/( left.width(), right.width() );

                        auto do_child = [&v,width](buffer_visitor& child){
                                size_t delta = width - child.width();

                                size_t lpad;

                                switch( Centering_Policy ){
                                        case centering_policy::center:
                                                lpad = delta / 2;
                                                break;
                                        case centering_policy::left:
                                                lpad = 0;
                                                break;
                                        case centering_policy::right:
                                                lpad = delta;
                                                break;
                                        default:
                                                assert( 0 && "unexpected");
                                }
                                                
                                size_t rpad = delta - lpad;

                                assert( width >= child.width() );
                                assert( lpad >= 0);
                                assert( rpad >= 0);

                                for(size_t i=0;i!=child.height();++i){
                                        assert( child.lines()[i].size() == child.width() );

                                        std::stringstream sstr;
                                        sstr << std::string(lpad,' ');
                                        sstr << child.lines()[i];
                                        sstr << std::string(rpad,' ');

                                        v.put_line( sstr.str() );
                                }
                        };

                        v.attr_width( width );
                        v.attr_height( height );
                        v.begin();

                        do_child( left );
                        do_child( right );

                        v.end();


                }
        };

        template<centering_policy Centering_Policy>
        struct side_by_side_{
                using visitor = picture::visitor;
                static void put_as_composed( visitor& v, buffer_visitor& left, buffer_visitor& right){
                        auto width = left.width() + right.width();
                        auto height = std::max/**/( left.height(), right.height() );
                        auto n = std::max/**/( left.lines().size() , right.lines().size() );

                        // these are the start and stops for each side, 
                        size_t left_begin, left_end;
                        size_t right_begin, right_end;

                        auto calc_begin_end = [&height](buffer_visitor& buf)->auto{
                                auto delta = height - buf.height();
                                auto buf_height = buf.height();
                                size_t begin;
                                switch(Centering_Policy){
                                        case centering_policy::center:
                                                begin = ( height - buf_height ) / 2;
                                                break;
                                        case centering_policy::top:
                                                begin = 0;
                                                break;
                                        case centering_policy::bottom:
                                                begin = height - buf_height;
                                                break;
                                        default:
                                                assert(0 && "unexpcted");
                                }
                                return std::make_tuple(begin, begin + buf_height );
                        };

                        std::tie(left_begin,left_end) = calc_begin_end(left);
                        std::tie(right_begin,right_end) = calc_begin_end(right);

                        std::string left_blank(left.width(),' ');
                        std::string right_blank(right.width(),' ');

                        v.attr_width( width );
                        v.attr_height( height );
                        v.begin();
                        for(size_t i = 0; i!= n;++i){
                                std::string tmp;
                                auto apply = [&tmp,&i](size_t begin, size_t end, buffer_visitor& buf, const std::string& blank){
                                        // if i \in [begin,end)
                                        if( begin <= i && i < end ){
                                                tmp += buf.lines()[i - begin];
                                        } else{
                                                tmp += blank;
                                        }
                                };

                                apply( left_begin, left_end, left, left_blank);
                                apply( right_begin, right_end, right, right_blank);

                                v.put_line( tmp );
                        }
                        v.end();
                        
                }
        };



        struct magnifier : public picture{
               explicit magnifier( handle impl):impl_(std::move(impl)){}
               void put(visitor& v)const final{
                       buffer_visitor buf;
                       impl_->put(buf);

                       size_t factor = 2;

                       v.attr_width( buf.width() * factor );
                       v.attr_height( buf.height() * factor );
                       v.begin();
                       for( std::string const& line : buf.lines()){
                               std::stringstream sstr;
                               for( char c : line ){
                                       for( size_t i = 0;i != factor;++i)
                                               sstr << c;
                               }
                               for( size_t i = 0;i != factor;++i)
                                        v.put_line( sstr.str() );
                       }
                       v.end();
               } 
        private:
                handle impl_;
        };

        template<typename Print_Policy>
        struct basic_frame_picture
                : public picture
                , private Print_Policy
        {
        private:
                using policy = Print_Policy;

                using tl_corner = typename policy::tl_corner;
                using tr_corner = typename policy::tr_corner;
                using bl_corner = typename policy::bl_corner;
                using br_corner = typename policy::br_corner;
                using top = typename policy::top;
                using side = typename policy::side;

                using this_type = basic_frame_picture;
        public:
                explicit basic_frame_picture(handle impl)
                        :impl_(std::move(impl))
                {}
                void put(visitor& v)const final{
                        buffer_visitor buffer;
                        impl_->put(buffer);

                        std::stringstream top_line;
                        top_line
                                << tl_corner()
                                << std::string(buffer.width(), top() )
                                << tr_corner()
                        ;
                        std::stringstream bottom_line;
                        bottom_line 
                                << bl_corner()
                                << std::string(buffer.width(), top() )
                                << br_corner()
                        ;
                        v.attr_width( buffer.width() + 2 );
                        v.attr_height( buffer.height() + 2 );
                        v.begin();
                        v.put_line( top_line.str() );
                        for( const auto& line : buffer.lines()){
                                std::string tmp;
                                tmp += side();
                                tmp += line;
                                tmp += side();
                                v.put_line( tmp );
                        }
                        v.put_line( bottom_line.str() );
                        v.end();
                        


                }
                handle impl_;
        };


        struct alpha_policy{
                using tl_corner = boost::mpl::char_<'+'>;
                using tr_corner = boost::mpl::char_<'+'>;
                using bl_corner = boost::mpl::char_<'+'>;
                using br_corner = boost::mpl::char_<'+'>;
                using top = boost::mpl::char_<'-'>;
                using side = boost::mpl::char_<'|'>;
        };
        
        struct beta_policy : public alpha_policy {
                using tl_corner = boost::mpl::char_<'/'>;
                using tr_corner = boost::mpl::char_<'\\'>;
                using bl_corner = boost::mpl::char_<'\\'>;
                using br_corner = boost::mpl::char_<'/'>;
        };

        using framed_picture_a = basic_frame_picture< alpha_policy >;
        using framed_picture_b = basic_frame_picture< beta_policy >;


};

namespace picture_compiler_registers{
}

/*
 *  tiny class just to imbue handle (std::shared_ptr<>) 
 *  with an operator<<
 */
struct compiled_picture{
        using handle = picture_compiler_detail::picture::handle;
        explicit compiled_picture(handle impl):impl_(std::move(impl)){}
        friend std::ostream& operator<<(std::ostream& ostr, compiled_picture const& self){
                self.display(ostr);
                return ostr;
        }
        void display(std::ostream& ostr = std::cout)const{
                ostr << *impl_ << "\n";
        }
private:
        handle impl_;
};

struct picture_compiler{
        using handle = picture_compiler_detail::picture::handle;

        #define PICTURE_COMPILER_reg_prefix tag_reg
        #define PICTURE_COMPILER_num_regs 8
        #define AUX(z,n,dummy) struct BOOST_PP_CAT(PICTURE_COMPILER_reg_prefix, n) {};
        BOOST_PP_REPEAT( PICTURE_COMPILER_num_regs, AUX , kj )
        #undef AUX
        using map_type = boost::fusion::map<
        #define AUX(z,n,dummy)                                  \
                BOOST_PP_COMMA_IF( n )                          \
                boost::fusion::pair<                            \
                BOOST_PP_CAT( PICTURE_COMPILER_reg_prefix, n ), \
                handle                                          \
        > 
        BOOST_PP_REPEAT( PICTURE_COMPILER_num_regs, AUX, ~ )
        #undef AUX
        >;
        using tag_vec = boost::mpl::vector<
        #define AUX(z,n,dummy) BOOST_PP_COMMA_IF(n) BOOST_PP_CAT(PICTURE_COMPILER_reg_prefix, n )
        BOOST_PP_REPEAT( PICTURE_COMPILER_num_regs, AUX, ~ )
        #undef AUX
        >;
        #undef PICTURE_COMPILER_reg_prefix
        #undef PICTURE_COMPILER_num_regs

        // helper macros
        #define PICTURE_COMPILER_check_tag( TAG) \
                static_assert( boost::mpl::contains< tag_vec, TAG >::value,"tag not a reg")


        #define DEF_TEXT_PICTURE_SV(r,d,POLICY)                                 \
                template<typename Tag>                                          \
                void BOOST_PP_CAT(make_text_picture_from_sv_,POLICY)            \
                        (const std::vector<std::string>& vs){                   \
                        PICTURE_COMPILER_check_tag( Tag );                      \
                        using namespace picture_compiler_detail;                \
                        using boost::fusion::at_key;                            \
                        using type = basic_text_picture                         \
                                <centering_policy::POLICY>;                     \
                        at_key<Tag>(map_) = std::make_shared<type>(vs);         \
                }

        BOOST_PP_SEQ_FOR_EACH( DEF_TEXT_PICTURE_SV, ~, (left)(right)(center))
        #undef DEF_TEXT_PICTURE_SV
        
        struct text_proxy_impl{
                using callback_t = std::function<void(const std::vector<std::string>&)>;
                explicit text_proxy_impl(callback_t callback)
                        :callback_(callback)
                {}
                ~text_proxy_impl(){
                        // this can throw !
                        callback_( buffer_ );
                }
                void push(const std::string& line){
                        buffer_.emplace_back(line);
                }
                std::string& back(){return buffer_.back();}
        private:
                callback_t callback_;
                std::vector<std::string> buffer_;
        };
        struct text_proxy{
                template<class Callback>
                explicit text_proxy(Callback&& callback)
                        :impl_(std::make_shared<text_proxy_impl>(
                                std::forward<Callback>(callback)))
                {}
                template<typename T>
                text_proxy& operator<<(T&& value){
                        impl_->push( boost::lexical_cast<std::string>(value ) );
                        return *this;
                }
                template<typename T>
                text_proxy& operator+(T&& value){
                        impl_->back() += boost::lexical_cast<std::string>(value);
                        return *this;
                }
        private:
                std::shared_ptr<text_proxy_impl> impl_;
        };
        #define DEF_MAKE_TEXT_PIC(r,d,POLICY)                                           \
                template<typename Tag>                                                  \
                text_proxy BOOST_PP_CAT(make_text_picture_,POLICY)(){                   \
                        auto callback = [this](const std::vector<std::string>& vs){     \
                                this->BOOST_PP_CAT(                                     \
                                        make_text_picture_from_sv_,                     \
                                                POLICY)<Tag>(vs);                       \
                        };                                                              \
                        return text_proxy(callback);                                    \
                }
        BOOST_PP_SEQ_FOR_EACH( DEF_MAKE_TEXT_PIC, ~, (left)(right)(center))
        #undef DEF_MAKE_TEXT_PIC



        template<typename Tag>
        compiled_picture compile(){
                PICTURE_COMPILER_check_tag( Tag );
                using boost::fusion::at_key;
                return compiled_picture{at_key<Tag>(map_)};
        }
        template<typename Tag>
        void make_magnify(){
                PICTURE_COMPILER_check_tag( Tag );
                using boost::fusion::at_key;
                using picture_compiler_detail::magnifier;
                auto tmp = at_key<Tag>(map_ );
                auto framed = std::make_shared<magnifier>( tmp );
                at_key<Tag>(map_) = framed;
        }
        template<typename Tag>
        void make_framed(){
                PICTURE_COMPILER_check_tag( Tag );
                using boost::fusion::at_key;
                using picture_compiler_detail::framed_picture_a;
                auto tmp = at_key<Tag>(map_ );
                auto framed = std::make_shared<framed_picture_a>( tmp );
                at_key<Tag>(map_) = framed;
        }
        template<typename Tag>
        void push(){
                PICTURE_COMPILER_check_tag( Tag );
                using boost::fusion::at_key;
                stack_.push_back( at_key<Tag>( map_ ) );
        }
        template<typename Tag>
        void pop(){
                PICTURE_COMPILER_check_tag( Tag );
                using boost::fusion::at_key;
                auto tmp = stack_.back();
                stack_.pop_back();
                at_key<Tag>(map_) = tmp;
        }
        size_t stack_size()const{
                return stack_.size();
        }
        template<typename Tag>
        void copy( handle ptr){
                PICTURE_COMPILER_check_tag( Tag );
                using boost::fusion::at_key;
                at_key<Tag>(map_) = ptr;
        }
        #define DEF_COMPOSE( COMPOSER, POLICY )                                        \
                template <typename Left, typename Right, typename Result>              \
                void BOOST_PP_CAT( make_, BOOST_PP_CAT( COMPOSER, POLICY ) )() {       \
                        PICTURE_COMPILER_check_tag( Result );                          \
                        PICTURE_COMPILER_check_tag( Left );                            \
                        PICTURE_COMPILER_check_tag( Right );                           \
                        using namespace picture_compiler_detail;                       \
                        using boost::fusion::at_key;                                   \
                        using meta_t = COMPOSER<centering_policy::POLICY>;             \
                        using composite_t = basic_picture_composer<meta_t>;            \
                        auto left = at_key<Left>( map_ );                              \
                        auto right = at_key<Right>( map_ );                            \
                        auto composed = std::make_shared<composite_t>( left, right );  \
                        at_key<Result>( map_ ) = composed;                             \
                }
        #define AUX(r, SEQ ) \
                DEF_COMPOSE( \
                        BOOST_PP_SEQ_ELEM(0, SEQ), \
                        BOOST_PP_SEQ_ELEM(1, SEQ) )   

        BOOST_PP_SEQ_FOR_EACH_PRODUCT(
                AUX, 
                        ((side_by_side_)(above_below_)) ((left)(center)(right))
        )
        BOOST_PP_SEQ_FOR_EACH_PRODUCT(
                AUX, 
                        ((overlay_))
                        ((left)(center)(right))
        )
        #undef AUX
        #undef DEF_COMPOSE

        void reverse_stack(){
                auto tmp = std::move(stack_);
                stack_.assign( tmp.rbegin(), tmp.rend());
        }
        #if 0
        static void debug(){
                using namespace picture_compiler_detail;
               
                std::vector<std::string> vs;
                vs.emplace_back("gerry  ");
                vs.emplace_back("-----  ");
                vs.emplace_back("*candy*");
                auto picture0 = std::make_shared<basic_text_picture<centering_policy::center> >(vs);

                vs.clear();
                vs.emplace_back("#####");
                vs.emplace_back("#   #");
                vs.emplace_back("#   #");
                vs.emplace_back("#   #");
                vs.emplace_back("#   #");
                vs.emplace_back("#####");

                auto picture1 = std::make_shared<make_text_picture_from_sv_left>(vs);
                auto framed0 = std::make_shared<framed_picture_a>(picture0);
                auto framed1 = std::make_shared<framed_picture_b>(framed0);
                auto sbs = std::make_shared<basic_picture_composer<side_by_side_<centering_policy::center> > >(picture0, picture1 );

                cout_visitor v;
                
                picture0->put( v );
                framed0->put( v );
                framed1->put( v );
                sbs->put( v );
        }
        static void self_test(){
                using namespace picture_compiler_detail;
                picture_compiler compiler;
                std::vector<std::string> vs;
                vs.emplace_back("gerry  ");
                vs.emplace_back("-----  ");
                vs.emplace_back("*candy*");
                compiler.make_text_picture_left<tag_reg0 >( vs );
                compiler.push< tag_reg0 >();
                vs.clear();
                vs.emplace_back("#####");
                vs.emplace_back("#   #");
                vs.emplace_back("#   #");
                vs.emplace_back("#   #");
                vs.emplace_back("#   #");
                vs.emplace_back("#####");
                compiler.make_text_picture_right< tag_reg0 >( vs );
                compiler.make_framed< tag_reg0 >();
                compiler.pop< tag_reg1> ();
                compiler.make_side_by_side_left<tag_reg2,tag_reg0,tag_reg1>();
                auto pic = compiler.compile<tag_reg2>();
                cout_visitor v;
                std::cout << *pic << "\n";
        }
        #endif

private:
        map_type map_;
        std::vector<handle> stack_;

        #undef PICTURE_COMPILER_check_tag
};
