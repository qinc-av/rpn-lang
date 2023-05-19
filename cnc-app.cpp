/***************************************************
 * file: github/elh/rpn-cnc/cnc-dictionary.cpp
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

struct MachineInterface::Privates {
  Privates(RpnController &rpn);
  std::map<std::string,word_t> _appDict;
  RpnController &_rpn;
};

MachineInterface::MachineInterface(RpnController &rpn) {
  m_p = new Privates(rpn);
}

MachineInterface::~MachineInterface() {
  delete m_p;
}

MachineInterface::Privates::Privates(RpnController &rpn) : _rpn(rpn) {
  rpn.addDictionary({
/*
     * Machine control words
     */
    { "MPOS->", { "Push Machine Position to the stack. ( -- mpos )", {
	  {},
	},
		  [this](const std::string &word, std::string &rest) -> std::string::size_type {
		    std::string::size_type rv = 0;
		    return rv;
		  }
      } },

    { "WPOS->", { "Push Work Position to the stack. ( -- wpos )", {
	  {},
	},
		  [this](const std::string &word, std::string &rest) -> std::string::size_type {
		    std::string::size_type rv = 0;
		    _rpn.stack_push(Vec3(1.2, 2.3, 3.4));
		    return rv;
		  }
      } },

    { "->WPOS", { "Set Work Position ( wpos -- )", {
	  { { "newpos", st_vec3 } },
	},
		  [this](const std::string &word, std::string &rest) -> std::string::size_type {
		    std::string::size_type rv = 0;
		    Vec3 np = _rpn.stack_pop_as_vec3();
		    printf("->WPOS: popped '%s' from stack\n", to_string(np).c_str());
		    return rv;
		  }
      } },

    { "SPEED->", { "Push Spindle Speed to the stack. ( -- speed )", {
	  {},
	},
		   [this](const std::string &word, std::string &rest) -> std::string::size_type {
		     std::string::size_type rv = 0;
		     _rpn.stack_push(10000);
		     return rv;
		   }
      } },

    { "->SPEED", { "Set Spindle Speed ( speed -- )", {
	  { { "speed", st_number } },
	},
		   [this](const std::string &word, std::string &rest) -> std::string::size_type {
		     std::string::size_type rv = 0;
		     return rv;
		   }
      } },

    { "FEED->", { "Push jog feed rate to the stack. ( -- feed )", {
	  {},
	},
		  [this](const std::string &word, std::string &rest) -> std::string::size_type {
		    std::string::size_type rv = 0;
		    return rv;
		  }
      } },

    { "->FEED", { "Set jog feed rate ( feed -- )", {
	  { { "feed", st_number } },
	},
		  [this](const std::string &word, std::string &rest) -> std::string::size_type {
		    std::string::size_type rv = 0;
		    return rv;
		  }
      } },

    { "JOG-R", { "Jog to relative position ( pos -- )", {
	  { { "offset", st_vec3 } },
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   return rv;
		 }
      } },

    { "JOG-WA", { "Jog to absolute work position ( wpos -- )", {
	  { { "wpos", st_vec3 } },
	},
		  [this](const std::string &word, std::string &rest) -> std::string::size_type {
		    std::string::size_type rv = 0;
		    return rv;
		  }
      } },

    { "JOG-MA", { "Jog to absolute machine position ( mpos -- )", {
	  { { "mpos", st_vec3 } },
	},
		  [this](const std::string &word, std::string &rest) -> std::string::size_type {
		    std::string::size_type rv = 0;
		    return rv;
		  }
      } },

    { "PROBE", { "Probe machine (target feed -- )", {
	  { { "target", st_vec3 }, { "feed" , st_number } }, // primitive G38.2
	},
		 [this](const std::string &word, std::string &rest) -> std::string::size_type {
		   std::string::size_type rv = 0;
		   return rv;
		 }
      } },

    { "MODAL-STATE->", { "Push machine's modal state on the stack ( -- state )", {
	  {},
	},
			 [this](const std::string &word, std::string &rest) -> std::string::size_type {
			   std::string::size_type rv = 0;
			   return rv;
			 }
      } },

    { "->MODAL-STATE", { "Send modal state to the machine ( state -- )", {
	  { { "state", st_string } },
	},
			 [this](const std::string &word, std::string &rest) -> std::string::size_type {
			   std::string::size_type rv = 0;
			   return rv;
			 }
      } },


    { "SEND", { "Send command", {
	  { { "g-code", st_string } },
	},
		[this](const std::string &word, std::string &rest) -> std::string::size_type {
		  std::string::size_type rv = 0;
		  return rv;
		}
      } }
    });
}

/* end of github/elh/rpn-cnc/cnc-dictionary.cpp */
