#include <cmath>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVector3D>
#include <QDoubleValidator>

#include "rpnkeypad.h"
#include "ui_rpnkeypad.h"

static double degrees_to_radians(double degrees) {
  return degrees * (M_PI / 180.);
}

static const QJsonObject toJsonObject(const QVector3D &pos) {
  QJsonObject ob;
  ob["x"] = pos.x();
  ob["y"] = pos.y();
  ob["z"] = pos.z();
  return ob;
}

static const QString toString(const QJsonValue &v) {
  QString o;
  switch(v.type()) {
  case QJsonValue::Null:
    o = "null";
    break;

  case QJsonValue::Bool:
    o = v.toBool() ? "true" : "false";
    break;

  case QJsonValue::Double:
    o = QString::number(v.toDouble());
    break;

  case QJsonValue::String:
    o = v.toString();
    break;

  case QJsonValue::Array:
    o = QJsonDocument(v.toArray()).toJson(QJsonDocument::Compact);
    break;

  case QJsonValue::Object:
    o = QJsonDocument(v.toObject()).toJson(QJsonDocument::Compact);
    break;

  case QJsonValue::Undefined:
    o = "undefined";
    break;
  }

  return o;
}

RpnKeypadDialog::RpnKeypadDialog(QStack<QJsonValue> &stack, QWidget* parent) : QDialog(parent), m_ui(new Ui::RpnKeypadDialog), m_stack(stack)
{
    m_ui->setupUi(this);
    m_ui->lineEdit->setValidator(new QDoubleValidator());
    m_ui->textEdit->setReadOnly(true);
    m_ui->textEdit->setAlignment(Qt::AlignRight);
    setWindowTitle("Candle RPN Keypad");
    redrawDisplay();
}

RpnKeypadDialog::~RpnKeypadDialog() { delete m_ui; }

/******************************** DIGITS ********************************/

void RpnKeypadDialog::on_button_0_clicked() { m_ui->lineEdit->insert("0"); }

void RpnKeypadDialog::on_button_1_clicked() { m_ui->lineEdit->insert("1"); }

void RpnKeypadDialog::on_button_2_clicked() { m_ui->lineEdit->insert("2"); }

void RpnKeypadDialog::on_button_3_clicked() { m_ui->lineEdit->insert("3"); }

void RpnKeypadDialog::on_button_4_clicked() { m_ui->lineEdit->insert("4"); }

void RpnKeypadDialog::on_button_5_clicked() { m_ui->lineEdit->insert("5"); }

void RpnKeypadDialog::on_button_6_clicked() { m_ui->lineEdit->insert("6"); }

void RpnKeypadDialog::on_button_7_clicked() { m_ui->lineEdit->insert("7"); }

void RpnKeypadDialog::on_button_8_clicked() { m_ui->lineEdit->insert("8"); }

void RpnKeypadDialog::on_button_9_clicked() { m_ui->lineEdit->insert("9"); }

void RpnKeypadDialog::on_button_dot_clicked() { m_ui->lineEdit->insert("."); }

/******************************** OPERATIONS ********************************/

void RpnKeypadDialog::on_button_sum_clicked()
{
    pushEntry();

    if (m_stack.length() < 2)                                  //stack must have at least two elements for an operation
        return;

    float a = popStack();
    float b = popStack();

    pushStack(b + a);                                         //inserts operation result into stack and updates TextEdit
}

void RpnKeypadDialog::on_button_subtract_clicked()
{
    pushEntry();

    if (m_stack.length() < 2)
        return;

    float a = popStack();
    float b = popStack();

    pushStack(b - a);
}

void RpnKeypadDialog::on_button_multiplication_clicked()
{
    pushEntry();

    if (m_stack.length() < 2)
        return;

    float a = popStack();
    float b = popStack();

    pushStack(b * a);
}

void RpnKeypadDialog::on_button_division_clicked()
{
    pushEntry();

    if (m_stack.length() < 2)
        return;

    if (m_stack.top() == 0)                                    //avoids division by zero
        return;

    float a = popStack();
    float b = popStack();

    pushStack(b / a);
}

void RpnKeypadDialog::on_button_sin_clicked()
{
    pushEntry();

    if (m_stack.isEmpty())
        return;

    float a = popStack();

    pushStack(sin(degrees_to_radians(a)));
}

