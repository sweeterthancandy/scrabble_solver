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


}

#include "ss_dict_helpers.h"
