#include <cmath>

#include "rpn.h"
#include "rpnkeypad.h"
#include "ui_rpnkeypad.h"
#include <QFontDatabase>

struct RpnKeypadController::Privates : public rpn::WordContext {
  Privates(rpn::Runtime &rpn, RpnKeypadController *d) : _rpn(rpn), _rpnd(d), _ui(new Ui::RpnKeypad) {
    add_private_words();

    _ui->setupUi(_rpnd);
    _ui->textEdit->setReadOnly(true);
    _ui->textEdit->setAlignment(Qt::AlignRight);

    for(auto *w : programmableButtons()) {
      QPushButton *b = dynamic_cast<QPushButton*>(w);
      b->setText("");
      b->setEnabled(false);
      b->setProperty("rpn-word", ""); // QString(QString::fromStdString(btn)));

      connect(b, SIGNAL(clicked()), _rpnd, SLOT(on_programmable_button_clicked()));
    }

    //    auto v = QFontDatabase::addApplicationFont(":/etc/digital-7-font/Digital7Mono-Yz9J4.ttf");
    auto v = QFontDatabase::addApplicationFont(":/etc/led-counter-7/led_counter-7.ttf");
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

  QList<QWidget*> programmableButtons() {
    QRegularExpression exp("^pb_");
    return _rpnd->findChildren<QWidget *>(exp);
  }

  RpnKeypadController *_rpnd;
  Ui::RpnKeypad* _ui;
  rpn::Runtime &_rpn;

  void redraw_display();
  void assign_button(unsigned row, unsigned column, const QString &label, const std::string &rpnword);
  void add_private_words();
};

RpnKeypadController::RpnKeypadController(rpn::Runtime &rpn, QWidget* parent) : QWidget(parent), _p(new Privates(rpn, this))  {
  setWindowTitle("RPN Keypad");
  installEventFilter(this);
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

void
RpnKeypadController::Privates::assign_button(unsigned row, unsigned column, const QString &label, const std::string &rpnword) {
  char btn[32];
  snprintf(btn, sizeof(btn), "pb_%d_%d", row, column);
  auto *b = _rpnd->findChild<QPushButton*>(btn);
  if (b) {
    b->setText(label);
    b->setProperty("rpn-word", QString(QString::fromStdString(rpnword)));
    b->setEnabled(true);
  }
}

void RpnKeypadController::on_programmable_button_clicked() {
  if (_p->pushEntry()==rpn::WordDefinition::Result::ok) {
    QObject *b = this->sender();
    QString qword = b->property("rpn-word").toString();
    if (qword != "") {
      std::string word=qword.toStdString();
      _p->_rpn.parse(word);
    }
  }
  _p->redraw_display();
}


/******************************** Stack display  ********************************/

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
