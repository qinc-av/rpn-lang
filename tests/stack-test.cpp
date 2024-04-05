/***************************************************
 * file: qinc/rpn-lang/tests/test-stack.cpp
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
  virtual bool operator==(const rpn::Stack::Object &orhs) const override {
    auto &rhs = PEEK_CAST(const CustomArray,orhs);
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
  virtual std::string deparse() const override {
    return "not-yet";
  }
private:
  std::vector<std::unique_ptr<rpn::Stack::Object>> _v;
};

class CustomObject {
 public:
  CustomObject(const std::string &v) : _v(v) {};
  virtual operator std::string() const { return std::string("CustomObject: <") + _v + ">"; };
  auto val() const { return _v; }
  bool operator==(const CustomObject &rhs) const {
    return _v == rhs._v;
  }
  bool operator>(const CustomObject &rhs) const {
    return _v > rhs._v;
  }
  bool operator<(const CustomObject &rhs) const {
    return _v < rhs._v;
  }
private:
  std::string _v;
};

using StCustomObject = TStackObject<CustomObject>;

static int stackCount = 0;
static rpn::Stack g_stack;

TEST_CASE("peek and types" "stack") {
  g_stack.push_double(3.14159265359); stackCount++;
  REQUIRE(g_stack.depth() == 1);

  g_stack.push_double(2.71828182846); stackCount++;
  g_stack.push_string("abcdefg"); stackCount++;
  g_stack.push_integer(1023); stackCount++;
  g_stack.push_boolean(true); stackCount++;
  REQUIRE(g_stack.depth() == stackCount);

  /*
   * now we have the stack as:
   *
   * 5: pi
   * 4: e
   * 3: "abcdefg"
   * 2: 1023
   * 1: true
   */
  REQUIRE( true == g_stack.peek_boolean(1) );
  REQUIRE( 1023 == g_stack.peek_integer(2) );
  REQUIRE( "abcdefg" == g_stack.peek_string(3) );
  REQUIRE( 2.71828182846 == g_stack.peek_double(4) );
  REQUIRE( 3.14159265359 == g_stack.peek_double(5) );
  REQUIRE( "1023" == g_stack.peek_as_string(2) ); // converts to string
  REQUIRE( 1023.0 == g_stack.peek_as_double(2) ); // converts to double

  CHECK_THROWS(g_stack.peek_string(2) == "1023"); // cast fails

  {
    REQUIRE( g_stack.peek_double(5) ==  3.14159265359);
  }

  {
    const std::string s1 = "string1";
    g_stack.push_string(s1);
    auto s2 = g_stack.pop_string();
    REQUIRE(s2 == s1);
  }
}

