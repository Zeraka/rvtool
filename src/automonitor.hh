#pragma once 

#include "util-base.h"
#include<vector>
#include<string>

typedef struct Monitor_label_t{
    std::string label;
    int next_state;
}Monitor_label;

typedef struct Monitor_state_t{
    int current_state;
    int label_numbers;
    std::vector<Monitor_label> monitor_labels;
}Monitor_state;

/*
    Size_t means state_numbers.
*/
typedef std::map<size_t , Monitor_state> Monitor;


int check_word_acceptance( spot::twa_graph_ptr &aut,
                          Monitor &monitor, const spot::bdd_dict_ptr &dict, std::string accept_word);
int Parse_automata_to_monitor(Monitor& monitor, spot::twa_graph_ptr &aut, const spot::bdd_dict_ptr &dict);
int test_check_word_acceptance_01( spot::twa_graph_ptr& aut, 
                                Monitor &monitor, const spot::bdd_dict_ptr &dict);

/*自定义输出自动机*/                               
