#include "ss_dict_helpers.h"

namespace{
        inline std::string toupper(std::string s){
                for(auto& c : s)
                        c = std::toupper(c);
                return std::move(s);
        }
}

namespace ss{
        std_dictionary::std_dictionary(std::vector<std::string> const& seq):
                words_(seq)
        {
                boost::sort(words_);
        }

        bool std_dictionary::contains(std::string const& word)const{
                return boost::binary_search(words_, toupper(word));
        }
        bool std_dictionary::contains_prefix(std::string const& prefix)const{
                auto iter = boost::lower_bound(words_, toupper(prefix));
                if( iter != words_.end() ){
                        return 
                            iter->size() >= prefix.size() &&
                            iter->substr(0,prefix.size()) == toupper(prefix);
                }
                return false;
        }
}

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
                                return std::make_unique<std_dictionary>(std::move(proto_dict));
                        } ),0);
}
