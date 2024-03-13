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

class RCCKeypadController : public rpn::KeypadController {
public:
  RCCKeypadController(RpnCalcController *rcc, RpnCalcUi *ui) : _rcc(rcc), _ui(ui) {
    add_words(_rpn);
    assignMenu("Keys", "stack-keys", "Stack");
    assignMenu("Keys", "math-keys", "Math");
    assignMenu("Keys", "logic-keys", "Logic");
    assignMenu("Keys", "type-keys", "Types");
    _rpn.eval("math-keys");
  }

  ~RCCKeypadController() {
    remove_words(_rpn);
  }

  virtual void assignButton(unsigned column, unsigned row, const std::string &rpnword, const std::string &label="") override {
    dispatch_async(dispatch_get_main_queue(), ^{
	NSString *w = [NSString stringWithUTF8String:rpnword.c_str()];
	NSString *l = (label!="") ? [NSString stringWithUTF8String:label.c_str()] : w;
	[_ui assignButtonWithCol:column row:row rpnword:w label:l];
      });
  }

  virtual void assignMenu(const std::string &menu, const std::string &rpnword, const std::string &label="") override {
  }

  virtual void clearAssignedButtons() override {
    dispatch_async(dispatch_get_main_queue(), ^{
	[_ui clearAssignedButtons];
      });
  }

  virtual void enable(bool pred) override {
    //  XXX-ELH: implement this.
  }

  RpnCalcController *_rcc;
  RpnCalcUi *_ui;
  rpn::Interp _rpn;
};

@implementation RpnCalcController {
  RCCKeypadController *_rkc;
  RpnCalcUi *_ui;
}

- (id) initWithUi:(RpnCalcUi*)ui {
  self = [super init];
  _ui = ui;
  _rkc = new RCCKeypadController(self, _ui);
  return self;
}

- (void) eval:(NSString*) nsline { // , std::function<void(rpn::WordDefinition::Result)>completionHandler=nullCompletionHandler);
  std::string line = [nsline UTF8String];
  _rkc->_rpn.eval(line, [self](rpn::WordDefinition::Result res) -> void {
      dispatch_async(dispatch_get_main_queue(),^(void){
	  [self->_ui commandFinishedWithResult:(int)res];
	});
    });
}

- (void) parseFile:(NSString*) nspath { // , std::function<void(rpn::WordDefinition::Result)>completionHandler=nullCompletionHandler);
  std::string path = [nspath UTF8String];
  _rkc->_rpn.parseFile(path, [&](rpn::WordDefinition::Result res) -> void {
    });
}


//    bool addDefinition(const std::string &word, const WordDefinition &def);
//    bool removeDefinition(const std::string &word);
//    bool addCompiledWord(const std::string &word, const std::string &def, const StackValidator &v = StackSizeValidator::zero);

- (BOOL) validateWord:(NSString *)nsword {
  std::string word = [nsword UTF8String];
  return _rkc->_rpn.validateWord(word) ? YES : NO;
}

- (BOOL) wordExists:(NSString *)nsword {
  std::string word = [nsword UTF8String];
  return _rkc->_rpn.wordExists(word) ? YES : NO;
}

- (NSString*) status {
  auto ss = _rkc->_rpn.status();
  return [NSString stringWithUTF8String:ss.c_str()];
}

- (NSString*) stackAsString {
  std::string stackDisplay;
  for(size_t i=_rkc->_rpn.stack.depth(); i!=0; i--) {
    char level[32];
    snprintf(level, sizeof(level), " : %02d%s", i, i>1?"\n":"");
    auto so = _rkc->_rpn.stack.peek_as_string(i);
    stackDisplay += so;
    stackDisplay += level;
  }
  return [NSString stringWithUTF8String:stackDisplay.c_str()];
}

@end

/* end of QInc/Projects/color-calc/src/libs/rpn-lang/ui/apple/RPN Calc/rpn-lang-objc.mm */
