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

struct RpnKeypadUi::Privates {
    Privates(RpnKeypadUi *d) :_rpnd(d), _ui(new Ui::RpnKeypad) {
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
  ~Privates() {
    delete _ui;
  }
  RpnKeypadUi *_rpnd;
  Ui::RpnKeypad* _ui;
  rpn::Runtime _rpn;

  void redrawDisplay();
  void pushEntry();
  void programmable_button(const std::string &id);
};

RpnKeypadUi::RpnKeypadUi(QWidget* parent) : QWidget(parent), _p(new Privates(this))  {
  setWindowTitle("RPN Keypad");
  /*_p->_ui->lineEdit->*/installEventFilter(this);
}

RpnKeypadUi::~RpnKeypadUi() { delete _p; }

void
RpnKeypadUi::assignButton(unsigned row, unsigned column, const QString &label, const std::string &rpnword) {
  char btn[32];
  snprintf(btn, sizeof(btn), "pb_%d_%d", row, column);
  auto *b = _p->_rpnd->findChild<QPushButton*>(btn);
  if (b) {
    b->setText(label);
    b->setText(label);
    b->setProperty("rpn-word", QString(QString::fromStdString(rpnword)));
    b->setEnabled(true);
  }
}

bool
RpnKeypadUi::eventFilter(QObject *watched, QEvent *event) {
  if(event->type() == QKeyEvent::KeyPress) {
    QKeyEvent * ke = static_cast<QKeyEvent*>(event);
    if(ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
      on_button_enter_clicked();
      return true; // do not process this event further
    }
  }
  return false; // process this event further
}

/******************************** DIGITS ********************************/

void RpnKeypadUi::on_button_0_clicked() { _p->_ui->lineEdit->insert("0"); }

void RpnKeypadUi::on_button_1_clicked() { _p->_ui->lineEdit->insert("1"); }

void RpnKeypadUi::on_button_2_clicked() { _p->_ui->lineEdit->insert("2"); }

void RpnKeypadUi::on_button_3_clicked() { _p->_ui->lineEdit->insert("3"); }

void RpnKeypadUi::on_button_4_clicked() { _p->_ui->lineEdit->insert("4"); }

void RpnKeypadUi::on_button_5_clicked() { _p->_ui->lineEdit->insert("5"); }

void RpnKeypadUi::on_button_6_clicked() { _p->_ui->lineEdit->insert("6"); }

void RpnKeypadUi::on_button_7_clicked() { _p->_ui->lineEdit->insert("7"); }

void RpnKeypadUi::on_button_8_clicked() { _p->_ui->lineEdit->insert("8"); }

void RpnKeypadUi::on_button_9_clicked() { _p->_ui->lineEdit->insert("9"); }

void RpnKeypadUi::on_button_dot_clicked() { _p->_ui->lineEdit->insert("."); }

/******************************** enter/back ********************************/

void RpnKeypadUi::on_button_enter_clicked()
{
  if (_p->_ui->lineEdit->text() == "") {
    _p->_rpn.stack.dup();
  } else {
    std::string line = _p->_ui->lineEdit->text().toStdString();
    _p->_ui->lineEdit->clear();
    auto st = _p->_rpn.parse(line);
  }
  _p->redrawDisplay();
}

void RpnKeypadUi::on_button_back_clicked() {
  if (_p->_ui->lineEdit->text() != "") {
    _p->_ui->lineEdit->backspace();
  } else {
    _p->_rpn.stack.drop();
    _p->redrawDisplay();
  }
}

void RpnKeypadUi::on_button_chs_clicked()
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

void RpnKeypadUi::on_button_add_clicked() {
}

void RpnKeypadUi::on_button_subtract_clicked() {
}

void RpnKeypadUi::on_button_multiply_clicked() {
}

void RpnKeypadUi::on_button_divide_clicked() {
}

/******************************** application programmable buttons ********************************/

void RpnKeypadUi::Privates::programmable_button(const std::string &button) {
  auto qs = QString::fromStdString(button);
  auto *b = _rpnd->findChild<QPushButton*>(qs);
  QString qword = b->property("rpn-word").toString();
  if (qword != "") {
    std::string word=qword.toStdString();
    _rpn.parse(word);
    redrawDisplay();
  }
}

void RpnKeypadUi::on_pb_1_1_clicked(){ _p->programmable_button("pb_1_1"); }
void RpnKeypadUi::on_pb_2_1_clicked(){ _p->programmable_button("pb_2_1"); }
void RpnKeypadUi::on_pb_3_1_clicked(){ _p->programmable_button("pb_3_1"); }
void RpnKeypadUi::on_pb_4_1_clicked(){ _p->programmable_button("pb_4_1"); }
void RpnKeypadUi::on_pb_5_1_clicked(){ _p->programmable_button("pb_5_1"); }
void RpnKeypadUi::on_pb_6_1_clicked(){ _p->programmable_button("pb_6_1"); }
void RpnKeypadUi::on_pb_7_1_clicked(){ _p->programmable_button("pb_7_1"); }
void RpnKeypadUi::on_pb_8_1_clicked(){ _p->programmable_button("pb_8_1"); }
void RpnKeypadUi::on_pb_9_1_clicked(){ _p->programmable_button("pb_9_1"); }

void RpnKeypadUi::on_pb_1_2_clicked(){ _p->programmable_button("pb_1_2"); }
void RpnKeypadUi::on_pb_2_2_clicked(){ _p->programmable_button("pb_2_2"); }
void RpnKeypadUi::on_pb_3_2_clicked(){ _p->programmable_button("pb_3_2"); }
void RpnKeypadUi::on_pb_4_2_clicked(){ _p->programmable_button("pb_4_2"); }
void RpnKeypadUi::on_pb_5_2_clicked(){ _p->programmable_button("pb_5_2"); }
void RpnKeypadUi::on_pb_6_2_clicked(){ _p->programmable_button("pb_6_2"); }
void RpnKeypadUi::on_pb_7_2_clicked(){ _p->programmable_button("pb_7_2"); }
void RpnKeypadUi::on_pb_8_2_clicked(){ _p->programmable_button("pb_8_2"); }
void RpnKeypadUi::on_pb_9_2_clicked(){ _p->programmable_button("pb_9_2"); }
void RpnKeypadUi::on_pb_10_2_clicked(){ _p->programmable_button("pb_10_2"); }

void RpnKeypadUi::on_pb_1_3_clicked(){ _p->programmable_button("pb_1_3"); }
void RpnKeypadUi::on_pb_2_3_clicked(){ _p->programmable_button("pb_2_3"); }
void RpnKeypadUi::on_pb_3_3_clicked(){ _p->programmable_button("pb_3_3"); }
void RpnKeypadUi::on_pb_4_3_clicked(){ _p->programmable_button("pb_4_3"); }
void RpnKeypadUi::on_pb_5_3_clicked(){ _p->programmable_button("pb_5_3"); }
void RpnKeypadUi::on_pb_6_3_clicked(){ _p->programmable_button("pb_6_3"); }
void RpnKeypadUi::on_pb_7_3_clicked(){ _p->programmable_button("pb_7_3"); }
void RpnKeypadUi::on_pb_8_3_clicked(){ _p->programmable_button("pb_8_3"); }
void RpnKeypadUi::on_pb_9_3_clicked(){ _p->programmable_button("pb_9_3"); }
void RpnKeypadUi::on_pb_10_3_clicked(){ _p->programmable_button("pb_10_3"); }

void RpnKeypadUi::on_pb_1_4_clicked(){ _p->programmable_button("pb_1_4"); }
void RpnKeypadUi::on_pb_2_4_clicked(){ _p->programmable_button("pb_2_4"); }
void RpnKeypadUi::on_pb_3_4_clicked(){ _p->programmable_button("pb_3_4"); }
void RpnKeypadUi::on_pb_4_4_clicked(){ _p->programmable_button("pb_4_4"); }
void RpnKeypadUi::on_pb_5_4_clicked(){ _p->programmable_button("pb_5_4"); }
void RpnKeypadUi::on_pb_6_4_clicked(){ _p->programmable_button("pb_6_4"); }
void RpnKeypadUi::on_pb_7_4_clicked(){ _p->programmable_button("pb_7_4"); }
void RpnKeypadUi::on_pb_8_4_clicked(){ _p->programmable_button("pb_8_4"); }
void RpnKeypadUi::on_pb_9_4_clicked(){ _p->programmable_button("pb_9_4"); }
void RpnKeypadUi::on_pb_10_4_clicked(){ _p->programmable_button("pb_10_4"); }

void RpnKeypadUi::on_pb_7_0_clicked(){ _p->programmable_button("pb_7_0"); }
void RpnKeypadUi::on_pb_8_0_clicked(){ _p->programmable_button("pb_8_0"); }
void RpnKeypadUi::on_pb_9_0_clicked(){ _p->programmable_button("pb_9_0"); }

/******************************** Stack display  ********************************/

void RpnKeypadUi::on_toggle_work_machine_clicked() {
}

void RpnKeypadUi::on_toggle_relative_absolute_clicked() {
}

void RpnKeypadUi::on_toggle_jog_cut_clicked() {
}

void
RpnKeypadUi::Privates::redrawDisplay() {
  _ui->textEdit->clear();
  _ui->textEdit->setAlignment(Qt::AlignRight);
  
  for(size_t i=_rpn.stack.depth(); i!=0; i--) {
    char level[32];
    snprintf(level, sizeof(level), " : %02d%s", i, i>1?"\n":"");
    const auto &so = _rpn.stack.peek(i);
    _ui->textEdit->insertPlainText(QString::fromStdString((std::string)so)+level);
  }
  _ui->statusLabel->setText(QString::fromStdString(_rpn.status()));
  _ui->textEdit->verticalScrollBar()->setValue(_ui->textEdit->verticalScrollBar()->maximum());
}

void
RpnKeypadUi::Privates::pushEntry() {
#if 0
  if (_p->_ui->lineEdit->text()!="") {
    float next = _p->_ui->lineEdit->text().toFloat();
    _p->_ui->lineEdit->clear();
    pushStack(next);
  }
#endif
}

