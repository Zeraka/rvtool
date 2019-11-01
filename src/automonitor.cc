#include <iostream>
#include <ostream>
#include <sstream>
#include <cstring>
#include <string>

#include <spot/tl/parse.hh>
#include <spot/twaalgos/translate.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twa/bddprint.hh>
#include <spot/parseaut/public.hh>

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

    const spot::bdd_dict_ptr &dict = pa->aut->get_dict();

    Monitor monitor_ ;
    Monitor& monitor = monitor_;
    //读取所有的状态以及接受集，放入Monitor类型的容器中去。
    Parse_automata_to_map(monitor, pa->aut, dict);

    //接受MQ发送过来的字符串

    //check_word_acceptance(label, pa->aut, monitor, dict, );

    //测试是否可以用
    test_check_word_acceptance_01(pa->aut, monitor, dict);
}


/*
功能：解析后的自动机的状态信息被放入结构体中
输入： 
输出：
*/
int Parse_automata_to_map(Monitor& monitor, spot::twa_graph_ptr &aut, const spot::bdd_dict_ptr &dict)
{
    int num_state = 0;

    printf("Parse_automata_to_map begin;\n");

    for(num_state; num_state < aut->num_states(); ++num_state)
    {   
        Monitor_state monitor_state ;

        monitor_state.current_state = num_state;
        for(auto& t :aut->out(num_state))
        {
            Monitor_label monitor_label ;
            if(t.src != num_state)
            {
                std::cout << "src != num_state, There is wrong" << std::endl;
                return ERROR;
            }   
            
            monitor_label.label = spot::bdd_format_formula(dict, t.cond);
            std::cout << "label is \"" << spot::bdd_format_formula(dict, t.cond)<< "\"" << std::endl;
            std::cout << "label is \"" << monitor_label.label<< "\"" << std::endl;
            monitor_label.next_state = t.dst;
            monitor_state.monitor_labels.push_back(monitor_label);
        }
    
        monitor[num_state] = monitor_state;        
    }

    printf("Parse_automata_to_map end;\n\n");
    return SUCCESS;
}


/*
功能： 检测输入的字是否符合Monitor要求。
输入： 输入一个字
输入的是Monitorstate
*/
int check_word_acceptance( spot::twa_graph_ptr &aut,
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
            int i = 0;

            std::cout << "length of vector" << monitor[state_number].monitor_labels.size() <<std::endl;
            //迭代,如果都满足，则使得
            for(auto& t : monitor[state_number].monitor_labels)
            {
                printf("dddddddddd\n");
                //如果满足label，则更新state_munber,
                std::cout << "t.label :\""<<t.label <<"\"" << std::endl;
                std::cout << "accpet_word :\"" << accept_word<< "\"" << std::endl;

                if(t.label == accept_word)
                {
                    state_number = t.next_state;
                    break;
                }
                //如果所有label都不满足，则报错。
                ++i;
                if(i > 2)
                {
                    std::cout << "wrong" << std::endl;
                    break;
                    return ERROR;
                }
                
            }
        return 0;
        }

    return SUCCESS;
}

//============================================================
//Test Units//================================================
//============================================================

/*
功能： 测试check_word_acceptance函数是否能够检测输入的字符串
*/
void test_check_word_acceptance_01( spot::twa_graph_ptr& aut, 
                                Monitor &monitor, const spot::bdd_dict_ptr &dict){


    //预定义的字符串格式
    std::string teststr[] = {"c", "!a&c","a"};

    //产生的自动机

    //将字符串输入自动机中去
    for(std::string str:teststr)
    {
        printf("aaaaaaaaaaa\n");
        check_word_acceptance( aut, monitor, dict, str);
    }
}

