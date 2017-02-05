#include <gtest/gtest.h>

#include "ss_io.h"
#include "ss_board.h"

class io : public testing::Test {
protected:
        virtual void SetUp()override {
                ss::board proto{10,10};
                char c{'a'};
                for(int i{0};i!=10;++i){
                        proto(i,i) = c;
                }
                b = std::move(proto);
        }
        ss::board b;
        //ss::composer composer;
};

TEST_F(io, dummy){
        ss::io::board_renderer r(b);
        r.put_tag("hello","world");
        r.display();
}

#if 0

using ss::io::placeholders;

TEST(io, composer_0) {
        composer.push(b);
        auto item = composer.make();
        item.render();
}


TEST(io, composer_1) {
        composer.push( _1 );
        auto item = composer.make();
        EXPECT_THROW( item.render() );
        item.render( b );
}

TEST(io, composer_2 ){
        composer.push( _1 );
}
#endif
