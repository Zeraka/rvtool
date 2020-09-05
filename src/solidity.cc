#include "automonitor.hh"
#include "solidity.hh"
#include "util-base.hh"
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "util-parse.hh"
using namespace std;

int m = 0,i=0,j=0;
/*
功能：将一个只含有 & 运算符的布尔表达式形式的字符串解析到Word_set结构体中。
*/
 int Parse_bstr_to_wordset_of_sol(int& m, std::string &str, Word_set &word_set,ofstream& ofile)
{
    //FuncBegin();
    //按空格分割, 分别加入map中,如果包含！,则值为0，否则为1
    stringList sli = splitstr(str, ' ');

    word_set.word = str;
    ofile<<"        ws_"<<m<<".word=\""<<str<<"\";\n";

    for (auto &t : sli)
    {
        if(t == "&") continue;
        VePrint(t);
        if (t[0] != '!')
        {
            word_set.wordset[t] = 1;
            ofile<<"        ws_"<<m<<".wordset[\""<<t<<"\"] = 1;"<<"\n";
        }
        else
        {
            t = t.substr(1,t.size()-1);
            word_set.wordset[t] = 0; 
            ofile<<"        ws_"<<m<<".wordset[\""<<t<<"\"] = 0;"<<"\n";
        }
    }
    //FuncEnd();
    return SUCCESS;
}

 int Parse_label_to_word_sets_of_sol(int i, std::string &label, std::vector<Word_set> &word_sets,ofstream& ofile)
{
    FuncBegin();
    //把字符串按照 | 分割 然后送入word_set中去
    if (label.empty() == true)
    {
        INFOPrint("str is null");
        FuncEnd();
        return ERROR;
    }

    std::vector<std::string> rpn = Parse_label_exp_to_RPN(label);
    std::vector<std::string> label_set = Parse_label_RPN_to_string_sets(rpn);
    
    //int m = 0;
    for (auto &t : label_set)
    {
        Word_set ws;
        string ws_m = "ws_"+m;
        //ofile<<"        Word_set ws_"<<m<<";\n";
        Parse_bstr_to_wordset_of_sol(m, t, ws, ofile);

        word_sets.push_back(ws);
        ofile<<"        monitor_label_"<<i<<".word_sets.push(ws_"<<m<<");"<<"\n\n";
        m++;
    }

    std::cout <<BOLDBLUE <<"Parse label " << label << " SUCCESS" <<RESET<< std::endl;
    FuncEnd();
    return SUCCESS;
}

 int Parse_label_to_word_sets_of_sol_pre(int i, std::string &label, std::vector<Word_set> &word_sets,ofstream& ofile)
{
    FuncBegin();
    //把字符串按照 | 分割 然后送入word_set中去
    if (label.empty() == true)
    {
        INFOPrint("str is null");
        FuncEnd();
        return ERROR;
    }

    std::vector<std::string> rpn = Parse_label_exp_to_RPN(label);
    std::vector<std::string> label_set = Parse_label_RPN_to_string_sets(rpn);
    
    //int m = 0;
    for (auto &t : label_set)
    {
        Word_set ws;
        string ws_m = "ws_"+m;
        ofile<<"        Word_set  ws_"<<m<<";\n";
        m++;
    }

    std::cout <<BOLDBLUE <<"Parse label " << label << " SUCCESS" <<RESET<< std::endl;
    FuncEnd();
    m=0;
    return SUCCESS;
}


