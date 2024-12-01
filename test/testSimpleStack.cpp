#include <gtest/gtest.h>
#include <glog/logging.h>


#include <string>
#include "simple_stack.hpp"


TEST(simple_stack,testPushandGRow) {
    simple_stack<int> stack(2,2);


    ASSERT_EQ(stack.size(),2);
    ASSERT_EQ(stack.items(),0);

    stack.push(1);
    stack.push(2);
    stack.push(3);
    stack.push(4);

    ASSERT_EQ(stack.size(),4);
    ASSERT_EQ(stack.items(),4);

    ASSERT_EQ(stack.pop(),4);
    ASSERT_EQ(stack.pop(),3);
    ASSERT_EQ(stack.pop(),2);
    ASSERT_EQ(stack.pop(),1);

    ASSERT_EQ(stack.items(),0);

    try {
        stack.pop();
    }
    catch(std::runtime_error& e) {
        ASSERT_STREQ(e.what(),"simple_stack::pop(): no elements on the stack");
    }
}

TEST(simple_stack,testPushandGRowReallyBig) {
    simple_stack<int> stack(1,2);

    for(int i=0;i<1024;i++) {
        stack.push(i);
    }

}
