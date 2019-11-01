#pragma once 

#include "util-base.h"
#include<vector>
#include<string>


#define TEST_AUTOMONITOR    1

typedef struct word_set{
    std::string word;
    std::map<std::string, size_t> wordset;
}Word_set;

typedef struct Monitor_label_t{
    std::string label;
    Word_set word_set;
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



typedef std::vector<Word_set>   acc_word_sets;

int check_word_acceptance( spot::twa_graph_ptr &aut,
                          Monitor &monitor, const spot::bdd_dict_ptr &dict, std::string accept_word);
int Parse_automata_to_monitor(Monitor& monitor, spot::twa_graph_ptr &aut, const spot::bdd_dict_ptr &dict);
int test_check_word_acceptance_01( spot::twa_graph_ptr& aut, 
                                Monitor &monitor, const spot::bdd_dict_ptr &dict);

int test_bdd_print( const spot::bdd_dict_ptr &dict, bdd b);//测试bddprint.h
/*自定义输出自动机*/                               
