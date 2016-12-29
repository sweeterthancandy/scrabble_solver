#pragma once

#include <boost/range/algorithm.hpp>
#include <string>
#include <utility>
#include <boost/exception/all.hpp>
#include <iostream>
#include <memory>

namespace ss{

	template<class T>
	struct generic_factory{ generic_factory& register_(std::string const& token, std::shared_ptr<T> proto){
                        std::cerr << "registing token=" << token << "\n";
			protos_.insert(std::make_pair(token, std::move(proto)));
			return *this;
		}
		template<class... Args>
		std::shared_ptr<T> make(std::string const& token, Args const... args){
                        
                        boost::for_each( protos_, [&](auto&& _){
                                std::cout << _.first << ",";
                        });
                        std::cout << "\n";
			auto iter = protos_.find(token);
			if( iter == protos_.end() ){
                                std::stringstream sstr;
                                sstr << "unknown token=" << token;
				BOOST_THROW_EXCEPTION(std::domain_error(sstr.str()));
			}
			return iter->second->clone(std::forward<Args>(args)...);
		}

		static generic_factory* get_inst(){
			static std::unique_ptr<generic_factory> mem;
			if( ! mem ){
				mem.reset(new generic_factory);
			}
			return mem.get();
		}
	private:
		generic_factory()=default;
		generic_factory(generic_factory const&)=default;
		generic_factory(generic_factory&&)=default;
		generic_factory& operator=(generic_factory const&)=default;
		generic_factory& operator=(generic_factory&&)=default;
	private:
		std::map<std::string, std::shared_ptr<T> > protos_;
	};
}
