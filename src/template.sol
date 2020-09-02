/*
solidity代码中结构体可以用到
*/
contract monitor_automata{
    struct word_set{
        string word;
        mapping (string=>uint) wordset;
    }

    struct Monitor_label{
        string label;
        string[] strlist;//动态数组
        word_set[] word_sets;
        int next_state;
    }
    struct Monitor_state{
        int own_state;
        int label_numbers;
        Monitor_label[] monitor_labels;//结构体数组
    }
    struct Monitor{
        int state_number;
        mapping(uint=>Monitor_state) nodes;
    }

    //声明一个Monitor对象
    Monitor monitor;
    
    word_set[] acc_word_sets;
    //只写什么函数
    Monitor_state monitor_state;

    //该函数用于将
    function add_monitor_label()  private returns(){
        
    }

    function Monitor_init() public{
        monitor_state.own_state = 0;

        //声明一个monitor_label
        //push(add_monitor_label());
        add_monitor_label();
    }
    
    //接收外来字符，进行检测
}