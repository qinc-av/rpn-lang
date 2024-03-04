
using namespace System;

// int main(array<System::String ^> ^args)
// {
//    return 0;
// }

#include "RpnCalcForm.h"
#include "rpncalcform-controller.h"

using namespace System::Windows::Forms;

[STAThread]
int main()
{
  
  Application::EnableVisualStyles();
  Application::SetCompatibleTextRenderingDefault(false);

  rpn_calc::Form ^rpnForm = gcnew rpn_calc::Form(L"RPN Calc");
  rpn_calc::Controller *rpnkpc = rpnForm->getRpnKeypadController();
  rpn::Interp &rpn = rpnkpc->rpnInterp();

  // add any dictionaries, menus, etc here...

  // then Run
  Application::Run(rpnForm);

  return 0;
}
