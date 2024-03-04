#include "RpnCalcForm.h"

#include <msclr/marshal_cppstd.h>
#include "rpncalcform-controller.h"

rpn_calc::Form::Form(System::String ^title) : _rpnkpc(nullptr) {
  InitializeComponent();
  if (title != L"") {
    this->Text = title;
  }
  createSoftButtons();

  _redrawDisplayDelegate = gcnew rpn_calc::Form::VoidDelegate(this, &Form::redrawDisplayMethod);
  _clearAssignedButtonsDelegate = gcnew rpn_calc::Form::VoidDelegate(this, &Form::clearAssignedButtonsMethod);
  _assignButtonDelegate = gcnew rpn_calc::Form::IISSDelegate(this, &Form::assignButtonMethod);
  _assignMenuDelegate = gcnew rpn_calc::Form::SSSDelegate(this, &Form::assignMenuMethod);
  _enableUiDelegate = gcnew rpn_calc::Form::BoolDelegate(this, &Form::enableUiMethod);

  _rpnkpc = new Controller(this);
}

rpn_calc::Form::~Form() {
  if (components) {
    delete components;
  }
  if (_rpnkpc) {
    delete _rpnkpc;
  }
}

void
rpn_calc::Form::createSoftButtons() {
  for(int col=5; col<tableLayoutPanel1->ColumnCount; col++) {
    for(int row=0; row<tableLayoutPanel1->RowCount; row++) {
      System::Windows::Forms::Control ^c = tableLayoutPanel1->GetControlFromPosition(col, row);
      if (c == nullptr) {
	System::Windows::Forms::Button^ sb = (gcnew System::Windows::Forms::Button());
	this->tableLayoutPanel1->Controls->Add(sb, col, row);
	sb->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(System::Windows::Forms::AnchorStyles::Top |
								       System::Windows::Forms::AnchorStyles::Bottom |
								       System::Windows::Forms::AnchorStyles::Left |
								       System::Windows::Forms::AnchorStyles::Right);
	sb->Size = System::Drawing::Size(198, 43);
	sb->Text = L"";
	sb->Name = L"_sb_" + col + "_" + row;
	sb->Tag = L"";
	sb->UseVisualStyleBackColor = true;
	sb->Click += gcnew System::EventHandler(this, &Form::_rpnWordButton_Click);
      }
    }
  }
}

void
rpn_calc::Form::redrawDisplayMethod() {
  std::string display = _rpnkpc->getStackAsString();
  _stackView->Text = gcnew String(display.c_str());

  // scroll to end
  _stackView->SelectionStart = _stackView->Text->Length;
  _stackView->ScrollToCaret();

  // update status
  std::string status = _rpnkpc->getStatusAsString();
  _statusLabel->Text = gcnew String(status.c_str());
}

void
rpn_calc::Form::clearAssignedButtonsMethod() {
  // this might be called before the objects are created
  if (tableLayoutPanel1 != nullptr) {
    for(int col=5; col<tableLayoutPanel1->ColumnCount; col++) {
      for(int row=0; row<tableLayoutPanel1->RowCount; row++) {
	System::Windows::Forms::Control ^c = tableLayoutPanel1->GetControlFromPosition(col, row);
	if (c!=nullptr && c->Name->StartsWith(L"_sb_")) {
	  c->Text = L"";
	  c->Tag = L"";
	}
      }
    }
  }
}

void
rpn_calc::Form::assignButtonMethod(int col, int row, System::String ^rpnword, System::String ^label) {
  if (tableLayoutPanel1 != nullptr) {
    // custom buttons start at col 5 and are 1-based indexed
    System::Windows::Forms::Control ^c = tableLayoutPanel1->GetControlFromPosition(col+4, row-1);
    if (c!=nullptr && c->Name->StartsWith(L"_sb_")) {
      c->Tag = rpnword;
      c->Text = label==L"" ? rpnword : label;
    }
  }
}

