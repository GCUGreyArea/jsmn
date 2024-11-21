#ifndef JQ_PATH
#define JQ_PATH

#include <string>

#include "jqpath.h"
#include "string_funcs.h"

class JQ {
public:
    JQ();
    JQ(struct jqpath& val);
    JQ(int depth, unsigned int hash, span sp);
    JQ(int depth, unsigned int hash, const char * str);
    ~JQ();

    bool operator==(struct jqpath &path);
    bool operator==(int val);
    bool operator==(float val);
    bool operator==(const char * str);
    bool operator==(std::string str);

    unsigned int get_depth() {return depth;}
    unsigned int get_hash() {return hash;}
    bool is_rendered() {return rendered;}


protected:

private:
    unsigned int hash;      //! The hash of the path
    unsigned int depth;     //! The depth of the path
    bool rendered;          //! Has this value been redered to an actual value yet
    span sp;                //! Place in the string for the value
    jqvalue_u val;          //! The value
    jqvaltype_e type;       //! The value type
    jq_operator op;         //! The opeartor
};

#endif // !JQ_PATH
