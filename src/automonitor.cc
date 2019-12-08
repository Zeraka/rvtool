#include <iostream>
#include <ostream>
#include <sstream>
#include <cstring>
#include <string>
#include <fstream> //Using ofstream
#include <stack>

#include <cstdio>

#include <spot/tl/parse.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/dot.hh>

#include <spot/twa/bddprint.hh>
#include <spot/parseaut/public.hh>

#include <yaml-cpp/yaml.h>
#include <zmq.hpp>

#include <graphviz/gvc.h> // if you want to get dot file to image.

#include "automonitor.hh"
#include "util-base.hh"
#include "util-debug.hh"
#include "server.hpp"
#include "parsehoa.hh"
#include "ltl-parse.hh"
#include "util-error.hh"

extern "C"
{

#include "cJSON.h"
}

using namespace std;

static int state_number = 0;
static int Test_splitstr();

int main(void)
{
    FuncBegin();
#if Test_AUTOMONITOR == 0

    YAML::Node node = YAML::LoadFile("automonitor.yaml");

    if (node == nullptr)
    {
        ErrorPrintNReturn(YAML_FILE_IS_NULL);
    }

    std::ofstream errorLog(node["output"]["error_log"].as<std::string>(), std::ios::app);

    std::string filename;
    std::string fileFormat;
    spot::parsed_aut_ptr pa;
    spot::twa_graph_ptr aut;

    //Todo Init the Log();

    //Parse the Yaml file to Generate monitor.
    if (node["monitor_generate_module"]["open_hoa_file"]["enabled"].as<bool>() == true)
    {
        //LocationPrint();
        INFOPrint("Enter open_hoa_file module");
        filename = node["monitor_generate_module"]["open_hoa_file"]["filename"].as<std::string>();
        VePrint(filename);
        pa = parse_aut(filename, spot::make_bdd_dict());

        if (pa->format_errors(std::cerr))
        {
            ErrorPrintNReturn(HOA_FORMAT_ERROR);
        }
        if (pa->aborted)
        {
            std::cerr << "--ABORT-- read\n";
            ErrorPrintNReturn(HOA_PARSE_ABORT_ERROR);
        }
        aut = pa->aut;
    }
    else if (node["monitor_generate_module"]["open_ltl_file"]["enabled"].as<bool>() == true)
    {
        INFOPrint("Enter open_ltl_file module");
        filename = node["monitor_generate_module"]["open_ltl_file"]["enabled"].as<std::string>();
        std::string fileFormat = node["monitor_generate_module"]["open_ltl_file"]["fileformat"].as<std::string>();
        parse_ltl_file(filename, fileFormat);
    }
    else if (node["monitor_generate_module"]["input_ltl_exp"]["enabled"].as<bool>() == true)
    {
        INFOPrint("Enter input ltl exp module");
        std::string ltl_exp = node["monitor_generate_module"]["input_ltl_exp"]["ltl_exp"].as<std::string>();
        spot::parsed_formula pf = spot::parse_infix_psl(ltl_exp);
        std::string outputfilename = node["monitor_generate_module"]["input_ltl_exp"]["outputfilename"].as<std::string>();
        std::string outputImageName = node["monitor_generate_module"]["input_ltl_exp"]["outputImage"].as<std::string>();
        if (pf.format_errors(std::cerr))
        {
            ErrorPrintNReturn(LTL_EXPRESSION_FORMAT_ERROR);
        }
        /*
            Translate LTL formula into a Monitor, form spot/twaalgos/ltl2tgba_fm.hh
        */
        spot::translator trans;
        trans.set_type(spot::postprocessor::Monitor);
        trans.set_pref(spot::postprocessor::Deterministic);
        spot::twa_graph_ptr autmata = trans.run(pf.f);

        aut = autmata;
        std::ofstream mycout(outputfilename);
        std::string dotname = outputfilename.replace(outputfilename.find(".hoa"), 4, ".dot", 4);
        INFOPrint("Output the HOA file of LTL: " + ltl_exp);
        print_hoa(std::cout, autmata) << '\n';
        print_hoa(mycout, autmata) << '\n';
        std::ofstream dotfile(dotname);
        //Print hoa to pdf
        print_dot(dotfile, autmata, "d"); //d is one of options, means origin format of dot.
        dotfile.close();

        /*Make dot file into image like pdf format.*/
        graph_t *g;
        GVC_t *gvc;
        FILE *fp;

        gvc = gvContext();
        fp = fopen(dotname.c_str(), "r");
        g = agread(fp, 0);
        gvLayout(gvc, g, "dot");
        gvRenderFilename(gvc, g, "pdf", outputImageName.c_str()); //Output for pdf format.
        //gvRender(gvc, g, "pdf", )
        gvFreeLayout(gvc, g);
        agclose(g);
        //<<end

        mycout.close();
    }
    else
    {
        ErrorPrintNReturn(YAML_NODE_PARSE_ERROR);
    }

#if ZMQ == 1

    const spot::bdd_dict_ptr &dict = aut->get_dict();

    Monitor monitor_;
    Monitor &monitor = monitor_;

    //读取所有的状态以及接受集，放入Monitor类型的容器中去。
    Parse_automata_to_monitor(monitor, aut, dict);
    monitor.state_number = aut->get_init_state_number(); //全局状态

    /*接受MQ发送过来的字符串*/
    std::string addr = node["server_bind_addr"].as<std::string>(); //改为读取配置文件
    VePrint(addr);
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);

    socket.bind(addr);
    
    INFOPrint("Sever has binded the address");

    while (1)
    {

        zmq::message_t request;
        socket.recv(&request);
        char *event = (char *)request.data();
        Check_json_log_format(event);

        std::string recvlog = event;
        VePrint(recvlog);

        //解析json文件
        //neb::CJsonObject oJson(recvlog);

        cJSON *cj = cJSON_Parse(recvlog.c_str());
        if (!cj)
        {
            INFOPrint("Parse Json Error");
            zmq::message_t reply(3);
            memcpy(reply.data(), "300", 3);
            socket.send(reply);
            return ERROR;
        }
        cJSON *aw = cJSON_GetObjectItem(cj, "eventName");
        std::string accept_word = aw->valuestring;
        cJSON_Delete(aw);
        //oJson.Get("eventName", accept_word);
        VePrint(accept_word);
        if (Check_word_acceptance(aut, monitor, dict, accept_word) == WORD_ACCEPTANCE_WRONG)
        {
            INFOPrint("Wrong Acceptance!");
            zmq::message_t reply(3);
            memcpy(reply.data(), "200", 3);
            socket.send(reply);
            std::cout << recvlog << std::endl;
            errorLog << recvlog << std::endl;
            cJSON_Delete(cj);

            //输出错误日志，把json格式输出。

            ErrorPrintNEXIT_0(WORD_ACCEPTANCE_WRONG);
        }
        sleep(1);
        zmq::message_t reply(3);
        memcpy(reply.data(), "100", 3);
        socket.send(reply);

        cJSON_Delete(cj);
    }

