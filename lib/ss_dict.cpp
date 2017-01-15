#include "ss_dict.h"

namespace{
        using namespace ss;

        int _ = (
               dictionary_factory::get_inst()->register_(
                       "regular",
                        [](){
                                std::ifstream fstr("dictionary.txt");
                                auto proto_dict = std::make_unique<dictionary_t>();
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
                        } ),0);
}
