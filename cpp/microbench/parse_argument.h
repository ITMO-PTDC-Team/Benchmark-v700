//
// Created by Ravil Galiev on 06.04.2023.
//

#ifndef SETBENCH_PARSE_ARGUMENT_H
#define SETBENCH_PARSE_ARGUMENT_H

struct ParseArgument {
    size_t pointer;
    size_t length;
    char **args;

    ParseArgument(size_t _length, char **_args)
            : length(_length), args(_args), pointer(0) {}

    char *getCurrent() {
        return args[pointer];
    }

    char *getNext() {
        return args[++pointer];
    }

    ParseArgument &next() {
        ++pointer;
        return *this;
    }

    bool hasNext() {
        return pointer < length;
    }
};

#endif //SETBENCH_PARSE_ARGUMENT_H
