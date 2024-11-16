#ifndef JQ_PATH
#define JQ_PATH

#include "jqpath.h"
#include "string_funcs.h"

class JQ {
public:
    JQ();
    JQ(struct jqpath& val);
    ~JQ();

    bool operator==(struct jqpath &path);

protected:

private:
    unsigned int hash;      //! The hash of the path
    unsigned int depth;     //! The depth of the path
    jqvalue_u val;          //! The value 
    jqvaltype_e type;       //! The value type
    jq_operator op;         //! The opeartor
};

#endif // !JQ_PATH
