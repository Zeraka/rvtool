/*
transfer LTL to monitor.
*/
#include <iostream>
#include <spot/tl/parse.hh>
#include <spot/twaalgos/ltl2tgba_fm.hh>
#include <spot/twaalgos/sccfilter.hh>
#include <spot/twaalgos/stripacc.hh>
#include <spot/twaalgos/minimize.hh>
#include <spot/twaalgos/hoa.hh>

int main()
{
  spot::parsed_formula pf = spot::parse_infix_psl("G(press -> red U green)");
  if (pf.format_errors(std::cerr))
    return 1;
  // 1. translate LTL formula into TGBA
  spot::twa_graph_ptr aut = spot::ltl_to_tgba_fm(pf.f, spot::make_bdd_dict());
  // 2. remove "dead" SCCs
  aut = spot::scc_filter(aut);
  // 3. strip the acceptance condition (in place)
  spot::strip_acceptance_here(aut);
  // 4. & 5. determinize and minimize the automaton
  aut = spot::minimize_monitor(aut);
  // output the result.
  print_hoa(std::cout, aut) << '\n';
  return 0;
}