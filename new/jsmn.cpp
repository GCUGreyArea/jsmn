#include "jsmn.h"


jsmn_parser::~jsmn_parser() {
    if(m_tokens) {
        delete[] m_tokens;
    }

    m_tokens = nullptr;
    m_num_tokens = 0;
    m_length = 0;
}

jsmntok_t *jsmn_parser::jsmn_alloc_token() {
    jsmntok_t *tok;

    // Over the allocated size, realloc.
    if (m_token_next >= m_num_tokens) {
        jsmntok_t *prev = m_tokens;
        unsigned int num_tokens = m_num_tokens;
        m_num_tokens = m_num_tokens * m_mull;
        jsmntok_t *next_toks = new jsmntok_t[m_num_tokens];
        std::memcpy(next_toks, prev, sizeof(jsmntok_t) * (size_t)num_tokens);

        delete[] m_tokens;
        m_tokens = next_toks;
    }

    // ASsign the token and return
    tok = &m_tokens[m_token_next++];
    tok->start = tok->end = -1;
    tok->size = 0;
#ifdef JSMN_PARENT_LINKS
    tok->parent = -1;
#endif
    return tok;
}

/**
 * Fills token type and boundaries.
 */
void jsmn_parser::jsmn_fill_token(jsmntok_t *token, const int start,
                                  const int end, jsmntype_t type) {
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
int jsmn_parser::jsmn_parse_primitive() {
    jsmntok_t *token;
    int start;

    start = m_pos;

    for (; m_pos < m_length && m_js[m_pos] != '\0'; m_pos++) {
        switch (m_js[m_pos]) {
#ifndef JSMN_STRICT
        /* In strict mode primitive must be followed by "," or "}" or "]" */
        case ':':
#endif
        case '\t':
        case '\r':
        case '\n':
        case ' ':
        case ',':
        case ']':
        case '}':
            goto found;
        default:
            /* to quiet a warning from gcc*/
            break;
        }
        if (m_js[m_pos] < 32 || m_js[m_pos] >= 127) {
            m_pos = start;
            return JSMN_ERROR_INVAL;
        }
    }
#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    m_pos = start;
    return JSMN_ERROR_PART;
#endif

found:
    if (m_tokens == NULL) {
        m_pos--;
        return 0;
    }
    token = jsmn_alloc_token();
    if (token == NULL) {
        m_pos = start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, start, m_pos, JSMN_PRIMITIVE);
#ifdef JSMN_PARENT_LINKS
    token->parent = m_toksuper;
#endif
    m_pos--;
    return 0;
}

/**
 * Fills next token with JSON string.
 */
int jsmn_parser::jsmn_parse_string() {
    jsmntok_t *token;

    int start = m_pos;

    /* Skip starting quote */
    m_pos++;

    for (; m_pos < m_length && m_js[m_pos] != '\0'; m_pos++) {
        char c = m_js[m_pos];

        /* Quote: end of string */
        if (c == '\"') {
            if (m_tokens == NULL) {
                return 0;
            }
            token = jsmn_alloc_token();
            if (token == NULL) {
                m_pos = start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_fill_token(token, start + 1, m_pos, JSMN_STRING);
#ifdef JSMN_PARENT_LINKS
            token->parent = m_toksuper;
#endif
            return 0;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && m_pos + 1 < m_length) {
            int i;
            m_pos++;
            switch (m_js[m_pos]) {
            /* Allowed escaped symbols */
            case '\"':
            case '/':
            case '\\':
            case 'b':
            case 'f':
            case 'r':
            case 'n':
            case 't':
                break;
            /* Allows escaped symbol \uXXXX */
            case 'u':
                m_pos++;
                for (i = 0; i < 4 && m_pos < m_length && m_js[m_pos] != '\0';
                     i++) {
                    /* If it isn't a hex character we have an error */
                    if (!((m_js[m_pos] >= 48 && m_js[m_pos] <= 57) || /* 0-9 */
                          (m_js[m_pos] >= 65 && m_js[m_pos] <= 70) || /* A-F */
                          (m_js[m_pos] >= 97 &&
                           m_js[m_pos] <= 102))) { /* a-f */
                        m_pos = start;
                        return JSMN_ERROR_INVAL;
                    }
                    m_pos++;
                }
                m_pos--;
                break;
            /* Unexpected symbol */
            default:
                m_pos = start;
                return JSMN_ERROR_INVAL;
            }
        }
    }
    m_pos = start;
    return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill m_tokens.
 */
int jsmn_parser::parse() {
    int r;
    int i;
    jsmntok_t *token;
    int count = m_token_next;

    for (; m_pos < m_length && m_js[m_pos] != '\0'; m_pos++) {
        char c;
        jsmntype_t type;

        c = m_js[m_pos];
        switch (c) {
        case '{':
        case '[':
            count++;
            // m_depth++;
            if (m_tokens == NULL) {
                break;
            }
            token = jsmn_alloc_token();
            if (token == NULL) {
                return JSMN_ERROR_NOMEM;
            }
            if (m_toksuper != -1) {
                jsmntok_t *t = &m_tokens[m_toksuper];
#ifdef JSMN_STRICT
                /* In strict mode an object or array can't become a key */
                if (t->type == JSMN_OBJECT) {
                    return JSMN_ERROR_INVAL;
                }
#endif
                t->size++;
#ifdef JSMN_PARENT_LINKS
                token->parent = m_toksuper;
#endif
            }
            token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
            token->start = m_pos;
            m_toksuper = m_token_next - 1;
            break;
        case '}':
        case ']':
            // m_depth--;
            if (m_tokens == NULL) {
                break;
            }
            type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
            if (m_token_next < 1) {
                return JSMN_ERROR_INVAL;
            }
            token = &m_tokens[m_token_next - 1];
            for (;;) {
                if (token->start != -1 && token->end == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    token->end = m_pos + 1;
                    m_toksuper = token->parent;
                    break;
                }
                if (token->parent == -1) {
                    if (token->type != type || m_toksuper == -1) {
                        return JSMN_ERROR_INVAL;
                    }
                    break;
                }
                token = &m_tokens[token->parent];
            }
#else
            for (i = m_token_next - 1; i >= 0; i--) {
                token = &m_tokens[i];
                if (token->start != -1 && token->end == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    m_toksuper = -1;
                    token->end = m_pos + 1;
                    break;
                }
            }
            /* Error if unmatched closing bracket */
            if (i == -1) {
                return JSMN_ERROR_INVAL;
            }
            for (; i >= 0; i--) {
                token = &m_tokens[i];
                if (token->start != -1 && token->end == -1) {
                    m_toksuper = i;
                    break;
                }
            }
#endif
            break;
        case '\"':
            r = jsmn_parse_string();
            if (r < 0) {
                return r;
            }
            count++;
            if (m_toksuper != -1 && m_tokens != NULL) {
                m_tokens[m_toksuper].size++;
            }
            break;
        case '\t':
        case '\r':
        case '\n':
        case ' ':
            break;
        case ':':
            m_toksuper = m_token_next - 1;
            break;
        case ',':
            if (m_tokens != NULL && m_toksuper != -1 &&
                m_tokens[m_toksuper].type != JSMN_ARRAY &&
                m_tokens[m_toksuper].type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
                m_toksuper = m_tokens[m_toksuper].parent;
#else
                for (i = m_token_next - 1; i >= 0; i--) {
                    if (m_tokens[i].type == JSMN_ARRAY ||
                        m_tokens[i].type == JSMN_OBJECT) {
                        if (m_tokens[i].start != -1 && m_tokens[i].end == -1) {
                            m_toksuper = i;
                            break;
                        }
                    }
                }
#endif
            }
            break;
#ifdef JSMN_STRICT
        /* In strict mode primitives are: numbers and booleans */
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 't':
        case 'f':
        case 'n':
            /* And they must not be keys of the object */
            if (m_tokens != NULL && m_toksuper != -1) {
                const jsmntok_t *t = &m_tokens[m_toksuper];
                if (t->type == JSMN_OBJECT ||
                    (t->type == JSMN_STRING && t->size != 0)) {
                    return JSMN_ERROR_INVAL;
                }
            }
#else
        /* In non-strict mode every unquoted value is a primitive */
        default:
#endif
            r = jsmn_parse_primitive();
            if (r < 0) {
                return r;
            }
            count++;
            if (m_toksuper != -1 && m_tokens != NULL) {
                m_tokens[m_toksuper].size++;
            }
            break;

#ifdef JSMN_STRICT
        /* Unexpected char in strict mode */
        default:
            return JSMN_ERROR_INVAL;
#endif
        }
    }

    if (m_tokens != NULL) {
        for (i = m_token_next - 1; i >= 0; i--) {
            /* Unmatched opened object or array */
            if (m_tokens[i].start != -1 && m_tokens[i].end == -1) {
                return JSMN_ERROR_PART;
            }
        }
    }

    return count;
}

/**
 * Creates a new parser based over a given buffer with an array of m_tokens
 * available.
 */
void jsmn_parser::init(const char *js) {
    m_pos = 0;
    m_token_next = 0;
    m_toksuper = -1;

    m_js = js;
    m_length = m_js.length();
    std::memset(m_tokens, 0, sizeof(jsmntok_t) * m_num_tokens);
}

void jsmn_parser::init(std::string js) {
    init(js.c_str());
}

bool jsmn_parser::serialise(const char *file_name) {
    FILE *fp = fopen(file_name, "wd");
    if (fp == NULL) {
        return false;
    }

    // Save the string and length
    fwrite(&m_length, sizeof(m_length), 1, fp);
    fwrite(m_js.data(), sizeof(char), m_length, fp);
    fwrite(&m_pos, sizeof(m_pos), 1, fp);

    // Write out the last token index
    fwrite(&m_token_next, sizeof(m_token_next), 1, fp);

    // Write the number of objects first
    fwrite(m_tokens, sizeof(struct jsmntok), m_token_next, fp);

    // Close the file and exit
    fclose(fp);

    return true;
}

/**
 * @brief Deserialise a previously serialised JSON structure
 *
 * @param file_name
 * @return true
 * @return false
 */
bool jsmn_parser::deserialise(const char *file_name) {
    FILE *fp = fopen(file_name, "rd");
    if (fp == NULL) {
        return false;
    }

    // Retrieve the string
    size_t size = fread(&m_length, sizeof(m_length), 1, fp);
    if (m_length == 0 || size == 0) {
        return false;
    }
    
    // Read in the string 
    char *js = new char[m_length + 1];
    size = fread(js, sizeof(char), m_length, fp);
    if (size == 0) {
        goto error;
    }
    js[m_length] = '\0';

    m_js = js;
    delete[] js;

    // Read back the position
    size = fread(&m_pos, sizeof(m_pos), 1, fp);
    if (size == 0) {
        goto error;
    }

    // Read in the last token idex
    size = fread(&m_token_next, sizeof(m_token_next), 1, fp);
    if (size == 0) {
        goto error;
    }

    // Set up the token array if we need to
    if (m_token_next - 1 > m_num_tokens) {
        if (m_tokens != nullptr) {
            delete[] m_tokens;
        }
        m_tokens = new jsmntok_t[m_token_next + 1];
        m_num_tokens = m_token_next + 1;
    }

    size = fread(m_tokens, sizeof(struct jsmntok), m_token_next, fp);
    if (size == 0) {
        goto error;
    }

    fclose(fp);
    return true;

error:
    fclose(fp);
    return false;
}

void jsmn_parser::print_token(int idx) {
    jsmntok_t *t = &m_tokens[idx];

    switch (t->type) {
    case JSMN_UNDEFINED:
        std::cout << "type: undefined, ";
        break;
    case JSMN_OBJECT:
        std::cout << "type: object, ";
        break;
    case JSMN_ARRAY:
        std::cout << "type: array, ";
        break;
    case JSMN_STRING:
        std::cout << "type: string, ";
        break;
    case JSMN_PRIMITIVE:
        std::cout << "type: primitive, ";
        break;
    }

    std::cout << "start  :" << t->start << ", ";
    std::cout << "end    :" << t->end << ", ";
    std::cout << "size   :" << t->size << ", "; 
    std::cout << "parent :" << t->parent << ", ";

    // copy the string into a buffer and print it
    if(t->type == JSMN_STRING || t->type == JSMN_PRIMITIVE) {
        size_t len = t->end - t->start;
        std::string str(m_js.data() + t->start, len);
        std::cout << "value: " << "\"" << str << "\"" <<std::endl;
    }
    else {
        std::cout << std::endl;
    }
}

std::string jsmn_parser::extract_string(int idx) {
    if(idx >= m_token_next) {
        throw std::runtime_error("token is out of bounds : " + std::to_string(idx));
    }

    jsmntok_t *t = &m_tokens[idx];
    size_t len = t->end - t->start;
    return std::string(m_js.data() + t->start, len);
}

int jsmn_parser::extract_int(int idx) {
    if(idx >= m_token_next) {
        throw std::runtime_error("token is out of bounds : " + std::to_string(idx));
    }

    if(m_tokens[idx].type == JSMN_STRING || m_tokens[idx].type == JSMN_PRIMITIVE) {

        jsmntok_t *t = &m_tokens[idx];
        size_t len = t->end - t->start;
        std::string str(m_js.data() + t->start, len);

        char* p;
        long converted = strtol(str.c_str(), &p, 10);
        if (*p) {
            throw std::runtime_error("not a number : " + std::string(m_js.data() + t->start, len));
        }

        return (int) converted;
    }

    throw std::runtime_error("can not convert token type to number");
}

bool jsmn_parser::extract_bool(int idx) {
    if(idx >= m_token_next) {
        throw std::runtime_error("token is out of bounds : " + std::to_string(idx));
    }

    if(m_tokens[idx].type == JSMN_PRIMITIVE) {
        std::string val = extract_string(idx);

        if(val == "true") {
            return true;
        }
        else if(val == "false") {
            return false;
        }


        throw std::runtime_error("unrecognised boolean value " + val + " at token index " + std::to_string(idx));
    }    

    throw std::runtime_error("can't extract boolean value form a non primative token : index : " + std::to_string(idx) + " string : " + extract_string(idx));
}

// size shows keys + values
int jsmn_parser::find_key_in_object(int tkn, std::string search) {

    if(tkn >= m_token_next) {
        throw std::runtime_error("Token out of bounds : " + std::to_string(tkn));
    }

    if(m_tokens[tkn].type != JSMN_OBJECT) {
        throw std::runtime_error("Token is not an object : " + std::to_string(tkn));
    }

    int place = tkn+1;

    int i = place;
    int end = m_tokens[tkn].end;
    while(i < m_token_next && m_tokens[i].start < end) {
        // Only test tokens who's parent is the object
        if(m_tokens[i].parent == tkn) {
            std::string st = extract_string(i);
            if(st == search) {
                return i;
            }
        }

        int end_this = m_tokens[i].end;
        while(i < m_token_next && m_tokens[i].start < end_this) {
            i++;
        }
    }

    return -1;
}

bool jsmn_parser::is_object(int tkn) {
    if(tkn >= m_token_next) {
        return false;
    }

    return m_tokens[tkn].type == JSMN_OBJECT;
}

bool jsmn_parser::is_string(int tkn) {
    if(tkn >= m_token_next) {
        return false;
    }

    return m_tokens[tkn].type == JSMN_STRING;    
}

/**
 * @brief 
 * 
 * @param key The key to use in the JSON object for this value 
 * @param value The value to insert
 * @param type The JSMN type for the object
 * @param obj The offset in the tokens that represents the object into which the key/value will be inserted 
 * @return true 
 * @return false 
 */
bool jsmn_parser::inster_kv_into_obj(std::string key, std::string value, jsmntype_t type, int obj) {
    if(obj >= m_token_next) {
        return false;
    }

    if(m_tokens[obj].type != JSMN_OBJECT) {
        return false;
    }

    // We need to find the insertion place in the string, 
    // and then reparse to make sure everythinig is OK
    jsmntok_t *t = &m_tokens[obj];
    std::string objstr(m_js.data() + 0, t->start+1);
    std::string endpart(m_js.data() + t->start+1, m_js.length() - t->start);

    // Key must be of typep string and must start with and end with "
    if(key[0] != '"' || key[key.length()-1] != '"') {
        return false;
    }

    // Make sure the value is OK based on jsmntype_t value
    // It must be in itself, legal JSON (when added to a correct object)
    switch(type) {
        case JSMN_ARRAY:
            if(value[0] != '[' || value[value.length()-1] != ']') {
                return false;
            }

        case JSMN_OBJECT:
            if(value[0] != '{' || value[value.length()-1] != '}') {
                return false;
            }

        case JSMN_PRIMITIVE:
            if((value[0] == 't' && value == "true") || (value[0] == 'f' && value == "false")) {
                break;
            }
            else if(isdigit(value[0]) || value[0] == 'e' || value[0] == '-') {
                break;
            }
            return false;

        case JSMN_STRING:
            // A string value must be "\"value\"" NOT "value"
            if(value[0] == '"' && value [value.length()-1] == '"') {
                break;
            }
            return false;

        case JSMN_UNDEFINED:
        default:
            return false;
    }

    // Make sure that the resulting JSON is OK
    std::string json = "{" + key + ":" + value + "}";
    jsmn_parser p;
    p.init(json);
    
    int r = p.parse();
    if(r < 0) {
        throw std::runtime_error("key and value create invalid JSON : " + json);
    }

    json = objstr + key + ":" + value + "," + endpart;

    init(json);

    r = parse();
    if(r < 0) {
        throw std::runtime_error("invalid JSON after insert : " + json);
    }

    return true;
}

bool jsmn_parser::insert_value_in_list(std::string value, jsmntype_t type, int obj) {

    if(m_tokens[obj].type != JSMN_ARRAY) {
        return false;
    }

    jsmntok_t *t = &m_tokens[obj];
    std::string objstr(m_js.data() + 0, t->start+1);
    std::string endpart(m_js.data() + t->start+1, m_js.length() - t->start);

    // Make sure the value is OK based on jsmntype_t value
    // It must be in itself, legal JSON (when added to a correct object)
    switch(type) {
        case JSMN_ARRAY:
            if(value[0] != '[' || value[value.length()-1] != ']') {
                return false;
            }

        case JSMN_OBJECT:
            if(value[0] != '{' || value[value.length()-1] != '}') {
                return false;
            }

        case JSMN_PRIMITIVE:
            if((value[0] == 't' && value == "true") || (value[0] == 'f' && value == "false")) {
                break;
            }
            else if(isdigit(value[0]) || value[0] == 'e' || value[0] == '-') {
                break;
            }
            return false;

        case JSMN_STRING:
            // A string value must be "\"value\"" NOT "value"
            if(value[0] == '"' && value [value.length()-1] == '"') {
                break;
            }
            return false;

        case JSMN_UNDEFINED:
        default:
            return false;
    }


    std::string json = objstr + value + "," + endpart;

    init(json);
    if(parse() < 0) {
        return false;
    }

    return true;
}

bool jsmn_parser::update_value_for_key(int k, std::string v) {
    if(k >= m_token_next || k+1 >= m_token_next) {
        throw std::runtime_error("Token out of bounds : " + std::to_string(k));
    }

    jsmntok_t * val = &m_tokens[k+1];

    std::string start(m_js.data(), val->start);
    std::string end(m_js.data() + val->end,m_js.length()-val->end);
    // end += '\0';

    std::string json = "";
    if((v[0] == '{' && v[v.length()-1] == '}') || (v[0] == '[' && v[v.length()-1] == ']')) {
        jsmn_parser p;
        p.init(v);
        if(p.parse() < 0) {
            throw std::runtime_error("invalid JSON : " + v);
        }

        json = start + v + end;
    }
    else {
        json = start + v + end;
    }

    init(json);
    if(parse() < 0) {
        throw std::runtime_error("JSON failed to parse : " + json);
    }

    return true;
}