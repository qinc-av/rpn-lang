#pragma once

#include "../../rpn.h"

namespace rpn_calc {
  class Controller;
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	/// </summary>
				       public ref class Form : public System::Windows::Forms::Form
	{
	public:
	  Form(System::String ^appTitle);

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
	  ~Form();

	private: Controller *_rpnkpc;
	public: Controller *getRpnKeypadController() { return _rpnkpc; }
	private: System::Windows::Forms::ToolStripMenuItem^ openToolStripMenuItem;
	protected:
	private: System::Windows::Forms::ToolStripMenuItem^ saveStackToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ restoreStackToolStripMenuItem;

	private: delegate void VoidDelegate();
	private: VoidDelegate ^_redrawDisplayDelegate;
	private: void redrawDisplayMethod();

	private: VoidDelegate ^_clearAssignedButtonsDelegate;
	private: void clearAssignedButtonsMethod();

	private: delegate void IISSDelegate(int,int,System::String ^rpnword, System::String ^label);
	private: IISSDelegate ^_assignButtonDelegate;
	private: void assignButtonMethod(int col, int row, System::String ^rpnword, System::String ^label);

	private: delegate void SSSDelegate(System::String ^menu, System::String ^rpnword, System::String ^label);
	private: SSSDelegate ^_assignMenuDelegate;
	private: void assignMenuMethod(System::String ^menu, System::String ^rpnword, System::String ^label);

	private: delegate void BoolDelegate(bool p);
	private: BoolDelegate ^_enableUiDelegate;
	private: void enableUiMethod(bool pred);

	private: System::Windows::Forms::MenuStrip^ menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^ fileToolStripMenuItem;

	private: System::Windows::Forms::ToolStripMenuItem^ keysToolStripMenuItem;

	private: System::Windows::Forms::TableLayoutPanel^ tableLayoutPanel1;
	private: System::Windows::Forms::Label^ _statusLabel;

	private: System::Windows::Forms::TextBox^ _stackView;

	private: System::Windows::Forms::TextBox^ _commandEntry;

    private: System::Windows::Forms::Button^ _hbOver;

    private: System::Windows::Forms::Button^ _hbMultiply;

    private: System::Windows::Forms::Button^ _hb0;
    private: System::Windows::Forms::Button^ _hb1;
    private: System::Windows::Forms::Button^ _hb2;
    private: System::Windows::Forms::Button^ _hb3;
    private: System::Windows::Forms::Button^ _hb4;
    private: System::Windows::Forms::Button^ _hb5;
    private: System::Windows::Forms::Button^ _hb6;
    private: System::Windows::Forms::Button^ _hb8;
    private: System::Windows::Forms::Button^ _hb7;
    private: System::Windows::Forms::Button^ _hb9;
    private: System::Windows::Forms::Button^ _hbDot;

    private: System::Windows::Forms::Button^ _hbClear;

    private: System::Windows::Forms::Button^ _hbDivide;

    private: System::Windows::Forms::Button^ _hbBack;

    private: System::Windows::Forms::Button^ _hbEnter;

    private: System::Windows::Forms::Button^ _hbPlus;

    private: System::Windows::Forms::Button^ _hbNegate;

    private: System::Windows::Forms::Button^ _hbSwap;

    private: System::Windows::Forms::Button^ _hbMinus;


	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->keysToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tableLayoutPanel1 = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->_hbEnter = (gcnew System::Windows::Forms::Button());
			this->_hbPlus = (gcnew System::Windows::Forms::Button());
			this->_hbNegate = (gcnew System::Windows::Forms::Button());
			this->_hb0 = (gcnew System::Windows::Forms::Button());
			this->_hbDot = (gcnew System::Windows::Forms::Button());
			this->_hbSwap = (gcnew System::Windows::Forms::Button());
			this->_hbMinus = (gcnew System::Windows::Forms::Button());
			this->_hb3 = (gcnew System::Windows::Forms::Button());
			this->_hb2 = (gcnew System::Windows::Forms::Button());
			this->_hb1 = (gcnew System::Windows::Forms::Button());
			this->_hbOver = (gcnew System::Windows::Forms::Button());
			this->_hbMultiply = (gcnew System::Windows::Forms::Button());
			this->_hb6 = (gcnew System::Windows::Forms::Button());
			this->_hb5 = (gcnew System::Windows::Forms::Button());
			this->_hb4 = (gcnew System::Windows::Forms::Button());
			this->_hbClear = (gcnew System::Windows::Forms::Button());
			this->_hbDivide = (gcnew System::Windows::Forms::Button());
			this->_hb9 = (gcnew System::Windows::Forms::Button());
			this->_hb8 = (gcnew System::Windows::Forms::Button());
			this->_hb7 = (gcnew System::Windows::Forms::Button());
			this->_stackView = (gcnew System::Windows::Forms::TextBox());
			this->_commandEntry = (gcnew System::Windows::Forms::TextBox());
			this->_hbBack = (gcnew System::Windows::Forms::Button());
			this->_statusLabel = (gcnew System::Windows::Forms::Label());
			this->openToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->saveStackToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->restoreStackToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->menuStrip1->SuspendLayout();
			this->tableLayoutPanel1->SuspendLayout();
			this->SuspendLayout();
			// 
			// menuStrip1
			// 
			this->menuStrip1->GripMargin = System::Windows::Forms::Padding(2, 2, 0, 2);
			this->menuStrip1->ImageScalingSize = System::Drawing::Size(32, 32);
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
				this->fileToolStripMenuItem,
					this->keysToolStripMenuItem
			});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(1654, 42);
			this->menuStrip1->TabIndex = 3;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
				this->openToolStripMenuItem,
					this->saveStackToolStripMenuItem, this->restoreStackToolStripMenuItem
			});
			this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
			this->fileToolStripMenuItem->Size = System::Drawing::Size(71, 38);
			this->fileToolStripMenuItem->Text = L"File";
			// 
			// keysToolStripMenuItem
			// 
			this->keysToolStripMenuItem->Name = L"keysToolStripMenuItem";
			this->keysToolStripMenuItem->Size = System::Drawing::Size(83, 38);
			this->keysToolStripMenuItem->Text = L"Keys";
			// 
			// tableLayoutPanel1
			// 
			this->tableLayoutPanel1->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->tableLayoutPanel1->ColumnCount = 9;
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				11.111F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				11.111F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				11.111F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				11.111F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				11.111F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				11.111F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				11.111F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				11.111F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				11.111F)));
			this->tableLayoutPanel1->Controls->Add(this->_hbEnter, 4, 10);
			this->tableLayoutPanel1->Controls->Add(this->_hbPlus, 3, 10);
			this->tableLayoutPanel1->Controls->Add(this->_hbNegate, 2, 10);
			this->tableLayoutPanel1->Controls->Add(this->_hb0, 1, 10);
			this->tableLayoutPanel1->Controls->Add(this->_hbDot, 0, 10);
			this->tableLayoutPanel1->Controls->Add(this->_hbSwap, 4, 9);
			this->tableLayoutPanel1->Controls->Add(this->_hbMinus, 3, 9);
			this->tableLayoutPanel1->Controls->Add(this->_hb3, 2, 9);
			this->tableLayoutPanel1->Controls->Add(this->_hb2, 1, 9);
			this->tableLayoutPanel1->Controls->Add(this->_hb1, 0, 9);
			this->tableLayoutPanel1->Controls->Add(this->_hbOver, 4, 8);
			this->tableLayoutPanel1->Controls->Add(this->_hbMultiply, 3, 8);
			this->tableLayoutPanel1->Controls->Add(this->_hb6, 2, 8);
			this->tableLayoutPanel1->Controls->Add(this->_hb5, 1, 8);
			this->tableLayoutPanel1->Controls->Add(this->_hb4, 0, 8);
			this->tableLayoutPanel1->Controls->Add(this->_hbClear, 4, 7);
			this->tableLayoutPanel1->Controls->Add(this->_hbDivide, 3, 7);
			this->tableLayoutPanel1->Controls->Add(this->_hb9, 2, 7);
			this->tableLayoutPanel1->Controls->Add(this->_hb8, 1, 7);
			this->tableLayoutPanel1->Controls->Add(this->_hb7, 0, 7);
			this->tableLayoutPanel1->Controls->Add(this->_stackView, 0, 0);
			this->tableLayoutPanel1->Controls->Add(this->_commandEntry, 0, 6);
			this->tableLayoutPanel1->Controls->Add(this->_hbBack, 4, 6);
			this->tableLayoutPanel1->Location = System::Drawing::Point(0, 43);
			this->tableLayoutPanel1->Name = L"tableLayoutPanel1";
			this->tableLayoutPanel1->RowCount = 11;
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 9.09F)));
			this->tableLayoutPanel1->Size = System::Drawing::Size(1654, 564);
			this->tableLayoutPanel1->TabIndex = 5;
			this->tableLayoutPanel1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form::tableLayoutPanel1_Paint);
			// 
			// _hbEnter
			// 
			this->_hbEnter->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->tableLayoutPanel1->SetColumnSpan(this->_hbEnter, 2);
			this->_hbEnter->Location = System::Drawing::Point(735, 513);
			this->_hbEnter->Name = L"_hbEnter";
			this->_hbEnter->Size = System::Drawing::Size(360, 48);
			this->_hbEnter->TabIndex = 62;
			this->_hbEnter->Text = L"ENTER";
			this->_hbEnter->UseVisualStyleBackColor = true;
			this->_hbEnter->Click += gcnew System::EventHandler(this, &Form::_hbEnter_Click);
			// 
			// _hbPlus
			// 
			this->_hbPlus->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbPlus->Location = System::Drawing::Point(552, 513);
			this->_hbPlus->Name = L"_hbPlus";
			this->_hbPlus->Size = System::Drawing::Size(177, 48);
			this->_hbPlus->TabIndex = 61;
			this->_hbPlus->Tag = L"+";
			this->_hbPlus->Text = L"+";
			this->_hbPlus->UseVisualStyleBackColor = true;
			this->_hbPlus->Click += gcnew System::EventHandler(this, &Form::_rpnWordButton_Click);
			// 
			// _hbNegate
			// 
			this->_hbNegate->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbNegate->Location = System::Drawing::Point(369, 513);
			this->_hbNegate->Name = L"_hbNegate";
			this->_hbNegate->Size = System::Drawing::Size(177, 48);
			this->_hbNegate->TabIndex = 60;
			this->_hbNegate->Tag = L"NEG";
			this->_hbNegate->Text = L"+/-";
			this->_hbNegate->UseVisualStyleBackColor = true;
			this->_hbNegate->Click += gcnew System::EventHandler(this, &Form::_rpnWordButton_Click);
			// 
			// _hb0
			// 
			this->_hb0->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb0->Location = System::Drawing::Point(186, 513);
			this->_hb0->Name = L"_hb0";
			this->_hb0->Size = System::Drawing::Size(177, 48);
			this->_hb0->TabIndex = 59;
			this->_hb0->Text = L"0";
			this->_hb0->UseVisualStyleBackColor = true;
			this->_hb0->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hbDot
			// 
			this->_hbDot->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbDot->Location = System::Drawing::Point(3, 513);
			this->_hbDot->Name = L"_hbDot";
			this->_hbDot->Size = System::Drawing::Size(177, 48);
			this->_hbDot->TabIndex = 58;
			this->_hbDot->Text = L".";
			this->_hbDot->UseVisualStyleBackColor = true;
			this->_hbDot->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hbSwap
			// 
			this->_hbSwap->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbSwap->Location = System::Drawing::Point(735, 462);
			this->_hbSwap->Name = L"_hbSwap";
			this->_hbSwap->Size = System::Drawing::Size(177, 45);
			this->_hbSwap->TabIndex = 53;
			this->_hbSwap->Tag = L"SWAP";
			this->_hbSwap->Text = L"SWAP";
			this->_hbSwap->UseVisualStyleBackColor = true;
			this->_hbSwap->Click += gcnew System::EventHandler(this, &Form::_rpnWordButton_Click);
			// 
			// _hbMinus
			// 
			this->_hbMinus->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbMinus->Location = System::Drawing::Point(552, 462);
			this->_hbMinus->Name = L"_hbMinus";
			this->_hbMinus->Size = System::Drawing::Size(177, 45);
			this->_hbMinus->TabIndex = 52;
			this->_hbMinus->Tag = L"-";
			this->_hbMinus->Text = L"-";
			this->_hbMinus->UseVisualStyleBackColor = true;
			this->_hbMinus->Click += gcnew System::EventHandler(this, &Form::_rpnWordButton_Click);
			// 
			// _hb3
			// 
			this->_hb3->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb3->Location = System::Drawing::Point(369, 462);
			this->_hb3->Name = L"_hb3";
			this->_hb3->Size = System::Drawing::Size(177, 45);
			this->_hb3->TabIndex = 51;
			this->_hb3->Text = L"3";
			this->_hb3->UseVisualStyleBackColor = true;
			this->_hb3->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hb2
			// 
			this->_hb2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb2->Location = System::Drawing::Point(186, 462);
			this->_hb2->Name = L"_hb2";
			this->_hb2->Size = System::Drawing::Size(177, 45);
			this->_hb2->TabIndex = 50;
			this->_hb2->Text = L"2";
			this->_hb2->UseVisualStyleBackColor = true;
			this->_hb2->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hb1
			// 
			this->_hb1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb1->Location = System::Drawing::Point(3, 462);
			this->_hb1->Name = L"_hb1";
			this->_hb1->Size = System::Drawing::Size(177, 45);
			this->_hb1->TabIndex = 49;
			this->_hb1->Text = L"1";
			this->_hb1->UseVisualStyleBackColor = true;
			this->_hb1->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hbOver
			// 
			this->_hbOver->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbOver->Location = System::Drawing::Point(735, 411);
			this->_hbOver->Name = L"_hbOver";
			this->_hbOver->Size = System::Drawing::Size(177, 45);
			this->_hbOver->TabIndex = 44;
			this->_hbOver->Tag = L"OVER";
			this->_hbOver->Text = L"OVER";
			this->_hbOver->UseVisualStyleBackColor = true;
			this->_hbOver->Click += gcnew System::EventHandler(this, &Form::_rpnWordButton_Click);
			// 
			// _hbMultiply
			// 
			this->_hbMultiply->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbMultiply->Location = System::Drawing::Point(552, 411);
			this->_hbMultiply->Name = L"_hbMultiply";
			this->_hbMultiply->Size = System::Drawing::Size(177, 45);
			this->_hbMultiply->TabIndex = 43;
			this->_hbMultiply->Tag = L"*";
			this->_hbMultiply->Text = L"*";
			this->_hbMultiply->UseVisualStyleBackColor = true;
			this->_hbMultiply->Click += gcnew System::EventHandler(this, &Form::_rpnWordButton_Click);
			// 
			// _hb6
			// 
			this->_hb6->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb6->Location = System::Drawing::Point(369, 411);
			this->_hb6->Name = L"_hb6";
			this->_hb6->Size = System::Drawing::Size(177, 45);
			this->_hb6->TabIndex = 42;
			this->_hb6->Text = L"6";
			this->_hb6->UseVisualStyleBackColor = true;
			this->_hb6->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hb5
			// 
			this->_hb5->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb5->Location = System::Drawing::Point(186, 411);
			this->_hb5->Name = L"_hb5";
			this->_hb5->Size = System::Drawing::Size(177, 45);
			this->_hb5->TabIndex = 41;
			this->_hb5->Text = L"5";
			this->_hb5->UseVisualStyleBackColor = true;
			this->_hb5->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hb4
			// 
			this->_hb4->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb4->Location = System::Drawing::Point(3, 411);
			this->_hb4->Name = L"_hb4";
			this->_hb4->Size = System::Drawing::Size(177, 45);
			this->_hb4->TabIndex = 40;
			this->_hb4->Text = L"4";
			this->_hb4->UseVisualStyleBackColor = true;
			this->_hb4->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hbClear
			// 
			this->_hbClear->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbClear->Location = System::Drawing::Point(735, 360);
			this->_hbClear->Name = L"_hbClear";
			this->_hbClear->Size = System::Drawing::Size(177, 45);
			this->_hbClear->TabIndex = 35;
			this->_hbClear->Tag = L"CLEAR";
			this->_hbClear->Text = L"Clear";
			this->_hbClear->UseVisualStyleBackColor = true;
			this->_hbClear->Click += gcnew System::EventHandler(this, &Form::_rpnWordButton_Click);
			// 
			// _hbDivide
			// 
			this->_hbDivide->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbDivide->Location = System::Drawing::Point(552, 360);
			this->_hbDivide->Name = L"_hbDivide";
			this->_hbDivide->Size = System::Drawing::Size(177, 45);
			this->_hbDivide->TabIndex = 34;
			this->_hbDivide->Tag = L"/";
			this->_hbDivide->Text = L"/";
			this->_hbDivide->UseVisualStyleBackColor = true;
			this->_hbDivide->Click += gcnew System::EventHandler(this, &Form::_rpnWordButton_Click);
			// 
			// _hb9
			// 
			this->_hb9->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb9->Location = System::Drawing::Point(369, 360);
			this->_hb9->Name = L"_hb9";
			this->_hb9->Size = System::Drawing::Size(177, 45);
			this->_hb9->TabIndex = 33;
			this->_hb9->Text = L"9";
			this->_hb9->UseVisualStyleBackColor = true;
			this->_hb9->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hb8
			// 
			this->_hb8->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb8->Location = System::Drawing::Point(186, 360);
			this->_hb8->Name = L"_hb8";
			this->_hb8->Size = System::Drawing::Size(177, 45);
			this->_hb8->TabIndex = 32;
			this->_hb8->Text = L"8";
			this->_hb8->UseVisualStyleBackColor = true;
			this->_hb8->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _hb7
			// 
			this->_hb7->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hb7->Location = System::Drawing::Point(3, 360);
			this->_hb7->Name = L"_hb7";
			this->_hb7->Size = System::Drawing::Size(177, 45);
			this->_hb7->TabIndex = 31;
			this->_hb7->Text = L"7";
			this->_hb7->UseVisualStyleBackColor = true;
			this->_hb7->Click += gcnew System::EventHandler(this, &Form::_hardbutton_Click);
			// 
			// _stackView
			// 
			this->_stackView->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->tableLayoutPanel1->SetColumnSpan(this->_stackView, 5);
			this->_stackView->Font = (gcnew System::Drawing::Font(L"Lucida Console", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->_stackView->Location = System::Drawing::Point(3, 3);
			this->_stackView->Multiline = true;
			this->_stackView->Name = L"_stackView";
			this->_stackView->ReadOnly = true;
			this->tableLayoutPanel1->SetRowSpan(this->_stackView, 6);
			this->_stackView->Size = System::Drawing::Size(909, 300);
			this->_stackView->TabIndex = 1;
			this->_stackView->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// _commandEntry
			// 
			this->_commandEntry->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->tableLayoutPanel1->SetColumnSpan(this->_commandEntry, 4);
			this->_commandEntry->Location = System::Drawing::Point(3, 309);
			this->_commandEntry->Name = L"_commandEntry";
			this->_commandEntry->Size = System::Drawing::Size(726, 31);
			this->_commandEntry->TabIndex = 0;
			this->_commandEntry->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->_commandEntry->TextChanged += gcnew System::EventHandler(this, &Form::_commandEntry_TextChanged);
			// 
			// _hbBack
			// 
			this->_hbBack->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_hbBack->Location = System::Drawing::Point(735, 309);
			this->_hbBack->Name = L"_hbBack";
			this->_hbBack->Size = System::Drawing::Size(177, 45);
			this->_hbBack->TabIndex = 2;
			this->_hbBack->Text = L"<BACK";
			this->_hbBack->UseVisualStyleBackColor = true;
			this->_hbBack->Click += gcnew System::EventHandler(this, &Form::_hbBack_Click);
			// 
			// _statusLabel
			// 
			this->_statusLabel->AutoSize = true;
			this->_statusLabel->Location = System::Drawing::Point(12, 619);
			this->_statusLabel->Name = L"_statusLabel";
			this->_statusLabel->Size = System::Drawing::Size(73, 25);
			this->_statusLabel->TabIndex = 6;
			this->_statusLabel->Text = L"Status";
			// 
			// openToolStripMenuItem
			// 
			this->openToolStripMenuItem->Name = L"openToolStripMenuItem";
			this->openToolStripMenuItem->Size = System::Drawing::Size(359, 44);
			this->openToolStripMenuItem->Text = L"Open...";
			this->openToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form::openToolStripMenuItem_Click);
			// 
			// saveStackToolStripMenuItem
			// 
			this->saveStackToolStripMenuItem->Name = L"saveStackToolStripMenuItem";
			this->saveStackToolStripMenuItem->Size = System::Drawing::Size(359, 44);
			this->saveStackToolStripMenuItem->Text = L"Save Stack";
			this->saveStackToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form::saveStackToolStripMenuItem_Click);
			// 
			// restoreStackToolStripMenuItem
			// 
			this->restoreStackToolStripMenuItem->Name = L"restoreStackToolStripMenuItem";
			this->restoreStackToolStripMenuItem->Size = System::Drawing::Size(359, 44);
			this->restoreStackToolStripMenuItem->Text = L"Restore Stack";
			this->restoreStackToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form::restoreStackToolStripMenuItem_Click);
			// 
			// Form
			// 
			this->AcceptButton = this->_hbEnter;
			this->AutoScaleDimensions = System::Drawing::SizeF(12, 25);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1654, 653);
			this->Controls->Add(this->_statusLabel);
			this->Controls->Add(this->tableLayoutPanel1);
			this->Controls->Add(this->menuStrip1);
			this->MainMenuStrip = this->menuStrip1;
			this->Name = L"Form";
			this->Text = L"RPN Calc";
			this->Load += gcnew System::EventHandler(this, &Form::Form1_Load);
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->tableLayoutPanel1->ResumeLayout(false);
			this->tableLayoutPanel1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
	  void createSoftButtons();
