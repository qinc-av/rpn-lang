/***************************************************
 * file: qinc/rpn-lang/src/cnc-dictionary.cpp
 *
 * @file    cnc-dictionary.cpp
 * @author  Eric L. Hernes
 * @version V1.0
 * @born_on   Friday, May 19, 2023
 * @copyright (C) Copyright Eric L. Hernes 2023
 * @copyright (C) Copyright Q, Inc. 2023
 *
 * @brief   An Eric L. Hernes Signature Series C++ module
 *
 */

#include "rpn-controller.h"
#include "cnc-app.h"

struct MachineInterface::Privates : public WordContext {
  Privates(RpnController &rpn);
  std::map<std::string,word_t> _appDict;
  static MachineInterface::Privates *s_instance;
};

MachineInterface::MachineInterface(RpnController &rpn) {
  m_p = new Privates(rpn);
}

MachineInterface::~MachineInterface() {
  delete m_p;
}

NATIVE_WORD_IMPL(MPOS_to) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  rpn.stack_push(Vec3(3.4, 4.5, 5.6));
  return 0;
}

NATIVE_WORD_IMPL(WPOS_to) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  rpn.stack_push(Vec3(1.2, 2.3, 3.4));
  return 0;
}

NATIVE_WORD_IMPL(to_WPOS) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  Vec3 np = rpn.stack_pop_as_vec3();
  printf("->WPOS: popped '%s' from stack\n", to_string(np).c_str());
  return 0;
}

NATIVE_WORD_IMPL(SPEED_to) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  rpn.stack_push(10000);
  return rv;
}

NATIVE_WORD_IMPL(to_SPEED) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(FEED_to) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(to_FEED) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(JOG_R) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(JOG_WA) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(JOG_MA) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(PROBE) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(MODALSTATE_to) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(to_MODALSTATE) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  return rv;
}

NATIVE_WORD_IMPL(SEND) {
  MachineInterface::Privates *p = dynamic_cast<MachineInterface::Privates*>(ctx);
  std::string::size_type rv = 0;
  printf("instance is %p\n", p);
  return rv;
}

#define PRIVATE_NATIVE_WORD_FN(sym) NATIVE_WORD_FN(sym), this

MachineInterface::Privates *MachineInterface::Privates::s_instance = nullptr;

MachineInterface::Privates::Privates(RpnController &rpn) {
  s_instance = this;
  rpn.addDictionary({
      //
      // Machine control words
      //
    { "MPOS->", {
	"Push Machine Position to the stack. ( -- mpos )", {
	  WORD_PARAMS_0(),
	    },
	  PRIVATE_NATIVE_WORD_FN(MPOS_to)
      } },

    { "WPOS->", {
	"Push Work Position to the stack. ( -- wpos )", {
	  WORD_PARAMS_0(),
	    },
	  PRIVATE_NATIVE_WORD_FN(WPOS_to)
      } },

    { "->WPOS", {
	"Set Work Position ( wpos -- )", {
	  { { "newpos", st_vec3 } },
	    },
	  PRIVATE_NATIVE_WORD_FN(to_WPOS)
      } },

    { "SPEED->", {
	"Push Spindle Speed to the stack. ( -- speed )", {
	  WORD_PARAMS_0(),
	    },
	  PRIVATE_NATIVE_WORD_FN(SPEED_to)
      } },

    { "->SPEED", {
	"Set Spindle Speed ( speed -- )", {
	  { { "speed", st_number } },
	    },
	  PRIVATE_NATIVE_WORD_FN(to_SPEED)
      } },

    { "FEED->", {
	"Push jog feed rate to the stack. ( -- feed )", {
	  WORD_PARAMS_0(),
	    },
	  PRIVATE_NATIVE_WORD_FN(FEED_to)
      } },

    { "->FEED", {
	"Set jog feed rate ( feed -- )", {
	  { { "feed", st_number } },
	    },
	  PRIVATE_NATIVE_WORD_FN(to_FEED)
      } },

    { "JOG-R", {
	"Jog to relative position ( pos -- )", {
	  { { "offset", st_vec3 } },
	    },
	  PRIVATE_NATIVE_WORD_FN(JOG_R)
      } },

    { "JOG-WA", {
	"Jog to absolute work position ( wpos -- )", {
	  { { "wpos", st_vec3 } },
	    },
	  PRIVATE_NATIVE_WORD_FN(JOG_WA)
      } },

    { "JOG-MA", {
	"Jog to absolute machine position ( mpos -- )", {
	  { { "mpos", st_vec3 } },
	    },
	  PRIVATE_NATIVE_WORD_FN(JOG_MA)
      } },

      { "PROBE", {
	"Probe machine (target feed -- )", {
	  { { "target", st_vec3 }, { "feed" , st_number } }, // primitive G38.2
	    },
	  PRIVATE_NATIVE_WORD_FN(PROBE)
      } },

      { "MODAL-STATE->", {
	  "Push machine's modal state on the stack ( -- state )", {
	    WORD_PARAMS_0(),
	      },
	  PRIVATE_NATIVE_WORD_FN(MODALSTATE_to)
	} },

    { "->MODAL-STATE", {
	"Send modal state to the machine ( state -- )", {
	  { { "state", st_string } },
	    },
	  PRIVATE_NATIVE_WORD_FN(to_MODALSTATE)
      } },


    { "SEND", {
	"Send command", {
	  { { "g-code", st_string } },
	    },
	  PRIVATE_NATIVE_WORD_FN(SEND)
      } }
    });
}

/* end of qinc/rpn-lang/src/cnc-dictionary.cpp */
