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
    Monitor public monitor;

    word_set[] public acc_word_sets;
    //word_sets是用于什么的？
    
    //只写什么函数
    Monitor_state monitor_state;


    function Monitor_init() public{
        //最开始将其完成的内容是什么
        
        monitor_state.own_state = 0;

  
    }
    
}