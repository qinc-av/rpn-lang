/***************************************************
 * file: github/elh/rpn-cnc/test-stack.cpp
 *
 * @file    test-stack.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Friday, May 26, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "rpn.h"

int
main(int ac, char **av) {
  rpn::Stack stack;
  //  stack.push(StDouble(3.1415));
  stack.push_double(3.1415);
  stack.push_string("wxyz");
  //  stack.push(StInteger(1234));
  stack.push_integer(1234);
  stack.push(StString(std::string("abcdefg")));
  stack.push(StDouble(19.2));

  assert (stack.depth() == 5);

  {
    double v = stack.pop_double(); // std::unique_ptr<StDouble> v(static_cast<StDouble*>(stack.pop().release()));
    printf("popped double (%f)\n", v);
  }

  {
    const StString &tsr(dynamic_cast<const StString&>(stack.peek(1)));
    printf("peeked [%s]\n", tsr.val().c_str());
    stack.print();
  }

  stack.swap();
  stack.print("swap");

  stack.rolldn(2);
  stack.print("rolld(2) [aka swap]");
  
  stack.pick(2);
  stack.print("over");

  stack.pick(4);
  stack.print("pick(4)");

  stack.push(StDouble(2.7));
  stack.dup();
  stack.print("push(2.7) dup");

  //  stack.push(StString("marker"));
  int n = stack.depth();
  stack.dupn(n);
  stack.print("depth dupn");

  stack.dropn(n);
  stack.print("dropn");

  stack.dupn(n);
  stack.print("dupn");

  stack.rollun(n/2);
  printf("rollun(%d)\n", n/2);
  stack.print("rollun");

  stack.rollu();
  stack.rollu();
  stack.print("rollu rollu");

  stack.rolld();
  stack.print("rolld");

  stack.rolldn(4);
  stack.print("rolld(4)");

  stack.tuckn(5);
  stack.print("tuckn(5)");

  stack.nipn(7);
  stack.print("nipn(7)");

  StObject obj;
  n=stack.depth();
  for(unsigned i=0; i<n/2; i++) {
    std::string key="v"+std::to_string(i);
    obj.inner().add_value(key, *stack.pop());
  }
  stack.push(obj);
  stack.rolld();
  stack.print("object");

  StArray arr;
  n=stack.depth();
  for(unsigned i=0; i<(n-1); i++) {
    arr.inner().add_value(*stack.pop());
  }
  stack.push(arr);
  stack.print("array");

  return 0;
  //  StDouble pi(3.1415); // std::make_unique<double>(3.14159));
  //  s_stack.push_back(TStackTypePointer<StackType>(pi));
  //  s_stack.push_back(new TStackType<double>(9.876e23));
  //  s_stack.push_back(new TStackType<int>(23));
  //  auto v = s_stack.back();

  
}

/* end of github/elh/rpn-cnc/test-stack.cpp */