static int export_automata_to_solidity_pre(Monitor &monitor,spot::twa_graph_ptr &aut,
                                            const spot::bdd_dict_ptr& dict, ofstream& ofile)
{
    int num_state = 0;

    FuncBegin();

    
    ofile << "pragma solidity >=0.4.22 <0.6.0;\n\n\n"
          << "contract monitor_automata{\n" 
          << "  struct Word_set{\n"
          << "      string word;\n"
          << "      mapping (string=>uint) wordset;\n"
          <<"   }\n\n"
          <<"   struct Monitor_label{\n"
          <<"       string label;\n"
          <<"       string[] strlist;\n"
          <<"       Word_set[] word_sets;\n"
          <<"       int next_state;\n"
          <<"   }\n\n"
          <<"   struct Monitor_state{\n"
          <<"       int own_state;\n"
          <<"       int label_numbers;\n"
          <<"       Monitor_label[] monitor_labels;\n"
          <<"   }\n\n"
          <<"   struct Monitor{\n"
          <<"       int state_number;\n"
          <<"       mapping(uint=>Monitor_state) nodes;\n"
          <<"   }\n\n"
          <<"       Monitor monitor;\n"
          << endl;

    for (num_state; num_state < aut->num_states(); ++num_state) //对每个状态赋值
    {

        Monitor_state monitor_state;
        ofile <<"       Monitor_state  monitor_state_" << j << ";"
             << "\n";

        monitor_state.own_state = num_state;


        for (auto &t : aut->out(num_state))
        {
            Monitor_label monitor_label;
            ofile << "      Monitor_label  monitor_label_" << i << ";\n";
            
            if (t.src != num_state)
            {
                std::cout << "src != num_state, There is wrong" << std::endl;
                return ERROR;
            }

            monitor_label.label = spot::bdd_format_formula(dict, t.cond);

            VePrint(monitor_label.label);
            monitor_label.next_state = t.dst;

            //把表示逻辑运算的字符串解析放入wordset数据结构中
            if (Parse_label_to_word_sets_of_sol_pre(i, monitor_label.label, monitor_label.word_sets,ofile) != SUCCESS)
            {
                AMReturn(ERROR);
            }

            monitor_label.strlist = splitstr(monitor_label.label, ' ');
            VePrint(monitor_label.strlist[0]);

            monitor_state.monitor_labels.push_back(monitor_label);

            i++;
        }

        monitor_state.label_numbers = monitor_state.monitor_labels.size();
        monitor.nodes[num_state] = monitor_state;
        j++;
    }
    i=0;j=0;
    FuncEnd();
    return SUCCESS;
}

/*输出自动机为solidity代码*/
int export_automata_to_solidity(Monitor &monitor, spot::twa_graph_ptr &aut, const spot::bdd_dict_ptr &dict)
{
    int num_state = 0;

    FuncBegin();


    ofstream ofile;
    ofile.open("contract_monitor.sol", ios::out);
    export_automata_to_solidity_pre(monitor, aut, dict, ofile);
    m=0;
    i=0;
    j=0;
    ofile << "  function Monitor_init() public{\n"<<"\n";

    for (num_state; num_state < aut->num_states(); ++num_state) //对每个状态赋值
    {

        Monitor_state monitor_state;
        ofile << "      monitor_state_" << j << ".own_state=" << num_state << ";"
              << "\n\n";
        monitor_state.own_state = num_state;


        for (auto &t : aut->out(num_state))
        {
            Monitor_label monitor_label;
            
            if (t.src != num_state)
            {
                std::cout << "src != num_state, There is wrong" << std::endl;
                return ERROR;
            }

            ofile << "      monitor_label_" << i << ".label=\"" << spot::bdd_format_formula(dict, t.cond) << "\";\n";
            monitor_label.label = spot::bdd_format_formula(dict, t.cond);

            VePrint(monitor_label.label);
            monitor_label.next_state = t.dst;
            ofile << "      monitor_label_" << i << ".next_state=" << t.dst << ";\n\n";

            //把表示逻辑运算的字符串解析放入wordset数据结构中
            if (Parse_label_to_word_sets_of_sol(i, monitor_label.label, monitor_label.word_sets,ofile) != SUCCESS)
            {
                AMReturn(ERROR);
            }

            monitor_label.strlist = splitstr(monitor_label.label, ' ');
            VePrint(monitor_label.strlist[0]);

            monitor_state.monitor_labels.push_back(monitor_label);
            ofile << "      monitor_state_" << j << ".monitor_labels.push(monitor_label_"<<i<<");\n";

            i++;
        }

        monitor_state.label_numbers = monitor_state.monitor_labels.size();
        ofile << "      monitor_state_" << j << ".label_numbers="
              <<monitor_state.monitor_labels.size()//这里的size是否正确？
              << ";\n";

        monitor.nodes[num_state] = monitor_state;//这里要修改
        ofile << "      monitor.nodes["<<num_state<<"]" << " = monitor_state_" << j << ";\n";
        j++;
    }
        ofile << "  }" << endl;
    ofile << "}" << endl;
    FuncEnd();
    return SUCCESS;
}
//缺少将Monitor结构体进行赋值的状态。
//声明Monitor以及需要将Monitor进行一个比较。
