/*
This file shows how to parse HOA format file to your own data structure.
*/

#include <string>
#include <iostream>
#include <spot/parseaut/public.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twa/bddprint.hh>

void custom_print(std::ostream& out, spot::twa_graph_ptr& aut);

int main()
{
  spot::parsed_aut_ptr pa = parse_aut("demo.hoa", spot::make_bdd_dict());//解析自动机。得到的pa是解析出来的
  if (pa->format_errors(std::cerr))//检查有无格式错误。检查有无其他的错误。
    return 1;
  // This cannot occur when reading a never claim, but
  // it could while reading a HOA file.
  if (pa->aborted)
    {
      std::cerr << "--ABORT-- read\n";
      return 1;
    }
  custom_print(std::cout, pa->aut);//pa->aut是其中的
}

void custom_print(std::ostream& out, spot::twa_graph_ptr& aut)//打印出新的格式的自动机
{
  // We need the dictionary to print the BDDs that label the edges
  const spot::bdd_dict_ptr& dict = aut->get_dict(); //get_dict()的意思就是把东西拿来

  // Some meta-data...
  out << "Acceptance: " << aut->get_acceptance() << '\n'; //得到了接受
  out << "Number of sets: " << aut->num_sets() << '\n';//得到了
  out << "Number of states: " << aut->num_states() << '\n';
  out << "Number of edges: " << aut->num_edges() << '\n';
  out << "Initial state: " << aut->get_init_state_number() << '\n';
  out << "Atomic propositions:";
  for (spot::formula ap: aut->ap())
      out << ' ' << ap << " (=" << dict->varnum(ap) << ')';
  out << '\n';

  // Arbitrary data can be attached to automata, by giving them
  // a type and a name.  The HOA parser and printer both use the
  // "automaton-name" to name the automaton.
  if (auto name = aut->get_named_prop<std::string>("automaton-name"))
     out << "Name: " << *name << '\n';

  // For the following prop_*() methods, the return value is an
  // instance of the spot::trival class that can represent
  // yes/maybe/no.  These properties correspond to bits stored in the
  // automaton, so they can be queried in constant time.  They are
  // only set whenever they can be determined at a cheap cost: for
  // instance an algorithm that always produces deterministic automata
  // would set the deterministic property on its output.  In this
  // example, the properties that are set come from the "properties:"
  // line of the input file.
  out << "Complete: " << aut->prop_complete() << '\n';
  out << "Deterministic: " << (aut->prop_universal()
                               && aut->is_existential()) << '\n';
  out << "Unambiguous: " << aut->prop_unambiguous() << '\n';
  out << "State-Based Acc: " << aut->prop_state_acc() << '\n';
  out << "Terminal: " << aut->prop_terminal() << '\n';
  out << "Weak: " << aut->prop_weak() << '\n';
  out << "Inherently Weak: " << aut->prop_inherently_weak() << '\n';
  out << "Stutter Invariant: " << aut->prop_stutter_invariant() << '\n';

  // States are numbered from 0 to n-1
  unsigned n = aut->num_states();
  for (unsigned s = 0; s < n; ++s)
    {
      out << "State " << s << ":\n";

      // The out(s) method returns a fake container that can be
      // iterated over as if the contents was the edges going
      // out of s.  Each of these edges is a quadruplet
      // (src,dst,cond,acc).  Note that because this returns
      // a reference, the edge can also be modified.
      for (auto& t: aut->out(s))
        {
          out << "  edge(" << t.src << " -> " << t.dst << ")\n    label = ";
          spot::bdd_print_formula(out, dict, t.cond);
          out << "\n    acc sets = " << t.acc << '\n';

          out << "print the ostream to string " << "\n";
          
        }
    }
}