/***************************************************
 * file: QInc/Projects/color-calc/src/libs/rpn-lang/ui/win/rpncalcform-controller.h
 *
 * @file    rpncalcform-controller.h
 * @author  Eric L. Hernes
 * @born_on   Sunday, March 3, 2024
 * @copyright (C) Copyright Eric L. Hernes 2024
 * @copyright (C) Copyright Q, Inc. 2024
 *
 * @brief   An Eric L. Hernes Signature Series C/C++ header
 *
 * $Id$
 */

#pragma once

#include <vcclr.h>
#include "../../rpn.h"

namespace rpn_calc {
    ref class Form;
    class Controller : public rpn::KeypadController {
    public:
	Controller(Form^ f);
	~Controller();
        virtual void assignButton(unsigned column, unsigned row, const std::string& rpnword, const std::string& label = "") override;
        virtual void assignMenu(const std::string& menu, const std::string& rpnword, const std::string& label = "") override;
        virtual void clearAssignedButtons() override;
        virtual void enable(bool pred) override; // enables/disables the keypad buttons
        void eval(std::string command);

      std::string getStackAsString();
      std::string getStatusAsString();
      rpn::Interp &rpnInterp() { return _rpn; }
    private:
        msclr::gcroot<Form^> _f;
        rpn::Interp _rpn;
    };
}

/* end of QInc/Projects/color-calc/src/libs/rpn-lang/ui/win/rpncalcform-controller.h */