TEST_CASE("stack operations" "stack") {
  g_stack.push_integer(511);
  g_stack.clear();
  stackCount = 0;
  REQUIRE(g_stack.depth() == stackCount);

  g_stack.push_double(3.14159265359); stackCount++;
  g_stack.push_double(2.71828182846); stackCount++;
  g_stack.push_string("abcdefg"); stackCount++;
  g_stack.push_integer(1023); stackCount++;
  g_stack.push_boolean(true); stackCount++;
  REQUIRE(g_stack.depth() == stackCount);
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // push / pop
  g_stack.push_boolean(false);
  REQUIRE( g_stack.pop_boolean() == false );
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // drop
  g_stack.push_integer(123);
  g_stack.drop();
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // dropn
  g_stack.push_integer(123);
  g_stack.push_integer(123);
  g_stack.push_integer(123);
  g_stack.push_integer(123);
  g_stack.dropn(4);
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( g_stack.peek_boolean(1) == true );
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // dup
  g_stack.dup();
  REQUIRE( g_stack.depth() == stackCount+1 );
  REQUIRE( g_stack.peek_boolean(1) == true );
  REQUIRE( g_stack.peek_boolean(2) == true );
  g_stack.drop();
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // dupn
  g_stack.dupn(3);
  REQUIRE( g_stack.depth() == stackCount+3 );
  REQUIRE( (g_stack.peek_boolean(1) == true && g_stack.peek_boolean(4)==true) );
  REQUIRE( (g_stack.peek_integer(2) == 1023 && g_stack.peek_integer(5) == 1023) );
  REQUIRE( (g_stack.peek_string(3) == "abcdefg" && g_stack.peek_string(6) == "abcdefg") );
  g_stack.dropn(3);
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // swap
  REQUIRE( (g_stack.peek_boolean(1) == true && g_stack.peek_integer(2)==1023) );
  g_stack.swap();
  REQUIRE( (g_stack.peek_boolean(2) == true && g_stack.peek_integer(1)==1023) );
  g_stack.swap();
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // nip  / nipn
  g_stack.dupn(2);
  g_stack.nipn(2); // drops the second '1023'
  REQUIRE( (g_stack.peek_boolean(1) == true && g_stack.peek_boolean(2)==true) );
  g_stack.drop();
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // over
  g_stack.over(); stackCount++;
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( (g_stack.peek_integer(1) == 1023 && g_stack.peek_boolean(2)==true && g_stack.peek_integer(3) == 1023) );
  g_stack.drop(); stackCount--;
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // pick
  g_stack.pick(3);
  REQUIRE( g_stack.depth() == stackCount+1 );
  REQUIRE( (g_stack.peek_string(1) == "abcdefg" && g_stack.peek_boolean(2)==true && g_stack.peek_integer(3) == 1023) );
  g_stack.drop();
  REQUIRE( ((true == g_stack.peek_boolean(1)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    (3.14159265359 == g_stack.peek_double(5))) );

  // rolld
  g_stack.rolld();
  REQUIRE( 3.14159265359 == g_stack.peek_double(4) );
  g_stack.rolld();
  REQUIRE( 3.14159265359 == g_stack.peek_double(3) );
  g_stack.rolld();
  REQUIRE( 3.14159265359 == g_stack.peek_double(2) );
  g_stack.rolld();
  REQUIRE( 3.14159265359 == g_stack.peek_double(1) );
  g_stack.rolld();
  REQUIRE( 3.14159265359 == g_stack.peek_double(5) );
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // rollu();
  g_stack.rollu();
  REQUIRE( 3.14159265359 == g_stack.peek_double(1) );
  g_stack.rollu();
  REQUIRE( 3.14159265359 == g_stack.peek_double(2) );
  g_stack.rollu();
  REQUIRE( 3.14159265359 == g_stack.peek_double(3) );
  g_stack.rollu();
  REQUIRE( 3.14159265359 == g_stack.peek_double(4) );
  g_stack.rollu();
  REQUIRE( 3.14159265359 == g_stack.peek_double(5) );
  REQUIRE( ((3.14159265359 == g_stack.peek_double(5)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    (true == g_stack.peek_boolean(1))) );

  // tuckn
  g_stack.tuckn(4); stackCount++;
  REQUIRE( true == g_stack.peek_boolean(4) );
  g_stack.rollun(4);
  g_stack.drop(); stackCount--;
  REQUIRE( ((true == g_stack.peek_boolean(1)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    (3.14159265359 == g_stack.peek_double(5))) );

  // rotu
  g_stack.rotu();
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( ((true == g_stack.peek_boolean(2)) &&
	    (1023 == g_stack.peek_integer(3)) &&
	    ("abcdefg" == g_stack.peek_string(1)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    (3.14159265359 == g_stack.peek_double(5))));
  g_stack.rotu();
  g_stack.rotu();
  // back to original
  REQUIRE( ((true == g_stack.peek_boolean(1)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    (3.14159265359 == g_stack.peek_double(5))) );

  // rotd
  g_stack.rotd();
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( ((true == g_stack.peek_boolean(3)) &&
	    (1023 == g_stack.peek_integer(1)) &&
	    ("abcdefg" == g_stack.peek_string(2)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    (3.14159265359 == g_stack.peek_double(5))));
  g_stack.rotd();
  g_stack.rotd();
  // back to original
  REQUIRE( ((true == g_stack.peek_boolean(1)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    (3.14159265359 == g_stack.peek_double(5))) );

  // rollun
  g_stack.rollun(4);
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( ((true == g_stack.peek_boolean(2)) &&
	    (1023 == g_stack.peek_integer(3)) &&
	    ("abcdefg" == g_stack.peek_string(4)) &&
	    (2.71828182846 == g_stack.peek_double(1)) &&
	    (3.14159265359 == g_stack.peek_double(5))));
  g_stack.rollun(4);
  g_stack.rollun(4);
  g_stack.rollun(4);
  // back to original
  REQUIRE( ((true == g_stack.peek_boolean(1)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    (3.14159265359 == g_stack.peek_double(5))) );

  // rotd
  g_stack.rolldn(4);
  REQUIRE( g_stack.depth() == stackCount );
  REQUIRE( ((true == g_stack.peek_boolean(4)) &&
	    (1023 == g_stack.peek_integer(1)) &&
	    ("abcdefg" == g_stack.peek_string(2)) &&
	    (2.71828182846 == g_stack.peek_double(3)) &&
	    (3.14159265359 == g_stack.peek_double(5))));
  g_stack.rolldn(4);
  g_stack.rolldn(4);
  g_stack.rolldn(4);
  // back to original
  REQUIRE( ((true == g_stack.peek_boolean(1)) &&
	    (1023 == g_stack.peek_integer(2)) &&
	    ("abcdefg" == g_stack.peek_string(3)) &&
	    (2.71828182846 == g_stack.peek_double(4)) &&
	    (3.14159265359 == g_stack.peek_double(5))) );

  g_stack.push_string("wxyz"); stackCount++;
  REQUIRE (g_stack.depth() == stackCount);
  REQUIRE("wxyz" == g_stack.peek_string(1));

  g_stack.push_integer(1234); stackCount++;
  REQUIRE (g_stack.depth() == stackCount);
  g_stack.swap();
  try {
    REQUIRE(1234 == g_stack.peek_integer(2));
  } catch (std::bad_cast e) {
    printf("1234 caught: %s\n", e.what());
  }

  REQUIRE(2.787 != g_stack.peek_double(stackCount));
  CHECK_THROWS(g_stack.peek_integer(stackCount) == 0); // conversion fails
  
  {
    CustomArray ar1;
    ar1.add_value(StDouble(9.8));
    ar1.add_value(StString(std::string("yyz")));
    g_stack.push(ar1); stackCount++;
    REQUIRE (g_stack.depth() == stackCount);
    auto &ar2 = PEEK_CAST(CustomArray,g_stack.peek(1));
    printf("ar1: %s\n", std::string(ar1).c_str());
    printf("ar2: %s\n", std::string(ar2).c_str());
    REQUIRE(ar1 == ar2);

    ar1.add_value(ar2);
    REQUIRE(!(ar1 == ar2));
  }

}

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

  //  g_stack.push(StDouble(3.1415));
  g_stack.push_double(3.1415); stackCount++;
  assert (g_stack.depth() == stackCount);
  g_stack.push_string("wxyz"); stackCount++;
  assert (g_stack.depth() == stackCount);
  //  g_stack.push(StInteger(1234));
  g_stack.push_integer(1234); stackCount++;
  assert (g_stack.depth() == stackCount);
  {
    CustomArray ar1;
    ar1.add_value(StDouble(9.8));
    ar1.add_value(StString(std::string("yyz")));
    g_stack.push(ar1); stackCount++;
    assert (g_stack.depth() == stackCount);
  }
		 
  g_stack.push(StString(std::string("abcdefg"))); stackCount++;
  assert (g_stack.depth() == stackCount);

  g_stack.push(StDouble(19.2)); stackCount++;
  assert (g_stack.depth() == stackCount);

  assert (g_stack.depth() == stackCount);

  {
    double v = g_stack.pop_double(); stackCount--;// std::unique_ptr<StDouble> v(static_cast<StDouble*>(g_stack.pop().release()));
    printf("popped double (%f)\n", v);
    assert (g_stack.depth() == stackCount);
}

  {
    printf("peeked [%s]\n", g_stack.peek_string(1).c_str());
    g_stack.print();
  }

  g_stack.push(StCustomObject(std::string("yyz"))); stackCount++;
  assert (g_stack.depth() == stackCount);
  
  g_stack.push_boolean(true); stackCount++;

  g_stack.swap();
  g_stack.print("swap");

  g_stack.rolldn(2);
  g_stack.print("rolld(2) [aka swap]");
  
  g_stack.pick(2); stackCount++;
  g_stack.print("over");

  g_stack.pick(4); stackCount++;
  g_stack.print("pick(4)");

  g_stack.push(StDouble(2.7)); stackCount++;
  g_stack.dup(); stackCount++;
  assert (g_stack.depth() == stackCount);
  g_stack.print("push(2.7) dup");

  //  g_stack.push(StString("marker"));
  int n = g_stack.depth();
  g_stack.dupn(n);  stackCount+=n;
  assert (g_stack.depth() == stackCount);
  g_stack.print("depth dupn");

  g_stack.dropn(n); stackCount-=n;
  assert (g_stack.depth() == stackCount);
  g_stack.print("dropn");

  g_stack.dupn(n); stackCount+=n;
  assert (g_stack.depth() == stackCount);
  g_stack.print("dupn");

  g_stack.rollun(n/2);
  assert (g_stack.depth() == stackCount);
  printf("rollun(%d)\n", n/2);
  g_stack.print("rollun");
  assert (g_stack.depth() == stackCount);

  g_stack.rollu();
  assert (g_stack.depth() == stackCount);
  g_stack.rollu();
  assert (g_stack.depth() == stackCount);
  g_stack.print("rollu rollu");

  g_stack.rolld();
  assert (g_stack.depth() == stackCount);
  g_stack.print("rolld");

  g_stack.rolldn(4);
  assert (g_stack.depth() == stackCount);
  g_stack.print("rolld(4)");

  g_stack.tuckn(5); stackCount++;
  assert (g_stack.depth() == stackCount);
  g_stack.print("tuckn(5)");

  g_stack.nipn(7); stackCount--;
  assert (g_stack.depth() == stackCount);
  g_stack.print("nipn(7)");

  StObject obj;
  n=g_stack.depth();
  for(unsigned i=0; i<n/2; i++) {
    std::string key="v"+std::to_string(i);
    obj.inner().add_value(key, *g_stack.pop());
  }
  g_stack.push(obj);
  g_stack.rolld();
  g_stack.print("object");

  StArray arr;
  n=g_stack.depth();
  for(unsigned i=0; i<(n-1); i++) {
    arr.inner().add_value(*g_stack.pop());
  }
  g_stack.push(arr);
  g_stack.print("array");

  return 0;
  //  StDouble pi(3.1415); // std::make_unique<double>(3.14159));
  //  s_g_stack.push_back(TStackTypePointer<StackType>(pi));
  //  s_g_stack.push_back(new TStackType<double>(9.876e23));
  //  s_g_stack.push_back(new TStackType<int>(23));
  //  auto v = s_g_stack.back();

  
}

/* end of qinc/rpn-lang/tests/test-g_stack.cpp */
