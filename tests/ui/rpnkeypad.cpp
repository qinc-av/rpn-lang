#include <cmath>

#include "rpn.h"
#include "rpnkeypad.h"
#include "ui_rpnkeypad.h"

static const std::vector<std::string> skButtonIds {
  "pb_1_1", 
    "pb_2_1", 
    "pb_3_1", 
    "pb_4_1", 
    "pb_5_1", 
    "pb_6_1", 
    "pb_7_1", 
    "pb_8_1", 
    "pb_9_1", 

    "pb_1_2", 
    "pb_2_2", 
    "pb_3_2", 
    "pb_4_2", 
    "pb_5_2", 
    "pb_6_2", 
    "pb_7_2", 
    "pb_8_2", 
    "pb_9_2", 
    "pb_10_2",

    "pb_1_3", 
    "pb_2_3", 
    "pb_3_3", 
    "pb_4_3", 
    "pb_5_3", 
    "pb_6_3",
    "pb_7_3", 
    "pb_8_3", 
    "pb_9_3", 
    "pb_10_3",

    "pb_1_4", 
    "pb_2_4", 
    "pb_3_4", 
    "pb_4_4", 
    "pb_5_4", 
    "pb_6_4", 
    "pb_7_4", 
    "pb_8_4", 
    "pb_9_4", 
    "pb_10_4",

    "pb_7_0", 
    "pb_8_0", 
    "pb_9_0"
    };

struct RpnKeypadDialog::Privates {
  Privates(RpnKeypadDialog *d) :_rpnd(d), _ui(new Ui::RpnKeypadDialog) {
    _ui->setupUi(_rpnd);
    _ui->textEdit->setReadOnly(true);
    _ui->textEdit->setAlignment(Qt::AlignRight);
    for(const auto &btn : skButtonIds) {
      auto *b = _rpnd->findChild<QPushButton*>(QString::fromStdString(btn));
      if (b) {
	b->setText("");
	b->setEnabled(false);
	b->setProperty("rpn-word", QString(QString::fromStdString(btn)));
      } else {
	fprintf(stderr, "couldn't find button for %s\n", btn.c_str());
      }
    }
    redrawDisplay();
  }
  RpnKeypadDialog *_rpnd;
  Ui::RpnKeypadDialog* _ui;
  rpn::Runtime _rpn;

  void redrawDisplay();
  void pushEntry();
  void programmable_button(const std::string &id);
};

RpnKeypadDialog::RpnKeypadDialog(QStack<QJsonValue> &stack, QWidget* parent) : QDialog(parent), _p(new Privates(this))  {
    setWindowTitle("RPN Keypad");
}

RpnKeypadDialog::~RpnKeypadDialog() { delete _p->_ui; }

/******************************** DIGITS ********************************/

void RpnKeypadDialog::on_button_0_clicked() { _p->_ui->lineEdit->insert("0"); }

void RpnKeypadDialog::on_button_1_clicked() { _p->_ui->lineEdit->insert("1"); }

void RpnKeypadDialog::on_button_2_clicked() { _p->_ui->lineEdit->insert("2"); }

void RpnKeypadDialog::on_button_3_clicked() { _p->_ui->lineEdit->insert("3"); }

void RpnKeypadDialog::on_button_4_clicked() { _p->_ui->lineEdit->insert("4"); }

void RpnKeypadDialog::on_button_5_clicked() { _p->_ui->lineEdit->insert("5"); }

void RpnKeypadDialog::on_button_6_clicked() { _p->_ui->lineEdit->insert("6"); }

void RpnKeypadDialog::on_button_7_clicked() { _p->_ui->lineEdit->insert("7"); }

void RpnKeypadDialog::on_button_8_clicked() { _p->_ui->lineEdit->insert("8"); }

void RpnKeypadDialog::on_button_9_clicked() { _p->_ui->lineEdit->insert("9"); }

