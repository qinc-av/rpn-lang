/***************************************************
 * file: QInc/Projects/RP42/rpn-lang/src/rpn-hl.mm
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

#include "../rpn.h"
#include "../rpn-hl.h"

@implementation RpnInterp {
  rpn::Interp *_rpn;
}

- (id) init {
  self = [super init];
  _rpn = new rpn::Interp(true);
  return self;
}

- (rpn::Interp &) rpnInterp {
  return *_rpn;
}

- (void) eval:(NSString*)nsline completionHandler:(void(^)(RpnResult result))completionHandler {
  std::string line = [nsline UTF8String];
  _rpn->eval(line, [=](rpn::WordDefinition::Result res) -> void {
      dispatch_async(dispatch_get_main_queue(),^(void){completionHandler((RpnResult)res);});
      //      completionHandler((RpnResult)res);
    });
}

- (void) parseFile:(NSString*) nspath completionHandler:(void(^)(RpnResult result))completionHandler {
  std::string path = [nspath UTF8String];
  _rpn->parseFile(path, [=](rpn::WordDefinition::Result res) -> void {
      dispatch_async(dispatch_get_main_queue(),^(void){completionHandler((RpnResult)res);});
    });
}

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
  return @(ss.c_str());
}

- (NSArray<NSString*>*) displayStack {
  NSMutableArray<NSString*> *si = [[NSMutableArray<NSString*> alloc] init];
  for(size_t i=0; i<_rpn->stack.depth(); i++) {
    auto disp = _rpn->stack.peek_for_display((int)(i+1));
    [si addObject:@(disp.c_str())];
  }
  return si;
}
@end

/* end of QInc/Projects/RP42/rpn-lang/src/rpn-hl.mm */
