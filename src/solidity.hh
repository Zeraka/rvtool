#ifndef SOLIDITY_HH
#define SOLIDITY_HH
#include <spot/tl/parse.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/dot.hh>

#include <spot/twa/bddprint.hh>
#include <spot/parseaut/public.hh>

#include "automonitor.hh"

/*
将自动机生成solidity程序。
*/
int export_automata_to_solidity(Monitor &monitor, spot::twa_graph_ptr &aut, const spot::bdd_dict_ptr &dict);

#endif