#else

#endif

#else
    /*测试一个monitor是否可检测出输入的行为违规*/
    Test_Check_word_acceptance_01();
    //Test_Check_word_acceptance_02(pa->aut, monitor, dict);

    // Test_splitstr();/*测试splitstr()*/

    // Test_Parse_bstr_to_wordset();/*测试Test_Parse_bstr_to_wordset()*/

    /*Test Communication module*/
    //Test_Commnunication_module_01();
    //Test_Commnunication_module_01(pa->aut, monitor, dict);
    //Test_Communication_module_02();
    //Test_Parse_label_exp_to_RPN();
    //Test_Parse_label_RPN_to_string_sets();
#endif
    FuncEnd();
}

//===================================================================

/*
    RPN means Reverse Polish notation.
*/
std::vector<std::string> Parse_label_exp_to_RPN(std::string &label)
{
    std::stack<std::string> a_stack;
    std::vector<std::string> rpn;
    int startPosition = 0;
    int endPosition = 0;

    /*The label example: "!green & (press | red)"*/
    char *labelstr = (char *)label.c_str();
    VePrint(label.c_str());
    //VePrint(label.length());
    while (endPosition != label.length())
    {
        char ch = *(labelstr + endPosition);
        //VePrint(ch);
        if (ch == '(' || ch == '&' || ch == '|')
        {
            //INFOPrint("Enter 1");
            std::string tmp = "";
            tmp.append(1, ch);
            a_stack.push(tmp);
            endPosition++;
            startPosition = endPosition;
        }
        else if (ch == ' ')
        {
            //INFOPrint("Enter 2");
            //VePrint(startPosition)
            //VePrint(endPosition);

            if (startPosition != endPosition)
            {
                string tmp = label.substr(startPosition, endPosition - startPosition);
                //VePrint(tmp);
                rpn.push_back(tmp);
            }
            endPosition++;
            startPosition = endPosition;
        }
        else if (ch == ')')
        {
            //INFOPrint("Enter 3");
            if (startPosition != endPosition)
            {
                std::string tmp = label.substr(startPosition, endPosition - startPosition);
                //VePrint(tmp);
                rpn.push_back(tmp); //Test the value of RPN?
            }
            endPosition++;
            startPosition = endPosition;

            while (a_stack.top() != "(")
            {
                std::string tmp = a_stack.top();
                a_stack.pop();
                rpn.push_back(tmp);
            }
            a_stack.pop();
        }
        else
        {
            //INFOPrint("Enter 4");

            endPosition++;
        }
    }

    if (endPosition == label.length())
    {
        if (startPosition != endPosition)
        {
            std::string tmp = label.substr(startPosition, endPosition - startPosition);
            VePrint(tmp);
            rpn.push_back(tmp); //Test the value of RPN?
        }
    }
    else
    {
        INFOPrint("PARSE_LABEL_TO_RPN_ERROR");
    }

    while (a_stack.empty() == false)
    {
        std::string tmp = a_stack.top();
        a_stack.pop();
        rpn.push_back(tmp);
    }

    if (rpn.front() == "|" || rpn.front() == "&")
    {
        INFOPrint("PARSE_LABEL_TO_RPN_ERROR");
    }
    return rpn;
}

