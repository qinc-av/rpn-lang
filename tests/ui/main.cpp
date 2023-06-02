/***************************************************
 * file: Candle/src/tests/rpnkeypad/main.cpp
 *
 * @file    rpnkeypad.cpp<tests>
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Saturday, May 13, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include <QApplication>
#include <QStack>
#include "rpnkeypad.h"

int
main(int ac, char **av) {
  QApplication app(ac, av);

  QStack<QJsonValue> stack;
  RpnKeypadDialog rpn(stack);
  rpn.setModal(true);
  rpn.show();
  return app.exec();
}

/* end of Candle/src/tests/rpnkeypad/main.cpp */
