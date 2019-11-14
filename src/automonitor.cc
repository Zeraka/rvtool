#include <iostream>
#include <ostream>
#include <sstream>
#include <cstring>
#include <string>

#include <cstdio>

#include <spot/tl/parse.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twa/bddprint.hh>
#include <spot/parseaut/public.hh>

#include <zmq.hpp>
#include "automonitor.hh"
#include "util-base.hh"
#include "util-debug.hh"
#include "server.hpp"

static int Test_splitstr();

int main(void)
{
    FuncBegin();
    //读取并解析自动机文件
    //spot::parsed_aut_ptr pa = parse_aut("demo.hoa", spot::make_bdd_dict());
    spot::parsed_aut_ptr pa = parse_aut("demo2.hoa", spot::make_bdd_dict());

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

    /*接受MQ发送过来的字符串*/
    //建立通信
#if ZMQ == 1
    std::string addr = "tcp://*:25555";
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
        //为什么速度这么慢呢
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


#else

#endif

#if Test_AUTOMONITOR == 0


#else
    /*测试一个monitor是否可检测出输入的行为违规*/
    //Test_Check_word_acceptance_01(pa->aut, monitor, dict);
    //Test_Check_word_acceptance_02(pa->aut, monitor, dict);
    /*测试splitstr()*/
    // Test_splitstr();

    /*测试Test_Parse_bstr_to_wordset()*/
    // Test_Parse_bstr_to_wordset();

    /*Test Communication module*/
    //Test_Commnunication_module_01();
    Test_Commnunication_module_01(pa->aut, monitor, dict);
#endif
    FuncEnd();
}

/*
功能： 将一个只含有 & 运算符的布尔表达式形式的字符串解析到 结构体中。
输入：
输出：
*/
int Parse_BoolString_to_set(std::string str, Word_set &word_set,
                            const spot::bdd_dict_ptr &dict)
{
    //将
}

/*
功能： 字符串检查
    不可能出现 !a & a 的情况。
*/

/*
功能：将一个只含有 & 运算符的布尔表达式形式的字符串解析到Word_set结构体中。
*/
int Parse_bstr_to_wordset(std::string str, Word_set &word_set)
{
    //FuncBegin();
    //按空格分割, 分别加入map中,如果包含！,则值为0，否则为1
    stringList sli = splitstr(str, ' ');

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
            word_set.wordset[t] = 0;
        }
    }
    //FuncEnd();
    return SUCCESS;
}

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

        monitor_state.current_state = num_state;
        for (auto &t : aut->out(num_state))
        {
            Monitor_label monitor_label;
            if (t.src != num_state)
            {
                std::cout << "src != num_state, There is wrong" << std::endl;
                return ERROR;
            }

            monitor_label.label = spot::bdd_format_formula(dict, t.cond);

            //VePrint(monitor_label.label);
            //Test_bdd_print(dict, t.cond);
            monitor_label.next_state = t.dst;

            if (Parse_bstr_to_wordset(monitor_label.label, monitor_label.word_set) != SUCCESS)
                AMReturn(ERROR);
            monitor_label.strlist = splitstr(monitor_label.label, ' ');
            //VePrint(monitor_label.strlist[0]);
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
}

/*
功能： 检测字符串是否匹配
*/
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
        if (monitor.nodes[state_number].current_state != state_number)
        {
            return ERROR;
        }
        int i = 1;

        for (auto &monitor_label : monitor.nodes[state_number].monitor_labels)
        {
            //如果满足label，则更新state_munber,
            //VePrint(state_number);
            //VePrint(monitor_label.label);
            //VePrint(accept_word);

            if (label_match_word(monitor_label, accept_word) == SUCCESS)
            {
                monitor.state_number = monitor_label.next_state; //更新Monitor的全局状态
                //VePrint()
                //INFOPrint("Accepted!");
                //VePrint(monitor.state_number);
                FuncEnd();
                return SUCCESS; //这里需要优化
            }
            else if (label_match_word(monitor_label, accept_word) != SUCCESS &&
                     i < monitor.nodes[state_number].label_numbers)
            {
                i++;
                //INFOPrint("Try next label\n");
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
int Test_Check_word_acceptance_01(spot::twa_graph_ptr &aut,
                                  Monitor &monitor, const spot::bdd_dict_ptr &dict)
{

    FuncBegin();
    //预定义的字符串格式
    //std::string teststr[] = {"red & !yellow", "red", "!red & yellow", "red & !yellow", "!red"};
    std::string teststr[] = {"red & !red", "red & !yellow", "red", "!red & yellow", "red & !yellow", "!red"};

    //产生的自动机

    //将字符串输入自动机中去
    for (std::string str : teststr)
    {
        VePrint(str);
        if (Check_word_acceptance(aut, monitor, dict, str) == WORD_ACCEPTANCE_WRONG)
        {
            INFOPrint("Accepted Failed");
            FuncEnd();
            return WORD_ACCEPTANCE_WRONG;
        }
    }

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
功能： 测试
*/

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
        //为什么速度这么慢呢
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
