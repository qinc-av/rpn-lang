/***************************************************
 * file: QInc/Projects/RP42/rpn-lang/rpn_hl.h
 *
 * @file    rpn_hl.h
 * @author  Eric L. Hernes
 * @born_on   Monday, March 25, 2024
 * @copyright (C) Copyright Eric L. Hernes 2024
 * @copyright (C) Copyright Q, Inc. 2024
 *
 * @brief   An Eric L. Hernes Signature Series C/C++ header
 *
 * $Id$
 */

#pragma once

#include <string>
#include <vector>
#include <functional>

namespace rpn {
  class Interp;
}

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, RpnResult) {
  rpn_ok,
  rpn_parse_error, // parsing problem, definition, comment, string-literal, etc
  rpn_dict_error, // no such word
  rpn_param_error, // parameters not right for the word
  rpn_eval_error, // eval went awry
  rpn_compile_error, // error in compiling
  rpn_implementation_error, // not implmemented or similar
};

@interface RpnInterp : NSObject
- (id) init;
- (void) eval:(NSString*)line completionHandler:(void(^)(RpnResult result))completionHandler;
- (void) parseFile:(NSString*) path completionHandler:(void(^)(RpnResult result))completionHandler;

- (BOOL) validateWord:(NSString *)word;
- (BOOL) wordExists:(NSString *)word;
- (NSString*) status;
- (NSArray<NSString*>*) displayStack;
- (rpn::Interp &) rpnInterp;
@end

/* C++ version */
class interp {
 public:
  enum class Result {
    ok,
      parse_error, // parsing problem, definition, comment, string-literal, etc
      dict_error, // no such word
      param_error, // parameters not right for the word
      eval_error, // eval went awry
      compile_error, // error in compiling
      implementation_error, // not implmemented or similar
      };
  interp();
  ~interp();
  static void nullCompletionHandler(Result) {};
  void eval(std::string line, std::function<void(Result)>completionHandler=nullCompletionHandler);
  void parseFile(const std::string &path, std::function<void(Result)>completionHandler=nullCompletionHandler);
  std::vector<std::string> stackItems();
  rpn::Interp &rpnInterp() const { return *_interp; }
 private:
  rpn::Interp *_interp;
} SWIFT_IMMORTAL_REFERENCE;

/* end of QInc/Projects/RP42/rpn-lang/rpn_hl.h */
