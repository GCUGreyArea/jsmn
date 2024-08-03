/*
 * MIT License
 *
 * Copyright (c) 2010 Serge Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef JSMN_H
#define JSMN_H

#include <stddef.h>
#include <string.h>

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
    JSMN_UNDEFINED = 0,
    JSMN_OBJECT = 1 << 0,
    JSMN_ARRAY = 1 << 1,
    JSMN_STRING = 1 << 2,
    JSMN_PRIMITIVE = 1 << 3
} jsmntype_t;

enum jsmnerr {
    /* Not enough tokens were provided */
    JSMN_ERROR_NOMEM = -1,
    /* Invalid character inside JSON string */
    JSMN_ERROR_INVAL = -2,
    /* The string is not a full JSON packet, more bytes expected */
    JSMN_ERROR_PART = -3
};

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct jsmntok {
    jsmntype_t type;
    int start;
    int end;
    int size;
    int parent;
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
class jsmn_parser {
  private:
    // Default tokens array size
    static constexpr unsigned int def_size = 1024;

    unsigned int m_pos;        // offset in the JSON string
    unsigned int m_token_next; // next token to allocate
    int m_toksuper;      // superior token node, e.g. parent object or array
    jsmntok_t *m_tokens; // Array of tokens
    unsigned int m_num_tokens; // Number of tokens in the array

    // String to parse
    const char *m_js; // NULL terminated JSON string
    size_t m_length;  // Length of JSON string
    unsigned int m_mull;

  protected:
    jsmntok_t *jsmn_alloc_token();
    void jsmn_fill_token(jsmntok_t *token, const int start, const int end,
                         jsmntype_t type);
    int jsmn_parse_primitive();
    int jsmn_parse_string();

  public:
    jsmn_parser(unsigned int mull = 2, const char *js = "{}")
        : m_mull(mull), m_pos(0), m_token_next(0), m_toksuper(-1), m_js(js),
          m_num_tokens(0), m_tokens(nullptr), m_length(strlen(js)) {}

    ~jsmn_parser();
    void init(const char *js);
    int parse();
	jsmntok_t * get_tokens() {return m_tokens;}
};

#endif /* JSMN_H */
