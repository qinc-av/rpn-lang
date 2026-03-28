/***************************************************
 * file: qinc/rpn-lang/src/types-dict.cpp
 *
 * @file    types-dict.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Monday, June 12, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "../rpn.h"

#include <cmath>

/***************************************************
 * Boolean literals
 */
NATIVE_WORD_DECL(types, push_true) {
  rpn.stack.push_boolean(true);
  return rpn::WordDefinition::Result::ok;
}
NATIVE_WORD_DECL(types, push_false) {
  rpn.stack.push_boolean(false);
  return rpn::WordDefinition::Result::ok;
}

/***************************************************
 * Integer
 */
// to_integer
NATIVE_WORD_DECL(types, to_int) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  double dval = rpn.stack.pop_as_double();
  int64_t ival = std::round(dval);
  rpn.stack.push_integer(ival);
  return rv;
}


/***************************************************
 * Float
 */
NATIVE_WORD_DECL(types, to_float) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  double dval = rpn.stack.pop_as_double();
  rpn.stack.push_double(dval);
  return rv;
}

/***************************************************
 * String
 */
NATIVE_WORD_DECL(types, to_string) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto obj = rpn.stack.pop();
  rpn.stack.push_string((std::string)*obj);
  return rv;
}

/***************************************************
 * Object
 */
NATIVE_WORD_DECL(t_object, to_object) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  std::string ident = rpn.stack.pop_string();
  auto val = rpn.stack.pop();
  stack::Object obj;
  obj.add_value(ident,*val.get());
  rpn.stack.push(obj);
  return rv;
}

NATIVE_WORD_DECL(t_object, object_to) {
  auto sob = rpn.stack.pop();
  const auto &obj = PEEK_CAST(const stack::Object, *sob);
  for (const auto &m : obj.val()) {
    rpn.stack.push(*m.second);
    rpn.stack.push_string(m.first);
  }
  rpn.stack.push_integer((int64_t)obj.val().size());
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(t_object, add_string_any_object) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  std::string ident = rpn.stack.pop_string();
  auto val = rpn.stack.pop();
  auto sob = rpn.stack.pop();
  stack::Object obj = PEEK_CAST(const stack::Object, *sob);
  obj.add_value(ident, *val.get());
  rpn.stack.push(obj);
  return rv;
}

NATIVE_WORD_DECL(t_object, add_object_string_any) {
  rpn.stack.rolldn(3);
  return NATIVE_WORD_FN(t_object, add_string_any_object)(rpn,ctx,rest);
}

/***************************************************
 * Array
 */
NATIVE_WORD_DECL(t_array, to_array) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  size_t n = size_t(rpn.stack.pop_as_integer());
  rpn.stack.reversen(n);
  stack::Array array;
  for(size_t i=0; i<n; i++) {
    array.add_value(*rpn.stack.pop());
  }
  rpn.stack.push(array);
  return rv;
}

NATIVE_WORD_DECL(t_array, array_to) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto sob = rpn.stack.pop();
  const auto &v = PEEK_CAST(const stack::Array,*sob).val();
  for(auto ri = v.rbegin(); ri != v.rend(); ri++) {
    rpn.stack.push(**ri);
  }
  rpn.stack.push_integer(v.size());
  return rv;
}

