
#include "jsmn.hpp"
#include <cstring>

jsmn_parser::~jsmn_parser() {
    if (m_num_tokens > 0) {
        delete[] m_tokens;
    }
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
    tok->parent = -1;
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
            token->parent = parser->m_toksuper;
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
                    m_toksuper = parent;
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
            if (m_tokens != NULL && parser->m_toksuper != -1) {
                const jsmntok_t *t = &m_tokens[parser->m_toksuper];
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
    m_length = strlen(m_js);

    if (m_num_tokens == 0) {
        m_tokens = new jsmntok_t[def_size];
        m_num_tokens = def_size;
    }
}