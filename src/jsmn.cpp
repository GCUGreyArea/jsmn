#include <iostream>
#include <stack>

#include <cstring>

#include "JQ.hpp"
#include "jsmn.hpp"
#include "hash.h"
#include "kv_state.h"
#include "Numbers.h"
#include "hash.h"

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

    // Assign the token and return
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
void jsmn_parser::do_list_values(unsigned int path_hash, unsigned int index, kv_state kv) {

    // Here we need to add the array index into the path
    std::string idx = "[" + std::to_string(index) + "]";
    unsigned int hv = merge_hash(path_hash,hash(idx.c_str(),idx.length()));
    path_holder p = {hv, m_depth, kv == kv_state::KEY,std::make_shared<std::vector<jsmntok_t *>>()};
    p.list->push_back(&m_tokens[m_token_next-1]);
    m_paths.emplace(hv,p);    
    hv = merge_hash(path_hash,hash("[]",2));

    if(index == 0) {
        path_holder p1 = {hv, m_depth, false, std::make_shared<std::vector<jsmntok_t *>>()};
        p1.list->push_back(&m_tokens[m_token_next-1]);
        m_paths.emplace(hv,p1);
    }
    else {
        auto it = m_paths.find(hv);
        if(it == m_paths.end()) {
            // TODO: Make that better!
            throw std::runtime_error("Bad state");
        }
        it->second.list->push_back(&m_tokens[m_token_next-1]);
    }

}

unsigned int jsmn_parser::get_token_hash() {
    jsmntok_t * t = &m_tokens[m_token_next-1];

    switch (t->type)
    {
    case JSMN_PRIMITIVE:
    case JSMN_STRING: 
        return hash(m_js.data() + t->start,t->end - t->start);

    default:
        throw std::runtime_error(std::string("Illegal use of internal function: get_token_hash() for token type: ") + to_string(t->type));
        break;
    }
}


int jsmn_parser::parse() {
    int r;
    int i;
    jsmntok_t *token;
    int count = m_token_next; // Represents the number fo tokens parsed
    int index = 0; // Current index of the current array

    unsigned int path_hash = 0;
    jsmn_parser::parse_state state = parse_state::START;
    kv_state kv = kv_state::START;

    std::stack<int> indicies_stack; // Save the staet of each list
    std::stack<unsigned int> hash_stack;
    std::stack<jsmn_parser::parse_state> state_stack;

    state_stack.push(parse_state::START);
    m_depth = 0;

    for (; m_pos < m_length && m_js[m_pos] != '\0'; m_pos++) {
        char c;
        jsmntype_t type;

        c = m_js[m_pos];
        switch (c) {
        case '{':
        case '[':
            // Save the old state
            state_stack.push(state); 
            hash_stack.push(path_hash);
            state = (c == '{') ? parse_state::OBJECT : parse_state::LIST; // Set the new state

            // If this is a list increment the counter for depth
            if(state == parse_state::LIST) {
                // Save the current index
                indicies_stack.push(index);
                index=0;
            }

            count++;
            m_depth++;

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
            if(token->type  == JSMN_ARRAY) {
                // restory the preivious state
                if(indicies_stack.size() > 0) { 
                    index = indicies_stack.top();
                    indicies_stack.pop();                
                }
            }
            m_depth--;

            // Get the previous state and remvoe it from the stack
            state = state_stack.top();
            path_hash = hash_stack.top();
            state_stack.pop();
            hash_stack.pop();

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

        case '\"': {
            r = jsmn_parse_string();
            if (r < 0) {
                return r;
            }

            // We need to update the kv value and 
            // update the path hash
            count++;
            if (m_toksuper != -1 && m_tokens != NULL) {
                m_tokens[m_toksuper].size++;
            }

            // NOTE: We need to add array indicies!
            // To do this we need to keep track of the indicies for this array
            // which implies a stack as lists might be nested in lists of objects
            if(state == parse_state::LIST) {
                do_list_values(path_hash, index, kv);
                index++;
            }
            else {
                // Get the hash value for this string
                int start = m_tokens[m_token_next-1].start;
                int end = m_tokens[m_token_next-1].end;
                int len = end - start;
                unsigned int hv = merge_hash(path_hash,hash(m_js.data() + start,len));
                // Otherwise just add to the paths
                path_holder p = {hv, m_depth, kv == kv_state::KEY, std::make_shared<std::vector<jsmntok_t *>>()};
                p.list->push_back(&m_tokens[m_token_next-1]);
                m_paths.emplace(hv,p);
                path_hash = hv;
            }
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

            if(state == parse_state::LIST) {
                do_list_values(path_hash, index, kv);
                index++;
            }
            else {
                // Otherwise just add to the paths
                path_holder p = {path_hash,m_depth, kv == kv_state::KEY, std::make_shared<std::vector<jsmntok_t *>>()};
                p.list->push_back(&m_tokens[m_token_next-1]);
                m_paths.emplace(path_hash,p);
            }

            // A numeric value is always a value because keys can only 
            // be strings
            kv = kv_state::START;
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
    m_paths.clear();
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

jsmn_parser::path_holder * jsmn_parser::get_path(struct jqpath * p) {
    if(p) {
        auto it = m_paths.find(p->hash);
        if(it != m_paths.end()) {
            if(it->second.depth == p->depth) {
                return &it->second; 
            } 
        }
    }

    return nullptr;
}
