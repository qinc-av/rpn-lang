#include <cmath>

#include "rpn.h"
#include "rpnkeypad.h"
#include "ui_rpnkeypad.h"
#include <QFontDatabase>

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

struct RpnKeypadController::Privates : public rpn::WordContext {
  Privates(rpn::Runtime &rpn, RpnKeypadController *d) : _rpn(rpn), _rpnd(d), _ui(new Ui::RpnKeypad) {
    add_private_words();

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

    auto v = QFontDatabase::addApplicationFont(":/etc/digital-7-font/Digital7Mono-Yz9J4.ttf");
    QString family = QFontDatabase::applicationFontFamilies(v).at(0);
    _ui->textEdit->setFontFamily(family);
    _ui->textEdit->setFontPointSize(18);
    redraw_display();
  }
  ~Privates() {
    delete _ui;
  }
  rpn::WordDefinition::Result pushEntry() {
    rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
    std::string line = _ui->lineEdit->text().toStdString();
    if (line != "") {
      rv = _rpn.parse(line);
      _ui->lineEdit->clear();
    }
    return rv;
  }

  RpnKeypadController *_rpnd;
  Ui::RpnKeypad* _ui;
  rpn::Runtime &_rpn;

  void redraw_display();
  void on_programmable_button(const std::string &id);
  void assign_button(unsigned row, unsigned column, const QString &label, const std::string &rpnword);
  void add_private_words();
};

RpnKeypadController::RpnKeypadController(rpn::Runtime &rpn, QWidget* parent) : QWidget(parent), _p(new Privates(rpn, this))  {
  setWindowTitle("RPN Keypad");
  /*_p->_ui->lineEdit->*/installEventFilter(this);
}

RpnKeypadController::~RpnKeypadController() { delete _p; }

void
RpnKeypadController::assignButton(unsigned row, unsigned column, const QString &label, const std::string &rpnword) {
  _p->assign_button(row,column,label,rpnword);
}

