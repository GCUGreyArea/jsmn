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

#define JSMN_STRICT 1
#define JSMN_PARENT_LINKS 1

#include <string>
#include <map>

#include "jqpath.h"


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

struct span {
    int start;
    int end;
};

struct path
{
    int depth;
	unsigned int hash_path;
	
    jsmntok * value_token;
};


/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
class jsmn_parser {
  private:
    // Default tokens array size
    static constexpr unsigned int def_size = 1024;

    size_t m_pos;        // offset in the JSON string
    size_t m_token_next; // next token to allocate
    int m_toksuper;      // superior token node, e.g. parent object or array
    jsmntok_t *m_tokens; // Array of tokens
    unsigned int m_num_tokens; // Number of tokens in the array

    // String to parse
    std::string m_js; // NULL terminated JSON string
    size_t m_length;  // Length of JSON string
    size_t m_mull;    // When we ru out pf tokens, grow by * m_mull
    size_t m_depth;   // Depth into the structure
    std::map<unsigned int,struct path> m_paths; // The value for each path

  protected:
    jsmntok_t *jsmn_alloc_token();
    void jsmn_fill_token(jsmntok_t *token, const int start, const int end,
                         jsmntype_t type);
    int jsmn_parse_primitive();
    int jsmn_parse_string();
    void render(int depth, unsigned int hash, unsigned int& token);


  public:
    jsmn_parser(std::string str, unsigned int mull) : jsmn_parser(str.c_str(),mull) {};
    jsmn_parser(const char *js = "{}",unsigned int mull = 2)
        : m_pos(0),
          m_token_next(0), 
          m_toksuper(-1),
          m_tokens(new jsmntok_t[def_size]),
          m_num_tokens(def_size), 
          // m_length(strlen(js)), 
          m_js(js),
          m_length(m_js.length()),
          m_mull(mull),  
          m_depth(0) {}

    ~jsmn_parser();
    void init(const char *js = "{}");
    int parse();

    jsmntok_t *get_tokens() { return m_tokens; }
    jsmntok_t *get_token(int idx) { return &m_tokens[idx]; }
    size_t parsed_tokens() {return m_token_next; }
    bool serialise(const char *file_name);
    bool deserialise(const char *file_name);
    void print_token(int idx);
    std::string get_json() {return m_js;} 

    unsigned int last_token() {return m_token_next;}
    void render();
};

#endif /* JSMN_H */
