#include <gtest/gtest.h>

#include "picture_compiler.h"
        
using tag_reg0 = picture_compiler::tag_reg0;
using tag_reg1 = picture_compiler::tag_reg1;
using tag_reg2 = picture_compiler::tag_reg2;
using tag_reg3 = picture_compiler::tag_reg3;

TEST( picture_compiler, picture_from_string_vector){
        picture_compiler compiler;
        using tag_reg0 = picture_compiler::tag_reg0;
        std::vector<std::string> vs;
        vs.emplace_back("gerry  ");
        vs.emplace_back("-----  ");
        vs.emplace_back("*candy*");
        compiler.make_text_picture_from_sv_left<tag_reg0 >( vs );
        auto pic = compiler.compile< tag_reg0 >();
        pic.display();
}

TEST( picture_compiler, picture_from_string){
        picture_compiler compiler;
        compiler.make_text_picture_right<tag_reg0>()
                << "hello"
                << "word";
        auto pic = compiler.compile< tag_reg0 >();
        pic.display();
}


TEST( picture_compiler, framed){
        picture_compiler compiler;
        compiler.make_text_picture_center<tag_reg0>()
                << "hello"
                << "cruel"
                << "wonderfull"
                << "word";
        compiler.make_framed<tag_reg0>();
        auto pic = compiler.compile< tag_reg0 >();
        pic.display();
}

TEST( picture_compiler, side_by_side){
        picture_compiler compiler;
        compiler.make_text_picture_center<tag_reg0>()
                << "left"
                << "__left_"
                << "left__________"
                << "____left"
                << "____left__";
        compiler.make_framed<tag_reg0>();


        compiler.make_text_picture_center<tag_reg1>()
                << "__right"
                << "_right_"
                << "right__";

        compiler.make_side_by_side_center<tag_reg0, tag_reg1, tag_reg0>();
        
        compiler.make_framed<tag_reg0>();

        auto pic = compiler.compile< tag_reg0 >();
        pic.display();
}

TEST( picture_compiler, overlay){
        picture_compiler compiler;
        compiler.make_text_picture_center<tag_reg0>()
                << "XXXXXXXXXXXXXX"
                << "XXXXXXXXXXXXXX"
                << "XXXXXXXXXXXXXX"
                << "XXXXXXXXXXXXXX"
                << "XXXXXXXXXXXXXX";

        compiler.make_text_picture_center<tag_reg1>()
                << "YYYYY"
                << "YYYYY"
                << "YYYYY";


        compiler.make_overlay_center<tag_reg0, tag_reg1, tag_reg2>();
        compiler.make_framed<tag_reg2>();
        compiler.compile< tag_reg2 >().display();
        
        compiler.make_overlay_center<tag_reg1, tag_reg0, tag_reg2>();
        compiler.make_framed<tag_reg2>();
        //EXPECT_THROW_ANY(compiler.compile< tag_reg2 >());
}
