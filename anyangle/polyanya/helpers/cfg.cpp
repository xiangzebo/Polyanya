#include "cfg.h"

warthog::util::cfg::cfg()
{
}

warthog::util::cfg::~cfg()
{
}

void 
warthog::util::cfg::parse_args(int argc, char** argv, warthog::util::param params[])
{
    parse_args(argc, argv, "", params);
}

void 
warthog::util::cfg::parse_args(int argc, char** argv, const char* options,
        warthog::util::param params[])
{
	int current_opt;
	for(    int c = getopt_long(argc, argv, options, params, &current_opt) ;
            c != -1 ; 
            c = getopt_long(argc, argv, options, params, &current_opt) )
	{
        if(c == '?') break;
        if(params[current_opt].has_arg == 0)
        {
            params_[params[current_opt].name].push_back("1");
        }
        if(!(optarg == 0))
        {
            params_[params[current_opt].name].push_back(optarg);
            for(int index = optind; 
                    index < argc && *argv[index] != '-'; 
                    index++)
            {
                params_[params[current_opt].name].push_back(argv[index]);
            }
        }
	}
}

std::string
warthog::util::cfg::get_param_value(std::string param_name)
{
	std::string ret("");
	std::map<std::string, std::vector<std::string>>::iterator it = 
		params_.find(param_name);
	if(it != params_.end())
	{
        if((*it).second.size() > 0)
        {
            ret = (*it).second.front();
            (*it).second.erase((*it).second.begin());
        }
	}
	return ret;
}

uint32_t
warthog::util::cfg::get_num_values(std::string param_name)
{
    uint32_t ret = 0;
	std::map<std::string, std::vector<std::string>>::iterator it = 
		params_.find(param_name);
	if(it != params_.end())
	{
        ret = (uint32_t)(*it).second.size();
    }
    return ret;
}

void
warthog::util::cfg::print(std::ostream& out)
{
	out << "cfg\n";
	for(std::map<std::string, std::vector<std::string>>::iterator it = params_.begin();
			it != params_.end(); it++)
	{
		out << (*it).first <<"=";
        for(uint32_t i = 0; i < (*it).second.size(); i++)
        {
             out << (*it).second.at(i);
        }
	}
}

void
warthog::util::cfg::print_values(const std::string& option, std::ostream& out)
{
    std::map<std::string, std::vector<std::string>>::iterator it 
        = params_.find(option);
    if(it != params_.end())
    {
        for(uint32_t i = 0; i < (*it).second.size(); i++)
        {
            out << " " << (*it).second.at(i);
        }
    }
}
