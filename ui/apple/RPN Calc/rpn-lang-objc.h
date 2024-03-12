/***************************************************
 * file: QInc/Projects/color-calc/src/libs/rpn-lang/ui/apple/RPN Calc/rpn-lang-objc.h
 *
 * @file    rpn-lang-objc.h
 * @author  Eric L. Hernes
 * @born_on   Monday, March 11, 2024
 * @copyright (C) Copyright Eric L. Hernes 2024
 * @copyright (C) Copyright Q, Inc. 2024
 *
 * @brief   An Eric L. Hernes Signature Series C/C++ header
 *
 * $Id$
 */

#pragma once

#include <Foundation/Foundation.h>

@class RpnCalcUi;

@interface RpnCalcController : NSObject
- (id) initWithUi:(RpnCalcUi*)ui;

- (void) eval:(NSString*) line; // , std::function<void(rpn::WordDefinition::Result)>completionHandler=nullCompletionHandler);
- (void) parseFile:(NSString*) path; // , std::function<void(rpn::WordDefinition::Result)>completionHandler=nullCompletionHandler);

//    bool addDefinition(const std::string &word, const WordDefinition &def);
//    bool removeDefinition(const std::string &word);
//    bool addCompiledWord(const std::string &word, const std::string &def, const StackValidator &v = StackSizeValidator::zero);

- (BOOL) validateWord:(NSString *)word;
- (BOOL) wordExists:(NSString *)word;
- (NSString*) status;
- (NSString*) stackAsString;
@end


/* end of QInc/Projects/color-calc/src/libs/rpn-lang/ui/apple/RPN Calc/rpn-lang-objc.h */
