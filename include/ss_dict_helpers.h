#pragma once

#include <vector>
#include <string>

#include "ss_generic_factory.h"
#include "ss_dict.h"

namespace ss{
	// only really expect one implemention of the interface,
	// but its nice for debugging to have pesudo implemtations
	struct std_dictionary : public dictionary_t{
                std_dictionary(){}
                explicit std_dictionary(std::vector<std::string> const& seq);

                bool contains(std::string const& word)const;
                bool contains_prefix(std::string const& prefix)const;
        private:
                std::vector<std::string> words_;
                std::vector<std::string> sdrow_;
        };

	template<bool B>
	struct bool_dictionary : public dictionary_t{
                // can't have anything less than a two letter word
                bool contains(std::string const& word)const{
			return ( word.size() < 2 ? false : B );
                }
                bool contains_prefix(std::string const& prefix)const{
			return B;
		}
	};

	using true_dictionary  = bool_dictionary<true>;
	using false_dictionary = bool_dictionary<false>;


	// these are purly for debugging
}