NATIVE_WORD_DECL(t_array, add_array_any) {
  // d2_any_array: TOS=any, NOS=array → append any to array
  auto any = rpn.stack.pop();
  auto sob = rpn.stack.pop();
  stack::Array arr = PEEK_CAST(const stack::Array, *sob);
  arr.add_value(*any);
  rpn.stack.push(arr);
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(t_array, add_any_array) {
  // d2_array_any: TOS=array, NOS=any → prepend any to array
  auto sob = rpn.stack.pop();
  const stack::Array &src = PEEK_CAST(const stack::Array, *sob);
  auto any = rpn.stack.pop();
  stack::Array result;
  result.add_value(*any);
  for (const auto &e : src.val()) {
    result.add_value(*e);
  }
  rpn.stack.push(result);
  return rpn::WordDefinition::Result::ok;
}


/***************************************************
 * Vec3
 */

// nan behaves like 0 when added to non-nan
static double nan_add_0(double a, double b) {
  double rv = std::nan("");
  switch(((std::isnan(a)&1)<<1)|(std::isnan(b)&1)) {
  case 0:
    rv = a + b;
    break;
  case 1:
    rv = a;
    break;
  case 2:
    rv = b;
    break;
  case 3:
    //    rv is already nan
    break;
  }
  return rv;
}

static double nan_add(double a, double b) {
  double rv = std::nan("");
  switch(((std::isnan(a)&1)<<1)|(std::isnan(b)&1)) {
  case 0:
    rv = a + b;
    break;
  case 1:
  case 2:
  case 3:
    //    rv is already nan
    break;
  }
  return rv;
}

static double nan_sub_0(double a, double b) {
  double rv = std::nan("");
  switch(((std::isnan(a)&1)<<1)|(std::isnan(b)&1)) {
  case 0:
    rv = a - b;
    break;
  case 1:
    rv = -b;
    break;
  case 2:
    rv = a;
    break;
  case 3:
    // rv is already nan
    break;
  }
  return rv;
}

static double nan_sub(double a, double b) {
  double rv = std::nan("");
  switch(((std::isnan(a)&1)<<1)|(std::isnan(b)&1)) {
  case 0:
    rv = a - b;
    break;
  case 1:
  case 2:
  case 3:
    // rv is already nan
    break;
  }
  return rv;
}

// vec3+number
NATIVE_WORD_DECL(vec3, add_vec3) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o1 = rpn.stack.pop();
  auto o2 = rpn.stack.pop();
  const auto &v1 = POP_CAST(StVec3,o1);
  const auto &v2 = POP_CAST(StVec3,o2);
  rpn.stack.push(StVec3(nan_add_0(v1._x, v2._x), nan_add_0(v1._y, v2._y), nan_add_0(v1._z, v2._z)));
  return rv;
}

// number+vec3
NATIVE_WORD_DECL(vec3, add_num_vec3) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  double n1 = rpn.stack.pop_as_double();
  auto o2 = rpn.stack.pop();
  const auto &v2 = POP_CAST(StVec3,o2);
  rpn.stack.push(StVec3(nan_add(n1, v2._x), nan_add(n1, v2._y), nan_add(n1, v2._z)));
  return rv;
}

// add two vec3's
NATIVE_WORD_DECL(vec3, add_vec3_num) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o1 = rpn.stack.pop();
  double n2 = rpn.stack.pop_as_double();
  const auto &v1 = POP_CAST(StVec3,o1);
  rpn.stack.push(StVec3(nan_add(v1._x, n2), nan_add(v1._y, n2), nan_add(v1._z, n2)));
  return rv;
}

NATIVE_WORD_DECL(vec3, sub_vec3) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o2 = rpn.stack.pop();
  auto o1 = rpn.stack.pop();
  const auto &v1 = POP_CAST(StVec3,o1);
  const auto &v2 = POP_CAST(StVec3,o2);
  rpn.stack.push(StVec3(nan_sub_0(v1._x, v2._x), nan_sub_0(v1._y, v2._y), nan_sub_0(v1._z, v2._z)));
  return rv;
}

// number+vec3
NATIVE_WORD_DECL(vec3, sub_num_vec3) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  double n1 = rpn.stack.pop_as_double();
  auto o2 = rpn.stack.pop();
  const auto &v2 = POP_CAST(StVec3,o2);
  rpn.stack.push(StVec3(nan_sub(n1, v2._x), nan_sub(n1, v2._y), nan_sub(n1, v2._z)));
  return rv;
}

// add two vec3's
NATIVE_WORD_DECL(vec3, sub_vec3_num) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o1 = rpn.stack.pop();
  double n2 = rpn.stack.pop_as_double();
  const auto &v1 = POP_CAST(StVec3,o1);
  rpn.stack.push(StVec3(nan_sub(v1._x, n2), nan_sub(v1._y, n2), nan_sub(v1._z, n2)));
  return rv;
}

// 3 numbers to a vec3
NATIVE_WORD_DECL(vec3, to_vec3) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  double z = rpn.stack.pop_as_double();
  double y = rpn.stack.pop_as_double();
  double x = rpn.stack.pop_as_double();
  rpn.stack.push(StVec3(x,y,z));
  return rv;
}

// vec3 to 3 doubles
NATIVE_WORD_DECL(vec3, vec3_to) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto o1 = rpn.stack.pop();
  const auto &v1 = POP_CAST(StVec3,o1);
  rpn.stack.push_double(v1._x);
  rpn.stack.push_double(v1._y);
  rpn.stack.push_double(v1._z);
  return rv;
}

