#include "ss_dict.h"

namespace{
        using namespace ss;

        int _ = (
               dictionary_factory::get_inst()->register_(
                       "regular",
                        [](){
                                std::ifstream fstr("dictionary.txt");
                                std::vector<std::string> proto_dict;
                                for(;;){
                                        std::string line;
                                        std::getline(fstr, line);
                                        if(line.size()){
                                                proto_dict.emplace_back(std::move(line));
                                        }
                                        if( fstr.eof() )
                                                break;
                                }
                                return std::make_unique<dictionary_t>(std::move(proto_dict));
                        } ),0);
}
