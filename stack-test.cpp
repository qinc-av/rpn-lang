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

class CustomArray : public rpn::Stack::Object {
public:
  CustomArray() = default;
  CustomArray(const CustomArray &a)  {
    for(auto const &e : a._v) {
      _v.push_back(e->deep_copy());
    }
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const { return std::make_unique<CustomArray>(*this); };
  void add_value(const rpn::Stack::Object &val) {
    _v.push_back(val.deep_copy());
  }
  virtual operator std::string() const {
    std::string rv = "[";
    for(auto const &e : _v) {
      rv += e->to_string();
      rv += ", ";
    }
    rv += "]";
    return rv;
  };
  auto const &val() const { return _v; };
private:
  std::vector<std::unique_ptr<rpn::Stack::Object>> _v;
};

class CustomObject {
 public:
  CustomObject(const std::string &v) : _v(v) {};
  virtual operator std::string() const { return std::string("CustomObject: <") + _v + ">"; };
  auto val() const { return _v; }
private:
  std::string _v;
};

using StCustomObject = TStackObject<CustomObject>;

int
main(int ac, char **av) {
  int stackCount = 0;
  rpn::Stack stack;
  //  stack.push(StDouble(3.1415));
  stack.push_double(3.1415); stackCount++;
  assert (stack.depth() == stackCount);
  stack.push_string("wxyz"); stackCount++;
  assert (stack.depth() == stackCount);
  //  stack.push(StInteger(1234));
  stack.push_integer(1234); stackCount++;
  assert (stack.depth() == stackCount);
  {
    CustomArray ar1;
    ar1.add_value(StDouble(9.8));
    ar1.add_value(StString(std::string("yyz")));
    stack.push(ar1); stackCount++;
    assert (stack.depth() == stackCount);
  }
		 
  stack.push(StString(std::string("abcdefg"))); stackCount++;
  assert (stack.depth() == stackCount);

  stack.push(StDouble(19.2)); stackCount++;
  assert (stack.depth() == stackCount);

  assert (stack.depth() == stackCount);

  {
    double v = stack.pop_double(); stackCount--;// std::unique_ptr<StDouble> v(static_cast<StDouble*>(stack.pop().release()));
    printf("popped double (%f)\n", v);
    assert (stack.depth() == stackCount);
}

  {
    const StString &tsr(dynamic_cast<const StString&>(stack.peek(1)));
    printf("peeked [%s]\n", tsr.val().c_str());
    stack.print();
  }

  stack.push(StCustomObject(std::string("yyz"))); stackCount++;
  assert (stack.depth() == stackCount);
  
  stack.push_boolean(true); stackCount++;

  stack.swap();
  stack.print("swap");

  stack.rolldn(2);
  stack.print("rolld(2) [aka swap]");
  
  stack.pick(2); stackCount++;
  stack.print("over");

  stack.pick(4); stackCount++;
  stack.print("pick(4)");

  stack.push(StDouble(2.7)); stackCount++;
  stack.dup(); stackCount++;
  assert (stack.depth() == stackCount);
  stack.print("push(2.7) dup");

  //  stack.push(StString("marker"));
  int n = stack.depth();
  stack.dupn(n);  stackCount+=n;
  assert (stack.depth() == stackCount);
  stack.print("depth dupn");

  stack.dropn(n); stackCount-=n;
  assert (stack.depth() == stackCount);
  stack.print("dropn");

  stack.dupn(n); stackCount+=n;
  assert (stack.depth() == stackCount);
  stack.print("dupn");

  stack.rollun(n/2);
  assert (stack.depth() == stackCount);
  printf("rollun(%d)\n", n/2);
  stack.print("rollun");
  assert (stack.depth() == stackCount);

  stack.rollu();
  assert (stack.depth() == stackCount);
  stack.rollu();
  assert (stack.depth() == stackCount);
  stack.print("rollu rollu");

  stack.rolld();
  assert (stack.depth() == stackCount);
  stack.print("rolld");

  stack.rolldn(4);
  assert (stack.depth() == stackCount);
  stack.print("rolld(4)");

  stack.tuckn(5); stackCount++;
  assert (stack.depth() == stackCount);
  stack.print("tuckn(5)");

  stack.nipn(7); stackCount--;
  assert (stack.depth() == stackCount);
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