void RpnKeypadDialog::on_button_cos_clicked()
{
    pushEntry();

    if (m_stack.isEmpty())
        return;

    float a = popStack();

    pushStack(cos(degrees_to_radians(a)));
}

void RpnKeypadDialog::on_button_tan_clicked()
{
    pushEntry();

    if (m_stack.isEmpty())
        return;

    float a = popStack();

    pushStack(tan(degrees_to_radians(a)));
}

void RpnKeypadDialog::on_button_sqrt_clicked()
{
    pushEntry();

    if (m_stack.isEmpty())
        return;

    float a = popStack();

    pushStack(sqrt(a));
}

void RpnKeypadDialog::on_button_plus_minus_clicked()
{
  if (m_ui->lineEdit->text() != "") {
    float val = m_ui->lineEdit->text().toFloat() * -1.;
    m_ui->lineEdit->setText(QString::number(val));
  } else if (!m_stack.isEmpty()) {
    float val = popStack();
    val *= -1.;
    pushStack(val);
  } else {
    return; // do nothing
  }
}

void RpnKeypadDialog::on_button_roll_down_clicked()
{
  pushEntry();
  if (!m_stack.isEmpty()) {
    auto val = m_stack.front();
    m_stack.pop_front();
    m_stack.push_back(val);
    redrawDisplay();
  }
}

void RpnKeypadDialog::on_button_roll_up_clicked()
{
  pushEntry();
  if (!m_stack.isEmpty()) {
    auto val = m_stack.last();
    m_stack.pop_back();
    m_stack.push_front(val);
    redrawDisplay();
  }
}

void RpnKeypadDialog::on_button_enter_clicked()
{
  if (m_ui->lineEdit->text() == "") {
    pushStack(m_stack.top());

  } else {

    pushEntry();
  }
}

void RpnKeypadDialog::on_button_swap_clicked()
{
    pushEntry();
    if (m_stack.length() < 2)
        return;

    float a = popStack();
    float b = popStack();

    pushStack(a);
    pushStack(b);
}


void RpnKeypadDialog::on_button_back_clicked() {
  if (m_ui->lineEdit->text() != "") {
    m_ui->lineEdit->backspace();
  } else {
    if (!m_stack.isEmpty()) {
      popStack();
    }
  }
}

void RpnKeypadDialog::on_button_go_x_clicked() {
  pushEntry();
  if (!m_stack.isEmpty()) {
    float pos = popStack();
    bool jog = m_ui->toggle_jog_cut->isChecked();
    bool rel = m_ui->toggle_relative_absolute->isChecked();
    bool work = m_ui->toggle_work_machine->isChecked();
    switch((rel&1)<<1|(work&1)) {
    case 3: // relative  + work
    case 2: // relative + machine
      //      m_mc->goRelative({pos, 0, 0}, jog);
      break;
    case 1: // absolute + work
      //      m_mc->goAbsoluteWork({pos, std::nanf(""), std::nanf("")}, jog);
      break;
    case 0: // absolute + machine
      //      m_mc->goAbsoluteMachine({pos, std::nanf(""), std::nanf("")}, jog);
      break;
    }
  }
}

void RpnKeypadDialog::on_button_set_x_clicked() {
  pushEntry();
  if (!m_stack.isEmpty()) {
    float pos = popStack();
    //    m_mc->setWorkPos({pos, std::nanf(""), std::nanf("")});
  }
}

void RpnKeypadDialog::on_button_push_x_clicked() {
  pushEntry();
  QVector3D pos;
  bool work = m_ui->toggle_work_machine->isChecked();
  if (work) {
    //    pos = m_mc->workPos();
  } else {
    //    pos = m_mc->machinePos();
  }
  pushStack(pos.x());
}

void RpnKeypadDialog::on_button_go_y_clicked() {
  pushEntry();
  if (!m_stack.isEmpty()) {
    float pos = popStack();
    bool jog = m_ui->toggle_jog_cut->isChecked();
    bool rel = m_ui->toggle_relative_absolute->isChecked();
    bool work = m_ui->toggle_work_machine->isChecked();
    switch((rel&1)<<1|(work&1)) {
    case 3: // relative  + work
    case 2: // relative + machine
      //      m_mc->goRelative({0, pos, 0}, jog);
      break;
    case 1: // absolute + work
      //      m_mc->goAbsoluteWork({std::nanf(""), pos, std::nanf("")}, jog);
      break;
    case 0: // absolute + machine
      //      m_mc->goAbsoluteMachine({std::nanf(""), pos, std::nanf("")}, jog);
      break;
    }
  }
}