void RpnKeypadDialog::on_button_dot_clicked() { _p->_ui->lineEdit->insert("."); }

/******************************** enter/back ********************************/

void RpnKeypadDialog::on_button_enter_clicked()
{
  if (_p->_ui->lineEdit->text() == "") {
    _p->_rpn.stack.dup();
  } else {
    std::string line = _p->_ui->lineEdit->text().toStdString();
    auto st = _p->_rpn.parse(line);
  }
  _p->redrawDisplay();
}

void RpnKeypadDialog::on_button_back_clicked() {
  if (_p->_ui->lineEdit->text() != "") {
    _p->_ui->lineEdit->backspace();
  } else {
    //    if (!m_stack.isEmpty()) {
      //      popStack();
    //    }
  }
}

void RpnKeypadDialog::on_button_chs_clicked()
{
  if (_p->_ui->lineEdit->text() != "") {
    float val = _p->_ui->lineEdit->text().toFloat() * -1.;
    _p->_ui->lineEdit->setText(QString::number(val));
    //  } else if (!m_stack.isEmpty()) {
    //    float val = popStack();
    //    val *= -1.;
    //    pushStack(val);
  } else {
    return; // do nothing
  }
}

void RpnKeypadDialog::on_button_add_clicked() {
}

void RpnKeypadDialog::on_button_subtract_clicked() {
}

void RpnKeypadDialog::on_button_multiply_clicked() {
}

void RpnKeypadDialog::on_button_divide_clicked() {
}

/******************************** application programmable buttons ********************************/

void RpnKeypadDialog::Privates::programmable_button(const std::string &button) {
  auto qs = QString::fromStdString(button);
  QWidget *pWin = QApplication::activeWindow();
  auto *b = _rpnd->findChild<QPushButton*>(qs);
  QString word = b->property("rpn-word").toString();
  qDebug() << qs + " clicked (" << b << ") with word (" << word <<")";
}

void RpnKeypadDialog::on_pb_1_1_clicked(){ _p->programmable_button("pb_1_1"); }
void RpnKeypadDialog::on_pb_2_1_clicked(){ _p->programmable_button("pb_2_1"); }
void RpnKeypadDialog::on_pb_3_1_clicked(){ _p->programmable_button("pb_3_1"); }
void RpnKeypadDialog::on_pb_4_1_clicked(){ _p->programmable_button("pb_4_1"); }
void RpnKeypadDialog::on_pb_5_1_clicked(){ _p->programmable_button("pb_5_1"); }
void RpnKeypadDialog::on_pb_6_1_clicked(){ _p->programmable_button("pb_6_1"); }
void RpnKeypadDialog::on_pb_7_1_clicked(){ _p->programmable_button("pb_7_1"); }
void RpnKeypadDialog::on_pb_8_1_clicked(){ _p->programmable_button("pb_8_1"); }
void RpnKeypadDialog::on_pb_9_1_clicked(){ _p->programmable_button("pb_9_1"); }

void RpnKeypadDialog::on_pb_1_2_clicked(){ _p->programmable_button("pb_1_2"); }
void RpnKeypadDialog::on_pb_2_2_clicked(){ _p->programmable_button("pb_2_2"); }
void RpnKeypadDialog::on_pb_3_2_clicked(){ _p->programmable_button("pb_3_2"); }
void RpnKeypadDialog::on_pb_4_2_clicked(){ _p->programmable_button("pb_4_2"); }
void RpnKeypadDialog::on_pb_5_2_clicked(){ _p->programmable_button("pb_5_2"); }
void RpnKeypadDialog::on_pb_6_2_clicked(){ _p->programmable_button("pb_6_2"); }
void RpnKeypadDialog::on_pb_7_2_clicked(){ _p->programmable_button("pb_7_2"); }
void RpnKeypadDialog::on_pb_8_2_clicked(){ _p->programmable_button("pb_8_2"); }
void RpnKeypadDialog::on_pb_9_2_clicked(){ _p->programmable_button("pb_9_2"); }
void RpnKeypadDialog::on_pb_10_2_clicked(){ _p->programmable_button("pb_10_2"); }