#pragma endregion
	  rpn::WordDefinition::Result pushEntry();
	  void insertString(System::String ^cc);

	  // delegated methods allow us to call UI stuff from off-ui thread
	  // however...
	  // XXX-ELH: if _assignMenuDelegate hasn't been set yet, assume we
	  //          are in initialization and running on the UI thread already
	  //
	  //
	  //  We use "BeginInvoke()" because plain Invoke() blocks until the method returns
	  //  and we'll deadlock waiting for the mutex to be released from the calling thread
	  //
	  //  I hope we can just trust "managed code management" to clean up the returned Object^
	  //  as appropriate.
	public: void redrawDisplay() {
	  if (_redrawDisplayDelegate!=nullptr && this->InvokeRequired && this->IsHandleCreated) {
	    BeginInvoke(_redrawDisplayDelegate);
	  } else {
	    redrawDisplayMethod();
	  }
	}
	public: void assignButton(int column, int row, const std::string &rpnword, const std::string &label) {
	  System::String ^srpnword = gcnew String(rpnword.c_str());
	  System::String ^slabel =gcnew String(label.c_str());
	  if (_assignButtonDelegate != nullptr && this->InvokeRequired && this->IsHandleCreated) {
	    array<Object^>^args = {column, row, srpnword, slabel};
	    BeginInvoke(_assignButtonDelegate, args);
	  } else {
	    assignButtonMethod(column,row,srpnword,slabel);
	  }
	}
	public: void assignMenu(const std::string &menu, const std::string &rpnword, const std::string &label) {
	  System::String ^smenu = gcnew String(menu.c_str());
	  System::String ^srpnword = gcnew String(rpnword.c_str());
	  System::String ^slabel =gcnew String(label.c_str());
	  if (_assignMenuDelegate != nullptr && this->InvokeRequired && this->IsHandleCreated) {
	    array<Object^>^args = {smenu, srpnword, slabel};
	    BeginInvoke(_assignMenuDelegate, args);
	  } else {
	    // XXX-ELH: if _assignMenuDelegate hasn't been set yet, assume we
	    //          are in initialization and running on the UI thread already
	    assignMenuMethod(smenu,srpnword,slabel);
	  }
	}
	public: void clearAssignedButtons() {
	  if(_clearAssignedButtonsDelegate!=nullptr && this->InvokeRequired && this->IsHandleCreated) {
	    BeginInvoke(_clearAssignedButtonsDelegate);
	  } else {
	    clearAssignedButtonsMethod();
	  }
	}
	public: void enableUI(bool pred) {
	  if (_enableUiDelegate!=nullptr && this->InvokeRequired && this->IsHandleCreated) {
	    array<Object^>^args = {pred};
	    BeginInvoke(_enableUiDelegate, args);
	  } else {
	    enableUiMethod(pred);
	  }
	}

	private: System::Void keysToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
	}
private: System::Void tableLayoutPanel1_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
}
private: System::Void _commandEntry_TextChanged(System::Object^ sender, System::EventArgs^ e) {
}

private: System::Void Form1_Load(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void _hardbutton_Click(System::Object^ sender, System::EventArgs^ e) {
  System::Windows::Forms::Button^ b = (System::Windows::Forms::Button^)sender;
  insertString((System::String^)b->Text);
}
private: System::Void _rpnWordButton_Click(System::Object^ sender, System::EventArgs^ e);
private: System::Void _rpnWordMenu_Click(System::Object^ sender, System::EventArgs^ e);

private: System::Void _hbEnter_Click(System::Object^ sender, System::EventArgs^ e);

private: System::Void _hbBack_Click(System::Object^ sender, System::EventArgs^ e);
private: System::Void openToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void saveStackToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void restoreStackToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
}
};
}
