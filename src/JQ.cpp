#include "JQ.hpp"
#include "jsmn.hpp"
#include <cstring>
#include <string>


JQ::JQ()
    : hash(0), 
      depth(0), 
      rendered(false), 
      sp{0, 0}, 
      type(JQ_NO_VAL),
      op(JQ_NOT_SET),
      tk(nullptr),
      js(nullptr) {}

JQ::JQ(struct jqpath &val)
    : hash(val.hash), 
      depth(val.depth), 
      rendered(val.rendered),
      sp(val.sp), 
      type(val.value.type), 
      op(val.op),
      tk(nullptr),
      js(nullptr)
{
    if (rendered) {
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
}

JQ::JQ(int depth, unsigned int hash, span sp)
    : hash(hash),
      depth(depth),  
      rendered(false),
      sp(sp), 
      type(JQ_NO_VAL),
      tk(nullptr),
      js(nullptr) {}


/*
    unsigned int hash;      //! The hash of the path
    unsigned int depth;     //! The depth of the path
    bool rendered;          //! Has this value been redered to an actual value yet
    span sp;                //! Place in the string for the value
    jqvalue_u val;          //! The value
    jqvaltype_e type;       //! The value type
    jq_operator op;         //! The opeartor
    jsmntok_t * tk;         //! Token ascociated with this value
    const char * js;        //! JSON string - cludge! 

*/

JQ::JQ(int depth, unsigned int hash, const char *str)
    : hash(hash), 
      depth(depth), 
      rendered(true), 
      sp({0,0}),
      type(JQ_STRING_VAL),
      tk(nullptr),
      js(nullptr) 
{
    val.string_val = copy_string((char *)str);
}


JQ::JQ(int depth, unsigned int hash, jsmntok_t * tk, const char * js)
    : hash(hash),
      depth(depth),  
      rendered(false), 
      type(JQ_NO_VAL),
      tk(tk),
      js(js) {}

JQ::~JQ() {
    if (type == JQ_STRING_VAL) {
        kill_string(val.string_val);
    }
}

bool JQ::operator==(struct jqpath &path) {
    if (path.hash == hash) {
        if (path.depth == depth) {
            if (path.value.type != JQ_NO_VAL && path.value.type == type) {
                switch (path.value.type) {
                case JQ_INT_VAL:
                    return path.value.value.int_val == val.int_val;
                case JQ_FLOAT_VAL:
                    return path.value.value.float_val == val.float_val;
                case JQ_STRING_VAL:
                    return cmp_string(path.value.value.string_val, val.string_val);
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
    if (this->type == JQ_INT_VAL) {
        return this->val.int_val == val;
    }

    return false;
}

bool JQ::operator==(float val) {
    if (this->type == JQ_FLOAT_VAL) {
        return this->val.float_val == val;
    }

    return false;
}

bool JQ::operator==(const char *str) {
    if(this->type == JQ_NO_VAL) {
        // Render the token
        if(this->rendered == false && this->tk != nullptr) {
            if(this->js == nullptr) {
                throw std::runtime_error("JSON string not set but token non null pointer");
            }

            // Render the token
            switch(tk->type) {
                case JSMN_STRING: {
                    // Include the quotes in the captured string
                    int len = this->tk->end - this->tk->start + 2;
                    this->type = JQ_STRING_VAL;
                    this->val.string_val = (char*)malloc(len+1);
                    if(this->val.string_val == nullptr) {
                        throw std::runtime_error("memory allcoation failure");
                    }

                    memccpy(this->val.string_val,&js[tk->start-1],1,len);
                    this->val.string_val[len] = '\0';

                    return *this == str;

                }
                break;
                
                case JSMN_PRIMITIVE: {

                }
                break;

                case JSMN_ARRAY:
                case JSMN_OBJECT:
                    throw std::runtime_error("unsupported comparison");  

                case JSMN_UNDEFINED:
                    throw std::runtime_error("undefined value");
            }
        }
    }
    else if (this->type == JQ_STRING_VAL) {
        return std::strcmp(this->val.string_val, str) == 0;
    }

    return false;
}

bool JQ::operator==(std::string str) {
    if (this->type == JQ_STRING_VAL) {
        return str == this->val.string_val;
    }

    return false;
}
