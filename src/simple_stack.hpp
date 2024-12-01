#ifndef SIMPLE_STACK_H
#define SIMPLE_STACK_H

#include <stdexcept>
#include <string>
#include <cstring>

template<class T> class simple_stack {
public:
    simple_stack(unsigned int size, unsigned int grow) :
        m_grow(grow),
        m_size(size),
        m_idx(0) {
            m_stack = new T[m_size];
            if(m_stack == nullptr) {
                throw std::runtime_error("Memory allocation failure in simple_stack::simple_stack(size,grow)");
            }
        }

    ~simple_stack() {
        delete [] m_stack;
        m_stack = nullptr;
        m_size = 0;
        m_idx = 0;
    }

    void push(T t) {
        if(m_idx >= m_size) {
            if(m_grow) {
                grow();
            }
            else {
                std::string msg = "simple_stack::push(t): stack size " + std::to_string(m_size) + " reached and grow is 0";
                throw std::runtime_error(msg);
            }
        }

        m_stack[m_idx++] = t;
    }


    T pop() {
        if(m_idx > 0) {
            T i = m_stack[--m_idx];
            return i;
        }

        throw std::runtime_error("simple_stack::pop(): no elements on the stack");
    }

    unsigned int size() {return m_size;}
    unsigned int items() {return m_idx;}

    void reset() {m_idx=0;}
    void clear() {m_idx=0;std::memset(m_stack,0,sizeof(T) * m_size);}


    void serialise(char * fl) {
        // TODO: Implement 
    }

    void deserialise(char * fl) {
        // TODO: Implement
    }

protected:
    void grow() {
        unsigned int new_size = (m_size * m_grow);
        T * ar = new T[new_size];
        if(ar == nullptr) {
            throw std::runtime_error("simple_stack::grow(): Memory allocation faiuure");
        }
        
        std::memmove(ar, m_stack, sizeof(T) * m_size);

        delete [] m_stack;
        m_stack = ar;
        m_size = new_size;
    }

private:
    unsigned int m_grow;
    unsigned int m_size;
    unsigned int m_idx;
    T * m_stack;
};

#endif//SIMPLE_STACK_H