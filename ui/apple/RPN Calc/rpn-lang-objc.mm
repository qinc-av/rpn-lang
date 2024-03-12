/***************************************************
 * file: QInc/Projects/color-calc/src/libs/rpn-lang/ui/apple/RPN Calc/rpn-lang-objc.mm
 *
 * (C) Copyright Eric L. Hernes 2024
 * (C) Copyright Q, Inc. 2024
 *
 * An Eric L. Hernes Signature Series
 *
 * Objective C++ module for something
 *
 * $Id$
 *
 */

#include "../../../rpn.h"
#include "rpn-lang-objc.h"
#include "RPN_Calc-Swift.h"

@implementation RpnCalcController {
  rpn::Interp *_rpn;
  RpnCalcUi *_ui;
}

- (id) initWithUi:(RpnCalcUi*)ui {
  self = [super init];
  _ui = ui;
  _rpn = new rpn::Interp;
  return self;
}

- (void) eval:(NSString*) nsline { // , std::function<void(rpn::WordDefinition::Result)>completionHandler=nullCompletionHandler);
  std::string line = [nsline UTF8String];
  _rpn->eval(line, [self](rpn::WordDefinition::Result res) -> void {
      dispatch_async(dispatch_get_main_queue(),^(void){
	  [self->_ui commandFinishedWithResult:(int)res];
	});
    });
}

- (void) parseFile:(NSString*) nspath { // , std::function<void(rpn::WordDefinition::Result)>completionHandler=nullCompletionHandler);
  std::string path = [nspath UTF8String];
  _rpn->parseFile(path, [&](rpn::WordDefinition::Result res) -> void {
    });
}


//    bool addDefinition(const std::string &word, const WordDefinition &def);
//    bool removeDefinition(const std::string &word);
//    bool addCompiledWord(const std::string &word, const std::string &def, const StackValidator &v = StackSizeValidator::zero);

- (BOOL) validateWord:(NSString *)nsword {
  std::string word = [nsword UTF8String];
  return _rpn->validateWord(word) ? YES : NO;
}

- (BOOL) wordExists:(NSString *)nsword {
  std::string word = [nsword UTF8String];
  return _rpn->wordExists(word) ? YES : NO;
}

- (NSString*) status {
  auto ss = _rpn->status();
  return [NSString stringWithUTF8String:ss.c_str()];
}

- (NSString*) stackAsString {
  std::string stackDisplay;
  for(size_t i=_rpn->stack.depth(); i!=0; i--) {
    char level[32];
    snprintf(level, sizeof(level), " : %02d%s", i, i>1?"\n":"");
    auto so = _rpn->stack.peek_as_string(i);
    stackDisplay += so;
    stackDisplay += level;
  }
  return [NSString stringWithUTF8String:stackDisplay.c_str()];
}

@end

/* end of QInc/Projects/color-calc/src/libs/rpn-lang/ui/apple/RPN Calc/rpn-lang-objc.mm */