//  number to x-value in Vec3
NATIVE_WORD_DECL(vec3, to_vec3x) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  double n1 = rpn.stack.pop_as_double();
  rpn.stack.push(StVec3(n1,std::nan(""),std::nan("")));
  return rv;
}

//  number to y-value in Vec3
NATIVE_WORD_DECL(vec3, to_vec3y) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  double n1 = rpn.stack.pop_as_double();
  rpn.stack.push(StVec3(std::nan(""),n1,std::nan("")));
  return rv;
}

//  number to z-value in Vec3
NATIVE_WORD_DECL(vec3, to_vec3z) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  double n1 = rpn.stack.pop_as_double();
  rpn.stack.push(StVec3(std::nan(""),std::nan(""),n1));
  return rv;
}

NATIVE_WORD_DECL(t_array, reverse) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  auto sob = rpn.stack.pop();
  stack::Array arr = PEEK_CAST(const stack::Array, *sob);
  arr.reverse();
  rpn.stack.push(arr);
  return rv;
}

/***************************************************
 * JSON
 */
NATIVE_WORD_DECL(types, to_json) {
  // ->JSON: pop any value, call to_json(), push stack::Json containing the "data" field.
  auto obj = rpn.stack.pop();
  auto descriptor = obj->to_json();
  rpn.stack.push(stack::Json(descriptor["data"]));
  return rpn::WordDefinition::Result::ok;
}

NATIVE_WORD_DECL(types, json_to) {
  // JSON->: unpack a stack::Json value onto the stack, analogous to ARRAY-> and OBJ->.
  // - JSON array  → elements pushed bottom-to-top as stack::Json, then integer count
  // - JSON object → (value stack::Json, key stack::String) pairs in iteration order, then integer count
  // - JSON scalar → push as native type (stack::Double/stack::Integer/stack::Boolean/stack::String)
  // - JSON null   → push stack::Json(null) unchanged
  // Note: this is a pure unpack; no round-trip with ->JSON is implied.
  auto obj = rpn.stack.pop();
  auto *jp = dynamic_cast<const stack::Json *>(obj.get());
  if (!jp) {
    rpn.stack.push(*obj);
    return rpn::WordDefinition::Result::param_error;
  }
  const auto &j = *jp;
  if (j.is_array()) {
    for (auto it = j.rbegin(); it != j.rend(); ++it) rpn.stack.push(stack::Json(*it));
    rpn.stack.push_integer((int64_t)j.size());
  } else if (j.is_object()) {
    for (auto it = j.begin(); it != j.end(); ++it) {
      rpn.stack.push(stack::Json(it.value()));
      rpn.stack.push_string(it.key());
    }
    rpn.stack.push_integer((int64_t)j.size());
  } else if (j.is_number_integer()) {
    rpn.stack.push_integer(j.get<int64_t>());
  } else if (j.is_number_float()) {
    rpn.stack.push_double(j.get<double>());
  } else if (j.is_boolean()) {
    rpn.stack.push_boolean(j.get<bool>());
  } else if (j.is_string()) {
    rpn.stack.push_string(j.get<std::string>());
  } else {
    rpn.stack.push(j);  // null or unrecognised — push as-is
  }
  return rpn::WordDefinition::Result::ok;
}

