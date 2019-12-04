#pragma once

#include "util-base.hh"
#include <vector>
#include <string>

#define Test_AUTOMONITOR 0
/*初始化时自动机的全局状态*/

typedef struct word_set_t
{
    std::string word;
    std::map<std::string, size_t> wordset;
} Word_set;

typedef struct Monitor_label_t  
{
    std::string label;//原始label的字符串格式
    stringList strlist;//被切分的字符串
    std::vector<Word_set> word_sets;
    int next_state;
} Monitor_label;

typedef struct Monitor_state_t
{
    int own_state;
    int label_numbers;
    std::vector<Monitor_label> monitor_labels;
} Monitor_state;

/*
    Size_t means state_numbers.
*/

typedef std::map<size_t, Monitor_state> Nodes;

/*Monitor的实体*/
typedef struct Monitor_
{
    int state_number; //monitor的全局状态
    Nodes nodes;
} Monitor;

typedef std::vector<Word_set> acc_word_sets;

int check_accept_word_format(std::string accept_word);
int Check_word_acceptance(spot::twa_graph_ptr &aut,
                          Monitor &monitor, const spot::bdd_dict_ptr &dict, std::string accept_word);
int Parse_automata_to_monitor(Monitor &monitor, spot::twa_graph_ptr &aut, const spot::bdd_dict_ptr &dict);

/*自定义输出自动机*/
int Parse_BoolString_to_set(std::string str, Word_set &word_set,
                            const spot::bdd_dict_ptr &dict);
std::vector<std::string> Parse_label_exp_to_RPN(std::string &label);
//int Parse_bstr_to_wordset(std::string&str, Word_set &word_set);
int label_match_word(Monitor_label &monitor_label, std::string accept_word);
int Parse_label_to_word_sets(std::string& label, std::vector<Word_set> &word_sets);

/*Unit Test Module*/
int Test_Parse_bstr_to_wordset();
int Test_Check_word_acceptance_01();
int Test_Check_word_acceptance_02(spot::twa_graph_ptr &aut,
                                  Monitor &monitor, const spot::bdd_dict_ptr &dict);
int Test_bdd_print(const spot::bdd_dict_ptr &dict, bdd b); //测试bddprint.h

int Test_Commnunication_module_01(spot::twa_graph_ptr &aut, Monitor &monitor,
                                  const spot::bdd_dict_ptr &dict);
int Test_Communication_module_02();
int Test_Parse_label_exp_to_RPN();
int Test_Parse_label_RPN_to_string_sets();
//end