void
rpn_calc::Form::assignMenuMethod(System::String ^menu, System::String ^rpnword, System::String ^label) {
  cli::array<ToolStripItem^> ^top_mia = menuStrip1->Items->Find(menu + L"ToolStripMenuItem", false);
  System::Windows::Forms::ToolStripMenuItem ^top_mi = nullptr;
  if (top_mia->Length > 0) {
    top_mi = (System::Windows::Forms::ToolStripMenuItem ^)top_mia[0];
  } else {
    top_mi = (gcnew System::Windows::Forms::ToolStripMenuItem());
    top_mi->Name = menu + L"ToolStripMenuItem";
    //    top_mi->Size = System::Drawing::Size(71, 38);
    top_mi->Text = menu;
    this->menuStrip1->Items->Add(top_mi);
  }

  System::String ^name = (label != L"") ? label : rpnword;
  array<ToolStripItem^> ^mia = top_mi->DropDownItems->Find(L"menu_" + name, false);

  System::Windows::Forms::ToolStripMenuItem^ mi = nullptr;
  if (mia->Length == 0) {
    mi = (gcnew System::Windows::Forms::ToolStripMenuItem());
    //    mi->Size = System::Drawing::Size(71, 38);
    mi->Click += gcnew System::EventHandler(this, &Form::_rpnWordMenu_Click);
    top_mi->DropDownItems->Add(mi);
  } else {
    mi = (System::Windows::Forms::ToolStripMenuItem^)mia[0];
  }

  mi->Text = name;
  mi->Name = L"menu_" + name;
  mi->Tag = rpnword;
}

void
rpn_calc::Form::enableUiMethod(bool pred) {
}

rpn::WordDefinition::Result
rpn_calc::Form::pushEntry() {
  std::string line = msclr::interop::marshal_as<std::string>(_commandEntry->Text);
  if (line != "") {
    _rpnkpc->eval(line);
    _commandEntry->Text = "";
  }
  return rpn::WordDefinition::Result::ok;
}

System::Void
rpn_calc::Form::_rpnWordButton_Click(System::Object^ sender, System::EventArgs^ e) {
  System::Windows::Forms::Control^ c = (System::Windows::Forms::Control^)sender;
  if (pushEntry()==rpn::WordDefinition::Result::ok) {
    std::string word = msclr::interop::marshal_as<std::string>((System::String^)c->Tag);
    _rpnkpc->eval(word);
  }
}

System::Void
rpn_calc::Form::_rpnWordMenu_Click(System::Object^ sender, System::EventArgs^ e) {
  System::Windows::Forms::ToolStripItem^ i = (System::Windows::Forms::ToolStripItem^)sender;
  if (pushEntry()==rpn::WordDefinition::Result::ok) {
    std::string word = msclr::interop::marshal_as<std::string>((System::String^)i->Tag);
    _rpnkpc->eval(word);
  }
}

void
rpn_calc::Form::insertString(System::String ^cc) {
  _commandEntry->AppendText(cc);
}

System::Void
rpn_calc::Form::_hbBack_Click(System::Object^ sender, System::EventArgs^ e) {
  if (_commandEntry->Text == "") {
    _rpnkpc->eval("DROP");
  } else {
    _commandEntry->Text = _commandEntry->Text->Substring(0, _commandEntry->TextLength - 1);
  }
}

System::Void
rpn_calc::Form::_hbEnter_Click(System::Object^ sender, System::EventArgs^ e) {
  if (_commandEntry->Text == "") {
    _rpnkpc->eval("DUP");
  } else {
    pushEntry();
  }
}

rpn_calc::Controller::Controller(Form^ f) : _f(f) {
  add_words(_rpn);
  assignMenu("Keys", "stack-keys", "Stack");
  assignMenu("Keys", "math-keys", "Math");
  assignMenu("Keys", "logic-keys", "Logic");
  assignMenu("Keys", "type-keys", "Types");
  _rpn.eval("math-keys");
}

rpn_calc::Controller::~Controller() {
  remove_words(_rpn);
}

void
rpn_calc::Controller::assignButton(unsigned column, unsigned row, const std::string &rpnword, const std::string &label) {
  _f->assignButton(column, row, rpnword, label);
}

void
rpn_calc::Controller::assignMenu(const std::string &menu, const std::string &rpnword, const std::string &label) {
  _f->assignMenu(menu, rpnword, label);
}

void
rpn_calc::Controller::clearAssignedButtons() {
  _f->clearAssignedButtons();
}

void
rpn_calc::Controller::enable(bool pred) { // enables/disables the keypad buttons
  _f->enableUI(pred);
}

void
rpn_calc::Controller::eval(std::string command) {
  _rpn.eval(command, [this](rpn::WordDefinition::Result res) -> void {
      _f->redrawDisplay();
    });
}

std::string
rpn_calc::Controller::getStackAsString() {
  std::string display;
  for(size_t i=_rpn.stack.depth(); i!=0; i--) {
    char level[32];
    snprintf(level, sizeof(level), " : %02d%s", i, i>1?"\r\n":"");
    auto so = _rpn.stack.peek_as_string(i);
    display += so;
    display += level;
  }
  return display;
}

std::string
rpn_calc::Controller::getStatusAsString() {
  return _rpn.status();
}
