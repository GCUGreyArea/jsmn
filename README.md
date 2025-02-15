# JSMN C++

## References

- [JQ Paths](https://jqlang.org/manual/)
- [JSMN C Library](https://github.com/zserge/jsmn)
- [Path design](docs/path-design.md)
- [Render design](docs/render-design.md)

## Dependancies

- [Google Test Framework](https://github.com/google/googletest)
- [GNUd Bison](https://www.gnu.org/software/bison/)
- [GNU Flex](https://gothub.projectsegfau.lt/westes/flex/)

Google test only needs to be installed if you are planning on building and running the unit tests. 

Flex and Bison can be installed with `apt`

```bash
sudo apt install flex 
sudo apt install bison
```

## Introduction

This is a prot of `JSMN` C library to C++. The ongoing work is to implement full `JQ path` compatibility so that the structures representing parsed `JSON` can be searched using `JQ path` syntax. In order to achieve this end the resulting tokens array can be rendered to a `C++` map that will corespond to the hash value parsed by the `JQ Parser`, which is written using `Flex` and `Bison`.

## Build and run unit tests

Unit tests can be run with

```bash
make test
```

## Creating and using a parser

A number of examples are provided in the `examples` directory. The most basic example can be foudn in [cmd_example.cpp](examples/cmd_example.cpp), This code shows how to use of the parser, along with how to serialise and deserialise parser data. This program can be built by running `make cmd_example` from the main project directory.

You can run the command test program with the following command

```bash
./build/cmd_example '{"name":"barry"}' ".name"
```

You should see the output

```bash
Parsed tokens: 3
Token: 0 type: object, start : 0, end : 16, size : 1, parent : -1, 
Token: 1 type: string, start : 2, end : 6, size : 1, parent : 0, value: "name"
Token: 2 type: string, start : 9, end : 14, size : 0, parent : 1, value: "barry"

Deserialised tokens: 3
Token: 0 type: object, start : 0, end : 16, size : 1, parent : -1, 
Token: 1 type: string, start : 2, end : 6, size : 1, parent : 0, value: "name"
Token: 2 type: string, start : 9, end : 14, size : 0, parent : 1, value: "barry"

Path value for .name  : barry
```

## Library functionality

Most of the functionality has been moved into a `.cpp` file, but there is no reason that it can't be part of a single `.h` or `.hpp` file. None of the alogoythms that parse `JSON` have been altered, but aditional functionality has been addded..

### Dynamic memory management

The parser now has dynamic memory managment through `new`. I debated using `realloc` which is easier to implement and significantly faster, but from a purely `C++` perspective is a `C` function. REimplementing that behaviour (if it is required) is simple, and only requires modification to two functions.

```c++
jsmntok_t *jsmn_parser::jsmn_alloc_token() {
    jsmntok_t *tok;

    // Over the allocated size, realloc.
    if (m_token_next >= m_num_tokens) {
        m_tokens = (jsmntok_t*) realloc(m_tokens,sizeof(jsmntok_t) * m_num_tokens *m_mull);
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

jsmn_parser(const char *js = "{}", unsigned int mull = 2)
    : m_pos(0), 
      m_token_next(0), 
      m_toksuper(-1),
      m_tokens((jsmntok_t*) malloc(sizeof(jsmntok_t(def_size)))), 
      m_num_tokens(def_size),
      m_js(js), 
      m_length(m_js.length()), 
      m_mull(mull), 
      m_depth(0) {}
```

Dynamic memory management in the puer C++ model has a cost as the content of the previous array need to be copied to a new array, however memory is only bounded by that which is availible to the compiler, and hence the program. `realloc` requires that a block of contiguous memory is availible to realocate to, which is not allways possible with very large `JSON` files.

## Code documentation

You can compile the code documentation through the `Doxygen` utility by running `make doc`. 

## TODO

### Full JQ Path functionality 

The functionality to search using an empty array has not been implemented due to the performance overhead of doing so.

```bash
.name[] = "Barry"
```

where the JSON string might be

```json
{"name":["Frank","John", "Fred","Barry"]}
```

Where using JQ 

```bash 
echo '{"name":["Frank","John", "Fred","Barry"]}' | jq .name[] 
```

would produce

```bash
"Frank"
"John"
"Fred"
"Barry"
```

### Parser reentrancy

The current `Flex` / `Bison` parser for the `JQ path` search syntax, because it declares glob assets, is not currently reentrant. This is easily remedied by moveing all values into a dynamically assigned structre to maintain state.

### Correct numeric equality

Currently while the `JQ path` parser renders all numeric values to their natural state (`1.1` becomes a `float` and `1` becomes an `integer`. As importantly `0001` and `1` are rendered into `1` and are equivolent), the `JSMN` implementation does not do this yet.
