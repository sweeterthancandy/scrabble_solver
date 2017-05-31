#ifndef SUB_COMMAND_H
#define SUB_COMMAND_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <boost/exception/all.hpp>

struct game_context;

struct sub_command{
        using handle = std::shared_ptr<sub_command>;
        virtual ~sub_command(){}
        virtual int run(game_context& ctx, std::vector<std::string> const& args)=0;
};

struct sub_command_factory{

        using factory_t = std::function<sub_command::handle()>;

        static sub_command_factory* get(){
                static sub_command_factory* self{0};
                if( self == nullptr )
                        self = new sub_command_factory;
                return self;
        }
        void register_(std::string const& name, factory_t fac){
                fm_[name] = fac;
        }
        auto make(std::string const& name)const{
                if( ! fm_.count(name))
                        BOOST_THROW_EXCEPTION(std::domain_error("no subcommand " + name));
                return fm_.find(name)->second();
        }
        void print_help(std::string const& prog_name){
                std::cout << prog_name << " <sub-command>\n";
                for( auto const& p : fm_ )
                        std::cout << "    " << p.first << "\n";
        }
private:
        std::map<std::string, factory_t> fm_;
};

#endif // SUB_COMMAND_H
