/***************************************************
 * file: github/elh/rpn-cnc/trpn.cpp
 *
 * @file    trpn.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Thursday, May 18, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "rpn-controller.h"
#include "cnc-app.h"

#include "rpn.h"

int
main(int ac, char **av) {
  rpn::Stack stack;
  stack.push(StDouble(3.1415));
  stack.push(StString("abcdefg"));
  //  stack.push(TStackObject<int>(1234));
  stack.push(TStackObject<std::string>("wxyz"));
  stack.push(TStackObject<XDouble>(19.2));
  auto v = stack.pop();
  printf("popped %s\n", v->to_string().c_str());
  const rpn::Stack::Object &p = stack.peek();
  printf("peeked %s\n", ((std::string)p).c_str());
  stack.print();

  stack.swap();
  stack.print("swap");

  stack.rolldn(2);
  stack.print("rolld(2) [aka swap]");
  
  stack.over();
  stack.print("over");

  stack.pick(4);
  stack.print("pick(4)");

  stack.push(StDouble(2.7));
  stack.dup();
  stack.print("push(2.7) dup");

  stack.push(TStackObject<XDouble>
	     (2.7));

  stack.push(StString("marker"));
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

  XObject obj;
  n=stack.depth();
  for(unsigned i=0; i<n/2; i++) {
    std::string key="v"+std::to_string(i);
    obj.add_value(key, *stack.pop());
  }
  stack.push(TStackObject<XObject>(obj));
  stack.rolld();
  stack.print("object");

  XArray arr;
  n=stack.depth();
  for(unsigned i=0; i<(n-1); i++) {
    arr.add_value(*stack.pop());
  }
  stack.push(TStackObject<XArray>(arr));
  stack.print("array");

  return 0;
  //  StDouble pi(3.1415); // std::make_unique<double>(3.14159));
  //  s_stack.push_back(TStackTypePointer<StackType>(pi));
  //  s_stack.push_back(new TStackType<double>(9.876e23));
  //  s_stack.push_back(new TStackType<int>(23));
  //  auto v = s_stack.back();

  RpnController control;
  MachineInterface mc(control);
  //  std::string file = "/Users/eric/work/github/elh/rpn-cnc/xyz-probe.4nc";
    std::string file = "/Users/eric/work/github/elh/rpn-cnc/tests.4nc";
  if (ac > 1) {
    file = av[1];
  }
  control.loadFile(file);
  //  } else {
  //    printf("usage: %s <filename>\n", av[0]);
  //  }
  
  return 0;
}

/* end of github/elh/rpn-cnc/trpn.cpp */
