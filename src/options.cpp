# include "options.hpp"
# include <list>
# include <iostream>

struct Option{
    std::string name;
    int* value;
    int min;
    int max;
};

std::list<Option> options_list;
int add_option(Option option) { options_list.push_back(option); return 0; }

void set_option(std::string name, int value){
    for (auto option : options_list){
        if (name == option.name){
            *option.value = value;
            return;
        }
    }
    std::cout << "Unknown option " << name << std::endl;
}

void list_options(){
    for (auto option : options_list){
        std::cout << "option name " << option.name << " type spin default " << *option.value 
        << " min " << option.min << " max " << option.max << std::endl;
    }
}

# define DEFINE_OPTION(name, default, min, max) int name = default; int _init_##name = add_option(Option{#name, &name, min, max});

DEFINE_OPTION(aw_start, 50, 8, 200);
DEFINE_OPTION(aw_increase, 32, 8, 40);

DEFINE_OPTION(rfp_margin, 128, 20, 500);
DEFINE_OPTION(nmp_depth, 3, 1, 10);

DEFINE_OPTION(lmr0, 4, 1, 128);
DEFINE_OPTION(lmr1, 4, 1, 128);
DEFINE_OPTION(lmr2, 8, 1, 128);
DEFINE_OPTION(lmr3, 16, 1, 128);

DEFINE_OPTION(tm_fraction, 40, 20, 100);