/**/
std::vector<std::string> Operate_Logic_AND(std::vector<std::string> &strs1, std::vector<std::string> &strs2)
{
    std::vector<std::string> newstrs;

    for (int m = 0; m < strs1.size(); m++)
    {
        for (int n = 0; n < strs2.size(); n++)
        {
            std::string tmp = strs1[m] + " & " + strs2[n];
            newstrs.push_back(tmp);
        }
    }
    return newstrs;
}

std::vector<std::string> Operate_Logic_OR(std::vector<std::string> &strs1, std::vector<std::string> &strs2)
{
    std::vector<std::string> newstrs;

    for (int n = 0; n < strs2.size(); n++)
    {
        strs1.push_back(strs2[n]);
    }
    newstrs = strs1;

    return newstrs;
}

/*Parse the label string */
//Test exmple "(a | b) | (c | d)"
std::vector<std::string> Parse_label_RPN_to_string_sets(std::vector<string> &rpn)
{
    typedef std::vector<std::string> Strvector;
    typedef std::vector<Strvector> Strvectorbucket;

    int m = -1;

    Strvectorbucket strvbu;
    for (int i = 0; i < rpn.size(); i++)
    {
        //VePrintByArg(rpn, i);
        if (rpn[i] != "|" && rpn[i] != "&")
        {
            //ENTERPrint(1);
            Strvector strv1;
            strv1.push_back(rpn[i]);
            strvbu.push_back(strv1);
            m++;
            //VePrint(m);
        }
        else if (rpn[i] == "|")
        {
            //ENTERPrint(2);

            Strvector strv2 = Operate_Logic_OR(strvbu[m - 1], strvbu[m]);
            //VePrintVector(strv2);
            m--;
            //VePrint(m);
            strvbu.pop_back();
            strvbu.pop_back();
            strvbu.push_back(strv2);
        }
        else if (rpn[i] == "&")
        {
            //ENTERPrint(3);
            //立即进行新字符串的合成。产生 a & b 类型的字符串。
            Strvector strv3 = Operate_Logic_AND(strvbu[m - 1], strvbu[m]);
            //VePrintVector(strv3);
            m--;
            //VePrint(m);
            strvbu.pop_back();
            strvbu.pop_back();
            strvbu.push_back(strv3);
        }
    }
    //Print the a_stack
    VePrintVector(strvbu[0]);
    if (m != 0)
    {
        ERRORPrint("Parse_label_RPN_to_string_sets Wrong");
    }
    FuncEnd();
    return strvbu[0];
}
/*
功能：将一个只含有 & 运算符的布尔表达式形式的字符串解析到Word_set结构体中。
*/
int Parse_bstr_to_wordset(std::string &str, Word_set &word_set)
{
    //FuncBegin();
    //按空格分割, 分别加入map中,如果包含！,则值为0，否则为1
    stringList sli = splitstr(str, ' ');

    //将解析到的结构放入
    word_set.word = str;
    for (auto &t : sli)
    {
        //如果不包含"!"
        if (t[0] != '!')
        {
            word_set.wordset[t] = 1;
        }
        else
        {
            word_set.wordset[t] = 0; //test a time thing is very good thing.
        }
    }
    //FuncEnd();
    return SUCCESS;
}
/*
Function: Parse the label to vector<Word_set>
*/
int Parse_label_to_word_sets(std::string &label, std::vector<Word_set> &word_sets)
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

    for (auto &t : label_set)
    {
        Word_set ws;
        Parse_bstr_to_wordset(t, ws);
        word_sets.push_back(ws);
    }

    /*
    stringList strli = splitstr(label, '|');

        for (auto &t : strli)
        {
            Word_set ws_;
            Word_set &ws = ws_;
            Parse_bstr_to_wordset(t, ws);
            word_sets.push_back(ws);
*/
    std::cout <<BOLDBLUE <<"Parse label " << label << " SUCCESS" <<RESET<< std::endl;
    FuncEnd();
    return SUCCESS;
}
//===================================================================
/*
功能：解析后的自动机的状态信息被放入结构体中
输入： 
输出：
*/
int Parse_automata_to_monitor(Monitor &monitor, spot::twa_graph_ptr &aut, const spot::bdd_dict_ptr &dict)
{
    int num_state = 0;

    FuncBegin();
    for (num_state; num_state < aut->num_states(); ++num_state)
    {
        Monitor_state monitor_state;

        monitor_state.own_state = num_state;
        for (auto &t : aut->out(num_state))
        {
            Monitor_label monitor_label;
            if (t.src != num_state)
            {
                std::cout << "src != num_state, There is wrong" << std::endl;
                return ERROR;
            }

            monitor_label.label = spot::bdd_format_formula(dict, t.cond);

            VePrint(monitor_label.label);
            //Test_bdd_print(dict, t.cond);
            monitor_label.next_state = t.dst;

            //把表示逻辑运算的字符串解析放入wordset数据结构中
            if (Parse_label_to_word_sets(monitor_label.label, monitor_label.word_sets) != SUCCESS)
            {
                AMReturn(ERROR);
            }
            monitor_label.strlist = splitstr(monitor_label.label, ' ');
            VePrint(monitor_label.strlist[0]);
            monitor_state.monitor_labels.push_back(monitor_label);
        }
        monitor_state.label_numbers = monitor_state.monitor_labels.size();
        monitor.nodes[num_state] = monitor_state;
    }

    FuncEnd();
    return SUCCESS;
}

