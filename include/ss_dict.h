#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "ss_generic_factory.h"

namespace ss{


        struct dictionary_t{
                #if 0
                struct sorter{
                        void operator()(std::string const& l_param,
                                        std::string const& r_param)const
                        {
                                if( l_param.size() == r_param.size() ){
                                        return l_param < r_param;
                                }
                                return l_param.size() < r_param.size();
                        }
                };
                struct prefix_cmp{
                        void operator()(std::string const& l_param,
                                        std::string const& r_param)const
                        {

                        }
                };
                #endif

                dictionary_t(){}
                explicit dictionary_t(std::vector<std::string> const& seq):
                        words_(seq)
                {
                        boost::sort(words_);
                }

                bool contains(std::string const& word)const{
                        return boost::binary_search(words_, word);
                }
                bool contains_prefix(std::string const& prefix)const{
                        auto iter = boost::lower_bound(words_, prefix);
                        if( iter != words_.end() ){
                                return 
                                    iter->size() >= prefix.size() &&
                                    iter->substr(0,prefix.size()) == prefix;
                        }
                        return false;
                }
                #if 0
                bool prefix(std::string const& word)const{
                        auto iter = boost::upper_bound(words_, word);
                }
                #endif
        private:
                std::vector<std::string> words_;
                std::vector<std::string> sdrow_;
        };

        using dictionary_factory = generic_factory<dictionary_t>;
}