void RpnKeypadDialog::on_button_set_y_clicked() {
  pushEntry();
  if (!m_stack.isEmpty()) {
    float pos = popStack();
    //    m_mc->setWorkPos({std::nanf(""), pos, std::nanf("")});
  }
}

void RpnKeypadDialog::on_button_push_y_clicked() {
  pushEntry();
  QVector3D pos;
  bool work = m_ui->toggle_work_machine->isChecked();
  if (work) {
    //    pos = m_mc->workPos();
  } else {
    //    pos = m_mc->machinePos();
  }
  pushStack(pos.y());
}

void RpnKeypadDialog::on_button_go_z_clicked() {
  pushEntry();
  if (!m_stack.isEmpty()) {
    float pos = popStack();
    bool jog = m_ui->toggle_jog_cut->isChecked();
    bool rel = m_ui->toggle_relative_absolute->isChecked();
    bool work = m_ui->toggle_work_machine->isChecked();
    switch((rel&1)<<1|(work&1)) {
    case 3: // relative  + work
    case 2: // relative + machine
      //      m_mc->goRelative({0, 0, pos}, jog);
      break;
    case 1: // absolute + work
      //      m_mc->goAbsoluteWork({std::nanf(""), std::nanf(""), pos}, jog);
      break;
    case 0: // absolute + machine
      //      m_mc->goAbsoluteMachine({std::nanf(""), std::nanf(""), pos}, jog);
      break;
    }
  }
}


void RpnKeypadDialog::on_button_set_z_clicked() {
  pushEntry();
  if (!m_stack.isEmpty()) {
    float pos = popStack();
    //    m_mc->setWorkPos({std::nanf(""), std::nanf(""), pos});
  }
}

void RpnKeypadDialog::on_button_push_z_clicked() {
  pushEntry();
  if (!m_stack.isEmpty()) {
    QVector3D pos;
    bool work = m_ui->toggle_work_machine->isChecked();
    if (work) {
      //      pos = m_mc->workPos();
    } else {
      //      pos = m_mc->machinePos();
    }
    pushStack(pos.z());
  }
}

void RpnKeypadDialog::on_button_ok_clicked() {
  done(0);
}

void RpnKeypadDialog::on_button_set_spindle_clicked() {
  pushEntry();
  if (!m_stack.isEmpty()) {
    float speed = popStack();
    //    m_mc->setSpindle(speed);
  }
}

void RpnKeypadDialog::on_button_push_spindle_clicked() {
  pushEntry();
  //  pushStack(m_mc->spindle());
}

void RpnKeypadDialog::on_button_set_feed_clicked() {
  pushEntry();
  if (!m_stack.isEmpty()) {
    float feed = popStack();
    //    m_mc->setJogFeed(feed);
  }
}

void RpnKeypadDialog::on_button_push_feed_clicked() {
  pushEntry();
  //  pushStack(m_mc->jogFeed());
}

void RpnKeypadDialog::on_button_go_xy_clicked() {
  pushEntry();
  if (m_stack.size()>1) {
    float y = popStack();
    float x = popStack();

    bool jog = m_ui->toggle_jog_cut->isChecked();
    bool rel = m_ui->toggle_relative_absolute->isChecked();
    bool work = m_ui->toggle_work_machine->isChecked();
    switch((rel&1)<<1|(work&1)) {
    case 3: // relative  + work
    case 2: // relative + machine
      //      m_mc->goRelative({x, y, 0}, jog);
      break;
    case 1: // absolute + work
      //      m_mc->goAbsoluteWork({x, y, std::nanf("")}, jog);
      break;
    case 0: // absolute + machine
      //      m_mc->goAbsoluteMachine({x, y, std::nanf("")}, jog);
      break;
    }
  }
}


void RpnKeypadDialog::on_button_set_xy_clicked() {
  pushEntry();
  if (m_stack.size()>1) {
    float y = popStack();
    float x = popStack();
    //    m_mc->setWorkPos({x, y, std::nanf("")});
  }
}

void RpnKeypadDialog::on_button_push_xy_clicked() {
  pushEntry();
  bool work = m_ui->toggle_work_machine->isChecked();
  QVector3D pos;
  if (work) {
    //    pos = m_mc->workPos();
  } else {
    //    pos = m_mc->machinePos();
  }
  pushStack(toJsonObject(pos));
  //  pushStack(pos.x());
  //  pushStack(pos.y());
}