/*
功能：输出自动机为文本格式
*/

/*
功能： 检测输入的布尔表达式是否违规 ,比如 a & !a
输入：
输出：
*/
int check_accept_word_format(std::string accept_word)
{
    /*检查 a & !a的情形*/
    /*检查 a & a的情形*/
    //切分, 找重复的元素,存在就失败,不存在就成功
    //每个字符串的第一个位置可以为 ！， 其余只能出现字母和数字
}

int Parse_acceptword_to_wordset(std::string &accept_word, Word_set &word_set)
{
    //FuncBegin();
    //按空格分割, 分别加入map中,如果包含！,则值为0，否则为1
    stringList sli = splitstr(accept_word, ' ');
    //如果出现了bug

    //将解析到的结构放入
    word_set.word = accept_word;
    for (auto t : sli)
    {
        //考虑到了 a & !a 的情况
        //VePrint(t);
        if (t.find("!") == std::string::npos)
        {
            //Not contain "!"
            if (word_set.wordset.find(t) == word_set.wordset.end())
            {
                word_set.wordset[t] = 1;
            }
            else
            {
                ErrorPrintNReturn(ACCEPT_WORD_FORMAT_WRONG);
            }
        }
        else if (t.find("!") == 0)
        {
            //Contain "!"
            std::string tmp = t.substr(1, t.length() - 1);
            //VePrint(tmp);
            if (word_set.wordset.find(tmp) == word_set.wordset.end())
            {
                word_set.wordset[tmp] = 0;
            }
            else
            {
                ErrorPrintNReturn(ACCEPT_WORD_FORMAT_WRONG);
            }
        }
        else
        {
            ErrorPrintNReturn(PARSE_ACCEPTEORD_TO_WORDSET_ERROR);
        }
    }
    //FuncEnd();
    return SUCCESS;
}