void RpnKeypadDialog::on_pb_1_3_clicked(){ _p->programmable_button("pb_1_3"); }
void RpnKeypadDialog::on_pb_2_3_clicked(){ _p->programmable_button("pb_2_3"); }
void RpnKeypadDialog::on_pb_3_3_clicked(){ _p->programmable_button("pb_3_3"); }
void RpnKeypadDialog::on_pb_4_3_clicked(){ _p->programmable_button("pb_4_3"); }
void RpnKeypadDialog::on_pb_5_3_clicked(){ _p->programmable_button("pb_5_3"); }
void RpnKeypadDialog::on_pb_6_3_clicked(){ _p->programmable_button("pb_6_3"); }
void RpnKeypadDialog::on_pb_7_3_clicked(){ _p->programmable_button("pb_7_3"); }
void RpnKeypadDialog::on_pb_8_3_clicked(){ _p->programmable_button("pb_8_3"); }
void RpnKeypadDialog::on_pb_9_3_clicked(){ _p->programmable_button("pb_9_3"); }
void RpnKeypadDialog::on_pb_10_3_clicked(){ _p->programmable_button("pb_10_3"); }

void RpnKeypadDialog::on_pb_1_4_clicked(){ _p->programmable_button("pb_1_4"); }
void RpnKeypadDialog::on_pb_2_4_clicked(){ _p->programmable_button("pb_2_4"); }
void RpnKeypadDialog::on_pb_3_4_clicked(){ _p->programmable_button("pb_3_4"); }
void RpnKeypadDialog::on_pb_4_4_clicked(){ _p->programmable_button("pb_4_4"); }
void RpnKeypadDialog::on_pb_5_4_clicked(){ _p->programmable_button("pb_5_4"); }
void RpnKeypadDialog::on_pb_6_4_clicked(){ _p->programmable_button("pb_6_4"); }
void RpnKeypadDialog::on_pb_7_4_clicked(){ _p->programmable_button("pb_7_4"); }
void RpnKeypadDialog::on_pb_8_4_clicked(){ _p->programmable_button("pb_8_4"); }
void RpnKeypadDialog::on_pb_9_4_clicked(){ _p->programmable_button("pb_9_4"); }
void RpnKeypadDialog::on_pb_10_4_clicked(){ _p->programmable_button("pb_10_4"); }

void RpnKeypadDialog::on_pb_7_0_clicked(){ _p->programmable_button("pb_7_0"); }
void RpnKeypadDialog::on_pb_8_0_clicked(){ _p->programmable_button("pb_8_0"); }
void RpnKeypadDialog::on_pb_9_0_clicked(){ _p->programmable_button("pb_9_0"); }

/******************************** Stack display  ********************************/

void RpnKeypadDialog::on_toggle_work_machine_clicked() {
}

void RpnKeypadDialog::on_toggle_relative_absolute_clicked() {
}

void RpnKeypadDialog::on_toggle_jog_cut_clicked() {
}

void
RpnKeypadDialog::Privates::redrawDisplay() {
  _ui->textEdit->clear();
  _ui->textEdit->setAlignment(Qt::AlignRight);
  
  for(size_t i=_rpn.stack.depth(); i!=0; i--) {
    const auto &so = _rpn.stack.peek(i);
    _ui->textEdit->insertPlainText(QString::fromStdString((std::string)so)+"\n");
  }
}

void
RpnKeypadDialog::Privates::pushEntry() {
#if 0
  if (_p->_ui->lineEdit->text()!="") {
    float next = _p->_ui->lineEdit->text().toFloat();
    _p->_ui->lineEdit->clear();
    pushStack(next);
  }
#endif
}

