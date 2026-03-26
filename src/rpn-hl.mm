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

- (NSDictionary*) wordHelp:(NSString*)nsword {
  auto h = _rpn->wordHelp([nsword UTF8String]);
  NSMutableArray<NSString*> *effects = [[NSMutableArray<NSString*> alloc] init];
  for (const auto &e : h.effects) [effects addObject:@(e.c_str())];
  return @{
    @"name":        @(h.name.c_str()),
    @"description": @(h.description.c_str()),
    @"category":    @(h.category.c_str()),
    @"effects":     effects,
  };
}

- (NSArray<NSString*>*) wordList {
  NSMutableArray<NSString*> *list = [[NSMutableArray<NSString*> alloc] init];
  for (const auto &w : _rpn->wordList()) [list addObject:@(w.c_str())];
  return list;
}

- (void) cancel    { _rpn->cancel(); }
- (void) cancelAll { _rpn->cancelAll(); }
- (BOOL) isCancelled { return _rpn->isCancelled() ? YES : NO; }

- (void) setProgressHandler:(void(^)(NSString *, double))handler {
  if (handler) {
    _rpn->setProgressHandler([handler](const std::string &msg, double fraction) {
      NSString *nsmsg = @(msg.c_str());
      dispatch_async(dispatch_get_main_queue(), ^{ handler(nsmsg, fraction); });
    });
  } else {
    _rpn->setProgressHandler(nullptr);
  }
}

- (void) reportProgress:(NSString *)message fraction:(double)fraction {
  _rpn->reportProgress([message UTF8String], fraction);
}
@end

/* end of QInc/Projects/RP42/rpn-lang/src/rpn-hl.mm */
