#!env python

validators = [
    "rpn::StrictTypeValidator::d1_double",
    "rpn::StrictTypeValidator::d1_integer",
    "rpn::StrictTypeValidator::d1_boolean",
    "rpn::StrictTypeValidator::d1_object",
    "rpn::StrictTypeValidator::d1_string",
    "rpn::StrictTypeValidator::d1_array",
    "rpn::StrictTypeValidator::d1_vec3",
    "rpn::StrictTypeValidator::d2_vec3_vec3",
    "rpn::StrictTypeValidator::d2_double_double",
    "rpn::StrictTypeValidator::d2_double_integer",
    "rpn::StrictTypeValidator::d2_integer_double",
    "rpn::StrictTypeValidator::d2_integer_integer",
    "rpn::StrictTypeValidator::d2_boolean_boolean",
    "rpn::StrictTypeValidator::d2_vec3_double",
    "rpn::StrictTypeValidator::d2_double_vec3",
    "rpn::StrictTypeValidator::d2_vec3_integer",
    "rpn::StrictTypeValidator::d2_integer_vec3",
    "rpn::StrictTypeValidator::d2_array_any",
    "rpn::StrictTypeValidator::d2_any_array",
    "rpn::StrictTypeValidator::d2_string_any",
    "rpn::StrictTypeValidator::d2_any_string",
    "rpn::StrictTypeValidator::d2_object_any",
    "rpn::StrictTypeValidator::d2_any_object",
    "rpn::StrictTypeValidator::d3_double_double_double",
    "rpn::StrictTypeValidator::d3_integer_double_double",
    "rpn::StrictTypeValidator::d3_double_integer_double",
    "rpn::StrictTypeValidator::d3_double_double_integer",
    "rpn::StrictTypeValidator::d3_integer_integer_integer",
    "rpn::StrictTypeValidator::d3_double_integer_integer",
    "rpn::StrictTypeValidator::d3_integer_double_integer",
    "rpn::StrictTypeValidator::d3_integer_integer_double",
    "rpn::StrictTypeValidator::d3_any_any_boolean",
    "rpn::StrictTypeValidator::d3_object_string_any",
    "rpn::StrictTypeValidator::d3_string_any_object",
    "rpn::StrictTypeValidator::d4_double_double_double_integer",
    "rpn::StrictTypeValidator::d4_integer_double_double_double",
    "timecode_validator::d1_tc",
    "timecode_validator::d2_tc_tc",
    "timecode_validator::d2_int_tc",
    "timecode_validator::d2_tc_int",
    "frac_validator::d1_frac",
    "frac_validator::d2_frac_frac",
    "frac_validator::d2_frac_int",
    "frac_validator::d2_frac_double",
    "frac_validator::d2_int_frac",
    "frac_validator::d2_double_frac",
    "frac_validator::d5_int_int_int_int_frac",
]

print("static const std::map<std::pair<rpn::StrictTypeValidator,rpn::StrictTypeValidator>, bool> sk_validatorTests = {");

for v1 in validators:
    for v2 in validators:
        print("  { { " + v1 + ", " + v2 + " }, " + ("true" if (v1==v2) else "false") + " }, ")

print("};\n")