bool
RpnKeypadController::eventFilter(QObject *watched, QEvent *event) {
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

void RpnKeypadController::on_button_0_clicked() { _p->_ui->lineEdit->insert("0"); }

void RpnKeypadController::on_button_1_clicked() { _p->_ui->lineEdit->insert("1"); }

void RpnKeypadController::on_button_2_clicked() { _p->_ui->lineEdit->insert("2"); }

void RpnKeypadController::on_button_3_clicked() { _p->_ui->lineEdit->insert("3"); }

void RpnKeypadController::on_button_4_clicked() { _p->_ui->lineEdit->insert("4"); }

void RpnKeypadController::on_button_5_clicked() { _p->_ui->lineEdit->insert("5"); }

void RpnKeypadController::on_button_6_clicked() { _p->_ui->lineEdit->insert("6"); }

void RpnKeypadController::on_button_7_clicked() { _p->_ui->lineEdit->insert("7"); }

void RpnKeypadController::on_button_8_clicked() { _p->_ui->lineEdit->insert("8"); }

void RpnKeypadController::on_button_9_clicked() { _p->_ui->lineEdit->insert("9"); }

void RpnKeypadController::on_button_dot_clicked() { _p->_ui->lineEdit->insert("."); }

/******************************** enter/back ********************************/


void
RpnKeypadController::on_button_enter_clicked() {
  if (_p->_ui->lineEdit->text() == "") {
    _p->_rpn.stack.dup();
  } else {
    _p->pushEntry();
  }
  _p->redraw_display();
}

void
RpnKeypadController::on_button_back_clicked() {
  if (_p->_ui->lineEdit->text() != "") {
    _p->_ui->lineEdit->backspace();
  } else {
    _p->_rpn.stack.drop();
    _p->redraw_display();
  }
}

void
RpnKeypadController::on_button_chs_clicked() {
  if (_p->_ui->lineEdit->text() != "") {
    float val = _p->_ui->lineEdit->text().toFloat() * -1.;
    _p->_ui->lineEdit->setText(QString::number(val));
  } else {
    std::string line = "CHS";
    _p->_rpn.parse(line);
  }
  _p->redraw_display();
}

void RpnKeypadController::on_button_add_clicked() {
  if (_p->pushEntry()==rpn::WordDefinition::Result::ok) {
      std::string line = "+";
      _p->_rpn.parse(line);
    }
  _p->redraw_display();
}

void RpnKeypadController::on_button_subtract_clicked() {
  if (_p->pushEntry()==rpn::WordDefinition::Result::ok) {
      std::string line = "-";
      _p->_rpn.parse(line);
    }
  _p->redraw_display();
}

void RpnKeypadController::on_button_multiply_clicked() {
  if (_p->pushEntry()==rpn::WordDefinition::Result::ok) {
      std::string line = "*";
      _p->_rpn.parse(line);
    }
  _p->redraw_display();
}

void RpnKeypadController::on_button_divide_clicked() {
  if (_p->pushEntry()==rpn::WordDefinition::Result::ok) {
      std::string line = "/";
      _p->_rpn.parse(line);
    }
  _p->redraw_display();
}

/******************************** application programmable buttons ********************************/

void RpnKeypadController::Privates::on_programmable_button(const std::string &button) {
  if (pushEntry()==rpn::WordDefinition::Result::ok) {
    auto qs = QString::fromStdString(button);
    auto *b = _rpnd->findChild<QPushButton*>(qs);
    QString qword = b->property("rpn-word").toString();
    if (qword != "") {
      std::string word=qword.toStdString();
      _rpn.parse(word);
    }
  }
  redraw_display();
}

void
RpnKeypadController::Privates::assign_button(unsigned row, unsigned column, const QString &label, const std::string &rpnword) {
  char btn[32];
  snprintf(btn, sizeof(btn), "pb_%d_%d", row, column);
  auto *b = _rpnd->findChild<QPushButton*>(btn);
  if (b) {
    b->setText(label);
    b->setText(label);
    b->setProperty("rpn-word", QString(QString::fromStdString(rpnword)));
    b->setEnabled(true);
  }
}

void RpnKeypadController::on_pb_1_1_clicked(){ _p->on_programmable_button("pb_1_1"); }
void RpnKeypadController::on_pb_2_1_clicked(){ _p->on_programmable_button("pb_2_1"); }
void RpnKeypadController::on_pb_3_1_clicked(){ _p->on_programmable_button("pb_3_1"); }
void RpnKeypadController::on_pb_4_1_clicked(){ _p->on_programmable_button("pb_4_1"); }
void RpnKeypadController::on_pb_5_1_clicked(){ _p->on_programmable_button("pb_5_1"); }
void RpnKeypadController::on_pb_6_1_clicked(){ _p->on_programmable_button("pb_6_1"); }
void RpnKeypadController::on_pb_7_1_clicked(){ _p->on_programmable_button("pb_7_1"); }
void RpnKeypadController::on_pb_8_1_clicked(){ _p->on_programmable_button("pb_8_1"); }
void RpnKeypadController::on_pb_9_1_clicked(){ _p->on_programmable_button("pb_9_1"); }

void RpnKeypadController::on_pb_1_2_clicked(){ _p->on_programmable_button("pb_1_2"); }
void RpnKeypadController::on_pb_2_2_clicked(){ _p->on_programmable_button("pb_2_2"); }
void RpnKeypadController::on_pb_3_2_clicked(){ _p->on_programmable_button("pb_3_2"); }
void RpnKeypadController::on_pb_4_2_clicked(){ _p->on_programmable_button("pb_4_2"); }
void RpnKeypadController::on_pb_5_2_clicked(){ _p->on_programmable_button("pb_5_2"); }
void RpnKeypadController::on_pb_6_2_clicked(){ _p->on_programmable_button("pb_6_2"); }
void RpnKeypadController::on_pb_7_2_clicked(){ _p->on_programmable_button("pb_7_2"); }
void RpnKeypadController::on_pb_8_2_clicked(){ _p->on_programmable_button("pb_8_2"); }
void RpnKeypadController::on_pb_9_2_clicked(){ _p->on_programmable_button("pb_9_2"); }
void RpnKeypadController::on_pb_10_2_clicked(){ _p->on_programmable_button("pb_10_2"); }

void RpnKeypadController::on_pb_1_3_clicked(){ _p->on_programmable_button("pb_1_3"); }
void RpnKeypadController::on_pb_2_3_clicked(){ _p->on_programmable_button("pb_2_3"); }
void RpnKeypadController::on_pb_3_3_clicked(){ _p->on_programmable_button("pb_3_3"); }
void RpnKeypadController::on_pb_4_3_clicked(){ _p->on_programmable_button("pb_4_3"); }
void RpnKeypadController::on_pb_5_3_clicked(){ _p->on_programmable_button("pb_5_3"); }
void RpnKeypadController::on_pb_6_3_clicked(){ _p->on_programmable_button("pb_6_3"); }
void RpnKeypadController::on_pb_7_3_clicked(){ _p->on_programmable_button("pb_7_3"); }
void RpnKeypadController::on_pb_8_3_clicked(){ _p->on_programmable_button("pb_8_3"); }
void RpnKeypadController::on_pb_9_3_clicked(){ _p->on_programmable_button("pb_9_3"); }
void RpnKeypadController::on_pb_10_3_clicked(){ _p->on_programmable_button("pb_10_3"); }

void RpnKeypadController::on_pb_1_4_clicked(){ _p->on_programmable_button("pb_1_4"); }
void RpnKeypadController::on_pb_2_4_clicked(){ _p->on_programmable_button("pb_2_4"); }
void RpnKeypadController::on_pb_3_4_clicked(){ _p->on_programmable_button("pb_3_4"); }
void RpnKeypadController::on_pb_4_4_clicked(){ _p->on_programmable_button("pb_4_4"); }
void RpnKeypadController::on_pb_5_4_clicked(){ _p->on_programmable_button("pb_5_4"); }
void RpnKeypadController::on_pb_6_4_clicked(){ _p->on_programmable_button("pb_6_4"); }
void RpnKeypadController::on_pb_7_4_clicked(){ _p->on_programmable_button("pb_7_4"); }
void RpnKeypadController::on_pb_8_4_clicked(){ _p->on_programmable_button("pb_8_4"); }
void RpnKeypadController::on_pb_9_4_clicked(){ _p->on_programmable_button("pb_9_4"); }
void RpnKeypadController::on_pb_10_4_clicked(){ _p->on_programmable_button("pb_10_4"); }

void RpnKeypadController::on_pb_7_0_clicked(){ _p->on_programmable_button("pb_7_0"); }
void RpnKeypadController::on_pb_8_0_clicked(){ _p->on_programmable_button("pb_8_0"); }
void RpnKeypadController::on_pb_9_0_clicked(){ _p->on_programmable_button("pb_9_0"); }

/******************************** Stack display  ********************************/

void RpnKeypadController::on_toggle_work_machine_clicked() {
}

void RpnKeypadController::on_toggle_relative_absolute_clicked() {
}

void RpnKeypadController::on_toggle_jog_cut_clicked() {
}

void
RpnKeypadController::Privates::redraw_display() {
  _ui->textEdit->clear();
  _ui->textEdit->setAlignment(Qt::AlignRight);
  
  for(size_t i=_rpn.stack.depth(); i!=0; i--) {
    char level[32];
    snprintf(level, sizeof(level), " : %02d%s", i, i>1?"\n":"");
    auto so = _rpn.stack.peek_as_string(i);
    _ui->textEdit->insertPlainText(QString::fromStdString(so+level));
  }
  _ui->statusLabel->setText(QString::fromStdString(_rpn.status()));
  _ui->textEdit->verticalScrollBar()->setValue(_ui->textEdit->verticalScrollBar()->maximum());
}

static const rpn::StrictTypeValidator skAssignValidator({
    typeid(StString).hash_code(), typeid(StString).hash_code(),typeid(StInteger).hash_code(), typeid(StInteger).hash_code()
      });

NATIVE_WORD_DECL(keypad, ASSIGN_KEY) {
  rpn::WordDefinition::Result rv = rpn::WordDefinition::Result::ok;
  RpnKeypadController::Privates *p = dynamic_cast<RpnKeypadController::Privates*>(ctx);
  auto word = rpn.stack.pop_string();
  auto label = rpn.stack.pop_string();
  auto column = rpn.stack.pop_integer();
  auto row = rpn.stack.pop_integer();

  if(rpn.wordExists(word)) {
    p->assign_button(row, column, QString::fromStdString(label), word);
  } else {
    rv = rpn::WordDefinition::Result::eval_error;
  }
  return rv;
}

void
RpnKeypadController::Privates::add_private_words() {
  _rpn.addDefinition("assign-key", { skAssignValidator, NATIVE_WORD_FN(keypad, ASSIGN_KEY), this });
}


// 1 1 ." sqrt" ." SQRT" assign-key