void RpnKeypadDialog::on_toggle_work_machine_clicked() {
  QString label;
  bool enabled = true;
  if (m_ui->toggle_work_machine->isChecked()) {
    label = "Work";
    enabled = true;
  } else {
    label = "Machine";
    enabled = false;
  }
  m_ui->button_set_x->setEnabled(enabled);
  m_ui->button_set_y->setEnabled(enabled);
  m_ui->button_set_z->setEnabled(enabled);
  m_ui->button_set_xy->setEnabled(enabled);

  m_ui->toggle_work_machine->setText(label);
}

void RpnKeypadDialog::on_toggle_relative_absolute_clicked() {
  QString label;
  if (m_ui->toggle_relative_absolute->isChecked()) {
    label = "Relative";
  } else {
    label = "Absolute";
  }
  m_ui->toggle_relative_absolute->setText(label);
}

void RpnKeypadDialog::on_toggle_jog_cut_clicked() {
  QString label;
  if (m_ui->toggle_jog_cut->isChecked()) {
    label = "Jog";
  } else {
    label = "Rapid";
  }
  m_ui->toggle_jog_cut->setText(label);
}

/******************************** UTILS ********************************/

float
RpnKeypadDialog::popStack() {
  float last = m_stack.pop().toDouble();

  redrawDisplay();
  return last;
}

void
RpnKeypadDialog::pushStack(float n) {
  float nn = round(n*100000.)/100000.;
  m_ui->textEdit->insertPlainText(QString::number(nn) + "\n");
  m_stack.push(n);
}

void
RpnKeypadDialog::pushStack(const QJsonValue &v) {
  m_ui->textEdit->insertPlainText(v.toString() + "\n");
  m_stack.push(v);
}

void
RpnKeypadDialog::redrawDisplay() {
  m_ui->textEdit->clear();
  m_ui->textEdit->setAlignment(Qt::AlignRight);
  foreach(const QJsonValue &v, m_stack) {
    m_ui->textEdit->insertPlainText(toString(v) + "\n");
  }
}

void
RpnKeypadDialog::pushEntry() {
  if (m_ui->lineEdit->text()!="") {
    float next = m_ui->lineEdit->text().toFloat();
    m_ui->lineEdit->clear();
    pushStack(next);
  }
}

#if 0
/*
  parameters:
  feedFast
  feedSlow

  multi-point parameters:
  zClear
  zProbe
  points
*/

class Prober {
  double m_fastFeed;
  double m_slowFeed;
  double m_zClear;
  double m_zProbe;

  /*
   * Plain Probe
   * mostly just wrapper for G38.2 - returns machine position after probe
   */
  static QVector3D probeSingle(const QVector3D &target, double feed);

  /*
   * Debounced Probing
   *    - probe at fastFeed until hit;
   *    - calculate backoff as percentage of actual travel distance
   *            for example, if we probe +X12 and hit at +10, with a backoff of "0.1"
   *            then we move back 0.1*10 = 1
   *        if backoff==0.; then we just do plain single probe
   */
  QVector3D probeSingleDebounced(const QVector3D &target, double backoff); 

  /*
   * probe at location other than current work pos
   *  1. moves to zClear
   *  2. moves to p0
   *  3. moves to zProbe
   *  4. executes probe
   *  5. saves position
   *  6. returns to p0 (prevents dragging probe across edge of workpiece)
   */
  QVector3D probeAtLocation(const QVector3D &p0, const QVector &t0, double backoff);

  /*
   * probe at several points
   */
  std::vector<QVector3D> probeAtLocations(const std::pair<QVector3D,QVector3D> &locations, double backoff);

  /*
   * probe interior rectangle
   */
  std::vector<QVector3D> probeInteriorRectangle(const QVector3D &p0, double xPlus, double xMinus, double yPlus, double yMinus, double backoff);

  
  /*
   * probe exterior rectangle
   */
  std::vector<QVector3D> probeExteriorRectangle(const QVector3D &p0, double xPlus, double xMinus, double yPlus, double yMinus, double backoff);

  /*
   * probe interior radial
   */
  std::vector<QVector3D> probeInteriorRadial(const QVector3D &p0, double startAngle, double endAngle, int n, double radius, double backoff);
};
#endif

/*
 *
 */
