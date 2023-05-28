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

#include <catch2/catch_test_macros.hpp>
#include "rpn.h"

class CustomArray : public rpn::Stack::Object {
public:
  CustomArray() = default;
  CustomArray(const CustomArray &a)  {
    for(auto const &e : a._v) {
      _v.push_back(e->deep_copy());
    }
  }
  virtual bool operator==(const rpn::Stack::Object &orhs) override {
    auto &rhs = OBJECT_CAST(CustomArray)(orhs);
    bool rv = _v.size() == rhs._v.size();
    for(auto i=_v.cbegin(), j=rhs._v.cbegin(); rv && i!= _v.cend(); i++,j++) {
      rv &= (**i == **j);
    }
    return rv;
  }
  virtual std::unique_ptr<rpn::Stack::Object> deep_copy() const override { return std::make_unique<CustomArray>(*this); };
  void add_value(const rpn::Stack::Object &val) {
    _v.push_back(val.deep_copy());
  }
  virtual operator std::string() const override {
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
  bool operator==(const CustomObject &rhs) {
    return _v == rhs._v;
  }
private:
  std::string _v;
};

using StCustomObject = TStackObject<CustomObject>;

static int stackCount = 0;
static rpn::Stack stack;

static int Factorial( int number ) {
  //   return number <= 1 ? number : Factorial( number - 1 ) * number;  // fail
  return number <= 1 ? 1      : Factorial( number - 1 ) * number;  // pass
}

TEST_CASE( "Factorial of 0 is 1 (fail)", "[single-file]" ) {
    REQUIRE( Factorial(0) == 1 );
}

TEST_CASE( "Factorials of 1 and higher are computed (pass)", "[single-file]" ) {
    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
    REQUIRE( Factorial(3) == 6 );
    REQUIRE( Factorial(10) == 3628800 );
}

TEST_CASE("push_double - test" "[single-file]") {
  stack.push_double(3.1415); stackCount++;
  REQUIRE( stackCount == 1);
  {
    auto &tsr = OBJECT_CAST(StDouble)(stack.peek(1));
    REQUIRE( tsr.val() == 3.1415 );
  }
  stack.push_string("wxyz"); stackCount++;
  REQUIRE (stack.depth() == stackCount);

  stack.push_integer(1234); stackCount++;
  REQUIRE (stack.depth() == stackCount);

  {
    CustomArray ar1;
    ar1.add_value(StDouble(9.8));
    ar1.add_value(StString(std::string("yyz")));
    stack.push(ar1); stackCount++;
    REQUIRE (stack.depth() == stackCount);
    auto &ar2 = OBJECT_CAST(CustomArray)(stack.peek(1));
    printf("ar1: %s\n", std::string(ar1).c_str());
    printf("ar2: %s\n", std::string(ar2).c_str());
    REQUIRE(ar1 == ar2);

    ar1.add_value(ar2);
    REQUIRE(!(ar1 == ar2));
  }

}

// additional test cases
// TEST_CASE("stack-test: push/pop", "[single-file]") {}

// TEST_CASE("stack-test: clear", "[single-file]") {}
// TEST_CASE("stack-test: depth", "[single-file]") {}
// TEST_CASE("stack-test: dropn", "[single-file]") {}
// TEST_CASE("stack-test: dupn", "[single-file]") {}
// TEST_CASE("stack-test: nipn", "[single-file]") {}
// TEST_CASE("stack-test: pick", "[single-file]") {}
// TEST_CASE("stack-test: rolldn", "[single-file]") {}
// TEST_CASE("stack-test: rollun", "[single-file]") {}
// TEST_CASE("stack-test: tuckn", "[single-file]") {}
// TEST_CASE("stack-test: swap", "[single-file]") {}
// TEST_CASE("stack-test: drop", "[single-file]") {}
// TEST_CASE("stack-test: rollu", "[single-file]") {}
// TEST_CASE("stack-test: rolld", "[single-file]") {}
// TEST_CASE("stack-test: over", "[single-file]") {}
// TEST_CASE("stack-test: dup", "[single-file]") {}
// TEST_CASE("stack-test: rotu", "[single-file]") {}
// TEST_CASE("stack-test: rotd", "[single-file]") {}


// TEST_CASE("object-test StDouble", "[single-file]") {}
// TEST_CASE("object-test StInteger", "[single-file]") {}
// TEST_CASE("object-test StString", "[single-file]") {}
// TEST_CASE("object-test StBoolean", "[single-file]") {}
// TEST_CASE("object-test StObject", "[single-file]") {}
// TEST_CASE("object-test StArray", "[single-file]") {}
// TEST_CASE("object-test CustomArray", "[single-file]") {}
// TEST_CASE("object-test CustomObject", "[single-file]") {}
// TEST_CASE("object-test Inheritance", "[single-file]") {}


int
rpn_main(int ac, char **av) {

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
