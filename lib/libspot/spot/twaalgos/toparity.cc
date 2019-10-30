// -*- coding: utf-8 -*-
// Copyright (C) 2018, 2019 Laboratoire de Recherche et Développement
// de l'Epita.
//
// This file is part of Spot, a model checking library.
//
// Spot is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// Spot is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "config.h"
#include <spot/twaalgos/toparity.hh>

#include <deque>
#include <map>

#include <sstream>

namespace spot
{

  namespace
  {
    struct lar_state
    {
      unsigned state;
      std::vector<unsigned> perm;

      bool operator<(const lar_state& s) const
      {
        return state == s.state ? perm < s.perm : state < s.state;
      }

      std::string to_string() const
      {
        std::stringstream s;
        s << state << " [";
        unsigned ps = perm.size();
        for (unsigned i = 0; i != ps; ++i)
          {
            if (i > 0)
              s << ',';
            s << perm[i];
          }
        s << ']';
        return s.str();
      }
    };

    class lar_generator
    {
      const const_twa_graph_ptr& aut_;
      twa_graph_ptr res_;
      const bool pretty_print;

      std::map<lar_state, unsigned> lar2num;
    public:
      explicit lar_generator(const const_twa_graph_ptr& a, bool pretty_print)
      : aut_(a)
      , res_(nullptr)
      , pretty_print(pretty_print)
      {}

      twa_graph_ptr run()
      {
        res_ = make_twa_graph(aut_->get_dict());
        res_->copy_ap_of(aut_);

        std::deque<lar_state> todo;
        auto get_state = [this, &todo](const lar_state& s)
          {
            auto it = lar2num.emplace(s, -1U);
            if (it.second) // insertion took place
              {
                unsigned nb = res_->new_state();
                it.first->second = nb;
                todo.push_back(s);
              }
            return it.first->second;
          };

        {
          std::vector<unsigned> p0;
          for (unsigned k : aut_->acc().all_sets().sets())
            p0.push_back(k);
          lar_state s0{aut_->get_init_state_number(), p0};
          unsigned init = get_state(s0); // put s0 in todo
          res_->set_init_state(init);
        }

        // main loop
        while (!todo.empty())
          {
            lar_state current = todo.front();
            todo.pop_front();

            // TODO todo could store this number to avoid one lookup
            unsigned src_num = get_state(current);

            for (const auto& e : aut_->out(current.state))
              {
                // find the new permutation
                std::vector<unsigned> new_perm = current.perm;
                unsigned h = 0;
                for (unsigned k : e.acc.sets())
                  {
                    auto it = std::find(new_perm.begin(), new_perm.end(), k);
                    h = std::max(h, unsigned(new_perm.end() - it));
                    std::rotate(it, it+1, new_perm.end());
                  }

                lar_state dst{e.dst, new_perm};
                unsigned dst_num = get_state(dst);

                // do the h last elements satisfy the acceptance condition?
                acc_cond::mark_t m(new_perm.end() - h, new_perm.end());
                if (aut_->acc().accepting(m))
                  res_->new_edge(src_num, dst_num, e.cond, {2*h});
                else
                  res_->new_edge(src_num, dst_num, e.cond, {2*h+1});
              }
          }

        // parity max even
        res_->set_acceptance(2*aut_->num_sets() + 2,
            acc_cond::acc_code::parity(true, false, 2*aut_->num_sets() + 2));

        if (pretty_print)
          {
            auto names = new std::vector<std::string>(res_->num_states());
            for (const auto& p : lar2num)
              (*names)[p.second] = p.first.to_string();
            res_->set_named_prop("state-names", names);
          }

        return res_;
      }
    };
  }

  twa_graph_ptr
  to_parity(const const_twa_graph_ptr& aut, bool pretty_print)
  {
    if (!aut->is_existential())
      throw std::runtime_error("LAR does not handle alternation");
    // if aut is already parity return it as is
    if (aut->acc().is_parity())
      return std::const_pointer_cast<twa_graph>(aut);

    lar_generator gen(aut, pretty_print);
    return gen.run();
  }

}
