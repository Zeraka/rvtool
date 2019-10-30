#include <iostream>
#include <ostream>
#include <strstream>
#include <sstream>

#include <string>

#include <spot/tl/parse.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twa/bddprint.hh>
#include <spot/parseaut/public.hh>
#include <spot/parseaut/parseaut.hh>


#include "automonitor.hh"
#include "util-base.h"
#include "util-debug.h"



int main(void)
{
    //检测自动机文件，如果有F(a)出现,则警告(当优先级为高的时候)

    //读取并解析自动机文件
    spot::parsed_aut_ptr pa = parse_aut("demo.hoa", spot::make_bdd_dict());

    if (pa->format_errors(std::cerr))
        return ERROR;
    if (pa->aborted)
    {
        std::cerr << "--ABORT-- read\n";
        return ERROR;
    }

    int initial_state = 0;
    int current_state = 0;
    std::ostrstream label_ ;
    std::ostrstream& label = label_;

    const spot::bdd_dict_ptr &dict = pa->aut->get_dict();

    Monitor monitor_ ;
    Monitor& monitor = monitor_;

    //读取所有的状态以及接受集，放入Monitor类型的容器中去。
    Parse_automata_to_map(monitor, pa->aut, dict);


    //接受MQ发送过来的字符串

    //check_word_acceptance(label, pa->aut, monitor, dict, );

    //测试是否可以用
    test_check_word_acceptance(label,pa->aut, monitor, dict);
}


/*
功能：解析后的自动机的状态信息被放入结构体中
输入： 
输出：
*/
int Parse_automata_to_map(Monitor& monitor, spot::twa_graph_ptr &aut, const spot::bdd_dict_ptr &dict)
{
    int num_state = 0;

    std::ostrstream label;
    for(num_state; num_state < aut->num_states; ++num_state)
    {   
        Monitor_state monitor_state = {0};


        monitor_state.current_state = num_state;
        for(auto& t :aut->out(num_state))
        {
            Monitor_label monitor_label = {0};

            if(t.src != num_state)
            {
                std::cout << "src != num_state, There is wrong" << std::endl;
                return ERROR;
            }   

            spot::bdd_print_formula(label, dict, t.cond);
            monitor_label.label = label.str();
            monitor_label.next_state = t.dst;
            monitor_state.monitor_labels.push_back(monitor_label);
        }
    
        monitor[num_state] = monitor_state;        
    }


    return SUCCESS;
}


/*
功能： 检测输入的字是否符合Monitor要求。
输入： 输入一个字
输入的是Monitorstate
*/
int check_word_acceptance(std::ostrstream &label, spot::twa_graph_ptr &aut,
                          Monitor& monitor, const spot::bdd_dict_ptr &dict, std::string accept_word)
{

        int state_number = 0;

        //读取初始状态
        state_number = aut->get_init_state_number();

        while(1){
            //检测字是否符合
            if(monitor[state_number].current_state != state_number)
            {
                return ERROR;
            }

            //迭代
            for(auto& t : monitor[state_number].monitor_labels)
            {
                int i = 0;
                //如果满足label，则更新state_munber,
                if(t.label == accept_word)
                {
                    state_number = t.next_state;
                    break;
                }
                //如果所有label都不满足，则报错。
                if(monitor[state_number].monitor_labels.end)
            }

        }

    return SUCCESS;
}

//============================================================
//Test Units//================================================
//============================================================

/*
功能： 测试check_word_acceptance函数是否能够检测输入的字符串
*/
void test_check_word_acceptance(std::ostrstream &label, spot::twa_graph_ptr& aut, 
                                Monitor &monitor, const spot::bdd_dict_ptr &dict){


    //预定义的字符串格式
    std::string teststr[] = {"c", "!a&c","a"};

    //产生的自动机

    //将字符串输入自动机中去
    for(std::string str:teststr)
    {
        check_word_acceptance(label, aut, monitor, dict, str);
    }
}


