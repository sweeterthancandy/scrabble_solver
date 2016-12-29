#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "ss_generic_factory.h"

namespace ss{

        using dictionary_t = std::vector<std::string>;
        using dictionary_factory = generic_factory<dictionary_t>;

        namespace auto_reg{
                int dummy = (dictionary_factory::get_inst() ->register_( "regular", [](){
                        std::ifstream fstr("dictionary.txt");
                        auto proto_dict = std::make_shared<dictionary_t>();
                        for(;;){
                                std::string line;
                                std::getline(fstr, line);
                                if(line.size()){
                                        proto_dict->emplace_back(std::move(line));
                                }
                                if( fstr.eof() )
                                        break;
                        }
                        return std::move(proto_dict);
                }()), 0 );
        }
}