/*
功能： 检测字符串是否匹配
*/
int label_match_word(Monitor_label &monitor_label, std::string accept_word)
{
    //解析accept_word,放入word_set中
    Word_set wset;

    if (Parse_acceptword_to_wordset(accept_word, wset) == ACCEPT_WORD_FORMAT_WRONG)
    {
        ERRORPrint("ACCEPT_WORD_FORMAT_WRONG");
        exit(0);
    }

    //比较wset中的字符是否出现在monitor_label中,
    for (auto &word_set : monitor_label.word_sets)
    {
        //如果accept_word的每个字符在word_set中都存在，则判为成功
        std::map<std::string, size_t>::iterator iter;

        for (iter = wset.wordset.begin(); iter != wset.wordset.end(); iter++)
        {
            if (word_set.wordset.find(iter->first) != word_set.wordset.end() &&
                word_set.wordset[iter->first] == wset.wordset[iter->first])
            {
                continue;
            }
            else
            {
                INFOPrint("not match");
                return NOMATCH;
            }
        }
    }
    return SUCCESS;
}
/*
int label_match_word(Monitor_label &monitor_label, std::string accept_word)
{
    for (auto str : monitor_label.strlist)
    {
        //假如，存在red,且不存在!red,才能够说明该str被accept_word包含了
        if (accept_word.find(str) != accept_word.npos && accept_word.find("!" + str) == accept_word.npos)
        {
            continue;
        }
        else
        {
            return NOMATCH;
        }
    }
    return SUCCESS;
}
*/

/*
功能： 检测输入的字是否符合Monitor要求。
输入： 输入一个字
输入的是Monitorstate
*/
int Check_word_acceptance(spot::twa_graph_ptr &aut,
                          Monitor &monitor, const spot::bdd_dict_ptr &dict, std::string accept_word)
{
    FuncBegin();

    //读取初始状态
    state_number = monitor.state_number;
    while (1)
    {
        //检测字是否符合
        if (monitor.nodes[state_number].own_state != state_number)
        {
            return ERROR;
        }
        int i = 1;

        for (auto &monitor_label : monitor.nodes[state_number].monitor_labels)
        {
            //如果满足label，则更新state_number,
            //VePrint(state_number);
            VePrint(monitor_label.label);
            VePrint(accept_word);

            if (label_match_word(monitor_label, accept_word) == SUCCESS)
            {
                monitor.state_number = monitor_label.next_state; //更新Monitor的全局状态
                INFOPrint("Accepted!");
                VePrint(monitor.state_number);
                FuncEnd();
                return SUCCESS; //这里需要优化
            }
            else if (label_match_word(monitor_label, accept_word) != SUCCESS &&
                     i < monitor.nodes[state_number].label_numbers)
            {
                i++;
                INFOPrint("Try next label\n");
                continue;
            }
            else
            {
                INFOPrint("\nAccepted Failed");
                std::cout << BOLDRED << "The label is \"" << monitor_label.label << RESET << std::endl;
                std::cout << BOLDRED << "The accepted word is \"" << accept_word << "\"" << RESET << std::endl;
                FuncEnd();
                return WORD_ACCEPTANCE_WRONG;
            }
        }
    }
}

//============================================================
//Test Units//================================================
//============================================================

