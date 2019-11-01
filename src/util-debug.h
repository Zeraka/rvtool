#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

enum{
    SUCCESS = 0,
    ERROR ,
    WORD_ACCEPTANCE_WRONG
};

#define FuncBegin() {                                   \
    std::cout << "\nFunction "<<__FUNCTION__  << " begin" << std::endl;  \
}

#define FuncEnd(){                                  \
    std::cout <<"Function "<< __FUNCTION__  << " end\n" << std::endl;   \
}

#define FuncEnd_print(x){                                  \
    std::cout <<"Function "<< __FUNCTION__  << " end\n" << std::endl;   \
    std::cout << x << std::endl;                                             \
}


#endif