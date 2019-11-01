#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

enum{
    SUCCESS = 0,
    ERROR ,
    WORD_ACCEPTANCE_WRONG
};

#define FuncBegin() {                                   \
    std::cout << "Function "<<__FUNCTION__  << " begin" << std::endl;  \
}

#define FuncEnd(){                                  \
    std::cout <<"Function "<< __FUNCTION__  << " end" << std::endl;   \
}


#endif