/*
功能： 测试Check_word_acceptance函数是否能够检测输入的字符串
*/
int Test_Check_word_acceptance_01()
{

    FuncBegin();
    //预定义的字符串格式
    //std::string teststr[] = {"red & !yellow", "red", "!red & yellow", "red & !yellow", "!red"};
    std::string teststr[] = {"red & !red", "red & !yellow", "red", "!red & yellow", "red & !yellow", "!red"};
    std::string filename = "demo.hoa";
    //产生的自动机
    const spot::bdd_dict_ptr &dict = spot::make_bdd_dict();
    spot::parsed_aut_ptr pa = parse_aut(filename, dict);
    if (pa->format_errors(std::cerr))
    {
        ErrorPrintNReturn(HOA_FORMAT_ERROR);
    }
    if (pa->aborted)
    {
        std::cerr << "--ABORT-- read\n";
        ErrorPrintNReturn(HOA_PARSE_ABORT_ERROR);
    }
    spot::twa_graph_ptr aut = pa->aut;
    Monitor monitor;
    Parse_automata_to_monitor(monitor, aut, dict);
    //将字符串输入自动机中去
    for (std::string to_be_tested_str : teststr)
    {
        VePrint(to_be_tested_str);
        if (Check_word_acceptance(aut, monitor, dict, to_be_tested_str) == WORD_ACCEPTANCE_WRONG)
        {
            INFOPrint("Accepted Failed");
            FuncEnd();
            ErrorPrintNReturn(WORD_ACCEPTANCE_WRONG);
        }
        else
        {
            INFOPrint("Check SUCCESS");
        }
    }
    INFOPrint("teststr is compished");

    FuncEnd();
}

/*
功能： 测试bddprint的功能。
*/
int Test_bdd_print(const spot::bdd_dict_ptr &dict, bdd b)
{
    FuncBegin();
    std::cout << "bdd_format_set: " << bdd_format_set(dict, b) << std::endl;
    std::cout << "bdd_format_isop: " << bdd_format_isop(dict, b) << std::endl;
    std::cout << "bdd_format_accset: " << bdd_format_accset(dict, b) << std::endl;
    std::cout << "bdd_format_sat: " << bdd_format_sat(dict, b) << std::endl;
    FuncEnd();
}
/*
功能：随机字生成,可以产生特定的随机字序列,用于输入到自动机中。
*/

/*
功能： 随机Monitor生成, 输入的字符串大致符合自动机,但在某一处随机产生了一个故意的错误
*/

/*
功能： 测试  ltl2tgba -D 'Ga|Gb|Gc' -d 
        文件名为demo2
*/
int Test_Check_word_acceptance_02(spot::twa_graph_ptr &aut,
                                  Monitor &monitor, const spot::bdd_dict_ptr &dict)
{

    FuncBegin();
    //预定义的字符串格式
    //std::string teststr[] = {"a & b & c", "!a & b & c", "b & !c", "b", "b",
    //                      "b", "b", "b", "b", "b", "b", "b", "b", "c", "b"};

    std::string teststr[] = {"a & !b & c & d ", "!a & b & c", "b & !c", "b", "b",
                             "b", "b", "b", "b", "b", "b", "b", "b", "c", "b"};

    //产生的自动机

    //将字符串输入自动机中去
    while (1)
    {
        for (std::string str : teststr)
        {
            if (Check_word_acceptance(aut, monitor, dict, str) == WORD_ACCEPTANCE_WRONG)
            {
                INFOPrint("Accepted Failed");
                FuncEnd();
                return WORD_ACCEPTANCE_WRONG;
            }
        }
    }
    FuncEnd();
}

/*
功能： 测试Parse_bstr_to_wordset()
*/
int Test_Parse_bstr_to_wordset()
{
    FuncBegin();

    std::string str = "!red & yellow & green & !pink";

    Word_set word_set_;
    Word_set &word_set = word_set_;

    std::map<std::string, size_t>::iterator iter; //遍历器

    if (Parse_bstr_to_wordset(str, word_set) != SUCCESS)
    {
        FuncEnd();
        AMReturn(ERROR);
    }
    else
    {
        VePrint(word_set.word);
        //遍历word_set
        iter = word_set.wordset.begin();
        while (iter != word_set.wordset.end())
        {
            std::cout << BOLDRED << iter->first << "--" << iter->second << RESET << std::endl;
            iter++;
        }
    }

    //Parse_bstr_to_wordset(str, word_set);

    FuncEnd();

    return SUCCESS;
}

/*
功能： 测试splitstr()函数功能
*/
static int Test_splitstr()
{
    FuncBegin();

    std::string str1 = "!red & yellow & green & !pink";
    stringList sli = splitstr(str1, ' ');

    for (auto &t : sli)
    {
        std::cout << "sli is : " << t << "\n";
    }
    //Parse_bstr_to_wordset(str, word_set);

    FuncEnd();
    return SUCCESS;
}

/*
功能：测试 label_match_word函数
*/

