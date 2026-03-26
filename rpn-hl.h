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
#include "rpn.h"  // for rpn::WordHelp

namespace rpn {
  class Interp;
}

#if __OBJC__

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

// Word introspection
- (NSDictionary*) wordHelp:(NSString*)word;   // keys: name, description, category, effects (NSArray<NSString*>)
- (NSArray<NSString*>*) wordList;
@end

#else // ! __OBJC__

/* C++ version */
class RpnInterp {
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
  RpnInterp(bool async);
  ~RpnInterp();
  static void nullCompletionHandler(Result) {};
  void eval(std::string line, std::function<void(Result)>completionHandler=nullCompletionHandler);
  void parseFile(const std::string &path, std::function<void(Result)>completionHandler=nullCompletionHandler);

  //  rpn::Interp &rpnInterp() const { return *_interp; }

  bool validateWord(const std::string &word);
  bool wordExists(const std::string &word);

  std::string status();
  std::vector<std::string> displayStack();

  rpn::WordHelp wordHelp(const std::string &word) const;
  std::vector<std::string> wordList() const;

private:
  rpn::Interp *_interp;
};

#endif // __OBJC__

/* end of QInc/Projects/RP42/rpn-lang/rpn_hl.h */
