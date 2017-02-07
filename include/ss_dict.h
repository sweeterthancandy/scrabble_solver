#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "ss_generic_factory.h"

namespace ss{

	/*
	 * For dictionaryies you've got 2 things,
	 * the mechinism which does queues of the list of words,
	 * and the list of words
	 *
	 */

        struct dictionary_t{
		virtual ~dictionary_t()=default;
                virtual bool contains(std::string const& word)const=0;
                virtual bool contains_prefix(std::string const& prefix)const=0;
	};
        
	using dictionary_factory = generic_factory<dictionary_t>;

	// only really expect one implemention of the interface,
	// but its nice for debugging to have pesudo implemtations
	struct std_dictionary : public dictionary_t{
                std_dictionary(){}
                explicit std_dictionary(std::vector<std::string> const& seq):
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
        private:
                std::vector<std::string> words_;
                std::vector<std::string> sdrow_;
        };

	template<bool B>
	struct bool_dictionary : public dictionary_t{
                bool contains(std::string const& word)const{
			return B;
                }
                bool contains_prefix(std::string const& prefix)const{
			return B;
		}
	};

	using true_dictionary  = bool_dictionary<true>;
	using false_dictionary = bool_dictionary<false>;


	// these are purly for debugging

}