/*检查输入违规字符串的情形*/

/*
brief\Test the module of Communication
*/
int Test_Commnunication_module_01(spot::twa_graph_ptr &aut, Monitor &monitor, const spot::bdd_dict_ptr &dict)
{
    FuncBegin();

    //通信得来的字符串作为输入

    std::string addr = "tcp://*:25555"; //自行修改
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);

    socket.bind(addr);
    INFOPrint("Sever has binded the address");

    while (1)
    {

        zmq::message_t request;
        socket.recv(&request);
        std::string accpet_word = (char *)request.data();
        VePrint(accpet_word);
        if (Check_word_acceptance(aut, monitor, dict, accpet_word) == WORD_ACCEPTANCE_WRONG)
        {
            INFOPrint("wrong acceptance");
            return WORD_ACCEPTANCE_WRONG;
        }
        sleep(1);

        zmq::message_t reply(3);
        memcpy(reply.data(), "200", 3);
        socket.send(reply);
    }

    FuncEnd();
}

/*
brief\ 测试zmq 以及 demo3
*/
int Test_Communication_module_02()
{
    FuncBegin();

    spot::parsed_aut_ptr pa = parse_aut("demo3.hoa", spot::make_bdd_dict());

    if (pa->format_errors(std::cerr))
        return ERROR;
    if (pa->aborted)
    {
        std::cerr << "--ABORT-- read\n";
        return ERROR;
    }

    const spot::bdd_dict_ptr &dict = pa->aut->get_dict();

    Monitor monitor_;
    Monitor &monitor = monitor_;

    //读取所有的状态以及接受集，放入Monitor类型的容器中去。
    Parse_automata_to_monitor(monitor, pa->aut, dict);
    monitor.state_number = pa->aut->get_init_state_number(); //全局状态

    std::string addr = "tcp://*:25555"; //改为读取配置文件
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);

    socket.bind(addr);
    INFOPrint("Sever has binded the address");

    while (1)
    {

        zmq::message_t request;
        socket.recv(&request);
        std::string accpet_word = (char *)request.data();
        VePrint(accpet_word);
        if (Check_word_acceptance(pa->aut, monitor, dict, accpet_word) == WORD_ACCEPTANCE_WRONG)
        {
            INFOPrint("Wrong Acceptance!");
            zmq::message_t reply(3);
            memcpy(reply.data(), "200", 3);
            socket.send(reply);

            return WORD_ACCEPTANCE_WRONG;
        }
        sleep(1);

        zmq::message_t reply(3);
        memcpy(reply.data(), "200", 3);
        socket.send(reply);
    }

    FuncEnd();
}

int Test_Parse_label_exp_to_RPN()
{
    FuncBegin();
    std::string label = "(event1 & event3) | (!event4 & event2) | event5";
    std::string label2 = "!green & (press | red)";
    std::string label3 = "( event1 & event3 ) | ( !event4 & event2) | event5";
    std::string label4 = "(a | b) | (c | d)";
    std::string label5 = "(!a & !b)|(!a & c)";

    std::vector<std::string> rpn;
    rpn = Parse_label_exp_to_RPN(label3);

    /*Print the rpn*/
    INFOPrint("Print the RPN's elements");
    for (int i = 0; i < rpn.size(); i++)
    {
        std::cout << rpn[i] << ",";
    }
    std::cout << "\n";

    return SUCCESS;
    FuncEnd();
}

int Test_Parse_label_RPN_to_string_sets()
{
    FuncBegin();
    std::string label = "(event1 & event3) | (!event4 & event2) | event5";
    std::string label2 = "!green & (press | red)";
    std::string label3 = "( event1 & event3 ) | ( !event4 & event2) | event5";
    std::string label4 = "(a | b) | (c | d)";
    std::string label5 = "(!a & !b)|(!a & c)";

    std::vector<std::string> rpn;
    rpn = Parse_label_exp_to_RPN(label3);

    /*Print the rpn*/
    INFOPrint("Print the RPN's elements");
    for (int i = 0; i < rpn.size(); i++)
    {
        std::cout << rpn[i] << ",";
    }
    std::cout << "\n";

    INFOPrint("Now test this funcion:");
    Parse_label_RPN_to_string_sets(rpn);

    return SUCCESS;
    FuncEnd();
}