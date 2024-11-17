#include <cstring>
#include "JQ.hpp"

JQ::JQ() : hash(0), depth(0), type(JQ_NO_VAL), op(JQ_NOT_SET) {}

JQ::JQ(struct jqpath &val)
    : hash(val.hash), depth(val.depth), type(val.value.type), op(val.op) {
    switch (type) {
    case JQ_NO_VAL:
        break;
    case JQ_INT_VAL:
        this->val.int_val = val.value.value.int_val;
        break;
    case JQ_FLOAT_VAL:
        this->val.float_val = val.value.value.float_val;
        break;
    case JQ_STRING_VAL:
        this->val.string_val = copy_string(val.value.value.string_val);
        break;
    }
}

JQ::~JQ() {
    if (type == JQ_STRING_VAL) {
        kill_string(val.string_val);
    }
}

bool JQ::operator==(struct jqpath &path) {
    if (path.hash == hash) {
        if (path.depth == depth) {
            if (path.op != JQ_NOT_SET && path.value.type == type) {
                switch (path.value.type) {
                case JQ_INT_VAL:
                    return path.value.value.int_val == val.int_val;
                case JQ_FLOAT_VAL:
                    return path.value.value.float_val == val.float_val;
                case JQ_STRING_VAL:
                    return cmp_string(path.value.value.string_val,
                                      val.string_val);

                default:
                    break;
                }
            } else {
                return true;
            }
        }
    }

    return false;
}

bool JQ::operator==(int val) {
    if(this->type == JQ_INT_VAL) {
        return this->val.int_val == val;
    } 

    return false;
}

bool JQ::operator==(float val) {
    if(this->type == JQ_FLOAT_VAL) {
        return this->val.float_val == val;
    }

    return false;
}

bool JQ::operator==(const char * str) {
    if(this->type == JQ_STRING_VAL) {
        return std::strcmp(this->val.string_val,str) == 0;
    }

    return false;
}

bool JQ::operator==(std::string str) {
    if(this->type == JQ_STRING_VAL) {
        return str == this->val.string_val;
    }

    return false;
}
