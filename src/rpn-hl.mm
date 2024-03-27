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
  _rpn = new rpn::Interp;
  return self;
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

- (NSArray<NSString*>*) stackItems {
  NSMutableArray<NSString*> *si = [[NSMutableArray<NSString*> alloc] init];
  for(size_t i=_rpn->stack.depth(); i!=0; i--) {
    auto so = _rpn->stack.peek_as_string((int)i);
    [si addObject:@(so.c_str())];
  }
  return si;
}
@end

/***********************************************************************
 * C++ version
 */
interp::interp() : _interp(new rpn::Interp) {
}
interp::~interp() {
  delete _interp;
}

void
interp::eval(std::string line, std::function<void(Result)>completionHandler) {
  _interp->eval(line, [&](rpn::WordDefinition::Result r1) {
      completionHandler((Result)r1);
    });
}

void
interp::parseFile(const std::string &path, std::function<void(Result)>completionHandler) {
  _interp->parseFile(path, [&](rpn::WordDefinition::Result r1) {
      completionHandler((Result)r1);
    });
}

std::vector<std::string>
interp::stackItems() {
  std::vector<std::string> rv;
  size_t n = _interp->stack.depth();
  for(size_t i=0; i<n; i++) {
    rv.push_back(_interp->stack.peek_as_string(i));
  }
  return rv;
}

/* end of QInc/Projects/RP42/rpn-lang/src/rpn-hl.mm */