void
rpn::Interp::addTypeWords() {
  setWordCategory("types");
  addDefinition("TRUE",  NATIVE_WORD_WDEF(types, rpn::StackSizeValidator::zero, push_true,  nullptr));
  addDefinition("FALSE", NATIVE_WORD_WDEF(types, rpn::StackSizeValidator::zero, push_false, nullptr));
  addDefinition("->INT", NATIVE_WORD_WDEF(types, rpn::StrictTypeValidator::d1_double, to_int, nullptr));
  addDefinition("->FLOAT", NATIVE_WORD_WDEF(types, rpn::StrictTypeValidator::d1_integer, to_float, nullptr));
  addDefinition("->STRING", NATIVE_WORD_WDEF(types, rpn::StackSizeValidator::one, to_string, nullptr));

  addDefinition("->OBJ", NATIVE_WORD_WDEF(t_object, rpn::StrictTypeValidator::d2_any_string, to_object, nullptr));
  addDefinition("OBJ->", NATIVE_WORD_WDEF(t_object, rpn::StrictTypeValidator::d1_object, object_to, nullptr));
  addDefinition("->ARRAY", NATIVE_WORD_WDEF(t_array, rpn::StackSizeValidator::ntos, to_array, nullptr));
  addDefinition("OBJ->", NATIVE_WORD_WDEF(t_array, rpn::StrictTypeValidator::d1_array, array_to, nullptr));

  addDefinition("ARREV", NATIVE_WORD_WDEF(t_array, rpn::StrictTypeValidator::d1_array, reverse, nullptr));

  addDefinition("+", NATIVE_WORD_WDEF(t_object, rpn::StrictTypeValidator::d3_any_string_object, add_object_string_any, nullptr));
  addDefinition("+", NATIVE_WORD_WDEF(t_object, rpn::StrictTypeValidator::d3_object_any_string, add_string_any_object, nullptr));
  addDefinition("+", NATIVE_WORD_WDEF(t_array, rpn::StrictTypeValidator::d2_any_array, add_array_any, nullptr));
  addDefinition("+", NATIVE_WORD_WDEF(t_array, rpn::StrictTypeValidator::d2_array_any, add_any_array, nullptr));

  addDefinition("+", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_vec3_vec3, add_vec3, nullptr));
  addDefinition("+", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_double_vec3, add_vec3_num, nullptr));
  addDefinition("+", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_integer_vec3, add_vec3_num, nullptr));
  addDefinition("+", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_vec3_double, add_num_vec3, nullptr));
  addDefinition("+", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_vec3_integer, add_num_vec3, nullptr));

  addDefinition("-", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_vec3_vec3, sub_vec3, nullptr));
  addDefinition("-", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_double_vec3, sub_vec3_num, nullptr));
  addDefinition("-", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_integer_vec3, sub_vec3_num, nullptr));
  addDefinition("-", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_vec3_double, sub_num_vec3, nullptr));
  addDefinition("-", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d2_vec3_integer, sub_num_vec3, nullptr));

  ADD_NATIVE_3_NUMBER_WDEF(vec3, (*this), "->VEC3", to_vec3, to_vec3, nullptr);
  addDefinition("->VEC3x", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d1_number, to_vec3x, nullptr));
  addDefinition("->VEC3y", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d1_number, to_vec3y, nullptr));
  addDefinition("->VEC3z", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d1_number, to_vec3z, nullptr));

  addDefinition("VEC3->", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d1_vec3, vec3_to, nullptr));
  addDefinition("OBJ->", NATIVE_WORD_WDEF(vec3, rpn::StrictTypeValidator::d1_vec3, vec3_to, nullptr));

  addWordMetadata("TRUE",     "Push boolean true.");
  addWordMetadata("FALSE",    "Push boolean false.");
  addWordMetadata("->INT",    "Convert TOS to integer (rounded to nearest).");
  addWordMetadata("->FLOAT",  "Convert TOS to double.");
  addWordMetadata("->STRING", "Convert TOS to its string representation.");
  addWordMetadata("->OBJ",    "Create a single-field object.  `value \"key\" ->OBJ`");
  addWordMetadata("OBJ->",    "Explode an object, array, complex, fraction, or vec3 to its components.");
  addWordMetadata("->ARRAY",  "Collect the top n items into an array. n is on TOS.");
  addWordMetadata("ARREV",    "Reverse an array in-place.");
  addWordMetadata("+",        "Add, append, or merge: numbers, vec3 components, array + element, or object + field.");
  addWordMetadata("-",        "Subtract: numbers or vec3 components.");
  addWordMetadata("->VEC3",   "Create a 3D vector from three numbers (x, y, z).");
  addWordMetadata("->VEC3x",  "Create a VEC3 with only the X component set (Y and Z are NaN).");
  addWordMetadata("->VEC3y",  "Create a VEC3 with only the Y component set (X and Z are NaN).");
  addWordMetadata("->VEC3z",  "Create a VEC3 with only the Z component set (X and Y are NaN).");
  addWordMetadata("VEC3->",   "Explode a VEC3 to x, y, z doubles.");

  addDefinition("->JSON", NATIVE_WORD_WDEF(types, rpn::StackSizeValidator::one, to_json, nullptr));
  addDefinition("JSON->", NATIVE_WORD_WDEF(types, rpn::StackSizeValidator::one, json_to, nullptr));
  addWordMetadata("->JSON", "Convert TOS to a JSON value (stack::Json), using the type's data encoding.");
  addWordMetadata("JSON->", "Unpack a JSON value: array→elements+count, object→(val,key) pairs+count, scalar→native type.");
}

/* end of qinc/rpn-lang/src/types-dict.cpp */
