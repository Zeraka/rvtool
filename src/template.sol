pragma solidity >=0.4.22 <0.6.0;

contract monitor_automata {
    struct Word_set {
        string word;
        mapping(string => uint256) wordset;
    }

    struct Monitor_label {
        string label;
        string[] strlist;
        Word_set[] word_sets;
        int256 next_state;
    }

    struct Monitor_state {
        int256 own_state;
        int256 label_numbers;
        Monitor_label[] monitor_labels;
    }

    struct Monitor {
        int256 state_number;
        mapping(uint256 => Monitor_state) nodes;
    }
    
    Monitor monitor;
    Monitor_state monitor_state_0;
    Monitor_label monitor_label_0;
    Word_set ws_0;
    Monitor_label monitor_label_1;
    Word_set ws_1;
    Monitor_state monitor_state_1;
    Monitor_label monitor_label_2;
    Word_set ws_2;

    function Monitor_init() public {
        //=================

        monitor_state_0.own_state = 0;

        monitor_label_0.label = "!event3";
        monitor_label_0.next_state = 0;

        ws_0.word = "!event3";
        ws_0.wordset["event3"] = 0;
        monitor_label_0.word_sets.push(ws_0);

        monitor_state_0.monitor_labels.push(monitor_label_0);

        monitor_label_1.label = "event3";
        monitor_label_1.next_state = 1;

        ws_1.word = "event3";
        ws_1.wordset["event3"] = 1;
        monitor_label_1.word_sets.push(ws_1);

        monitor_state_0.monitor_labels.push(monitor_label_1);
        monitor_state_0.label_numbers = 2;
        monitor.nodes[0] = monitor_state_0;
        //=================

        monitor_state_1.own_state = 1;

        monitor_label_2.label = "!event1 & !event3 & event4";
        monitor_label_2.next_state = 0;

        ws_2.word = "!event1 & !event3 & event4";
        ws_2.wordset["event1"] = 0;
        ws_2.wordset["event3"] = 0;
        ws_2.wordset["event4"] = 1;
        monitor_label_2.word_sets.push(ws_2);

        monitor_state_1.monitor_labels.push(monitor_label_2);
        monitor_state_1.label_numbers = 1;
        monitor.nodes[1] = monitor_state_1;
    }

    //将输入的字符串解析为数据结构。然后将其
}
