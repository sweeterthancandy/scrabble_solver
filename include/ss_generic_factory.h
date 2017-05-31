#pragma once

#include <boost/range/algorithm.hpp>
#include <string>
#include <utility>
#include <boost/exception/all.hpp>
#include <iostream>
#include <memory>

namespace ss{


	template<class T, class... DeclArgs>
	struct generic_factory{
                using ptr_type = std::unique_ptr<T>;
                using fac_type = std::function<ptr_type(DeclArgs...)>;

                generic_factory& register_(std::string const& token, fac_type fac){
                        //std::cout << "(" << this << ")register_(" << token << ")\n";
			protos_.insert(std::make_pair(token, std::move(fac)));
			return *this;
		}
		template<class... Args>
		std::shared_ptr<T> make(std::string const& token, Args const... args){
                        //std::cout << "(" << this << ")find(" << token << ")\n";
			auto iter = protos_.find(token);
			if( iter == protos_.end() ){
                                std::stringstream sstr;
                                sstr << "unknown token=" << token;
				BOOST_THROW_EXCEPTION(std::domain_error(sstr.str()));
			}
			return iter->second(std::forward<Args>(args)...);
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
		std::map<std::string, fac_type> protos_;
	};
}
