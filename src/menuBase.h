/* -*- C++ -*- */
/********************
Sept. 2016 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com
creative commons license 3.0: Attribution-ShareAlike CC BY-SA
This software is furnished "as is", without technical support, and with no
warranty, express or implied, as to its usefulness for any purpose.

Thread Safe: No
Extensible: Yes

base for menu library
definitions and enumerations

www.r-site.net

***/

//#define DEBUG

#ifndef RSITE_ARDUINO_MENU_SYSTEM_BASE
  #define RSITE_ARDUINO_MENU_SYSTEM_BASE

  #ifdef DEBUG
    extern bool debugFlag;
  #endif

  #include <Arduino.h>
  //#include <Streaming.h>//https://github.com/scottdky/Streaming
  #ifndef ARDUINO_SAM_DUE
    //#include <Assert4a.h> //https://github.com/nettigo/Assert4a
  #endif
  #define assert(x)
  #include "macros.h"

  namespace Menu {
    //menu structure objects
    class menuOut;
    class navNode;
    class navRoot;
    class prompt;
    template<typename T> class menuField;
    //struct menuNodeShadow;
    class menuNode;
    class menu;
    template<typename T> class menuVariant;
    template<typename T> class toggle;
    template<typename T> class select;
    template<typename T> class choose;

    typedef int8_t idx_t;
    idx_t print_P(Print& s,const char* at,idx_t sz=0);

    enum result {proceed=0,quit};
    enum systemStyles {_noStyle=0,_menuData=1,_canNav=1<<1,_parentDraw=1<<2,_isVariant=1<<3};
    enum styles {noStyle=0,wrapStyle=1};//,dontEnter=2};
    //representing parsed input codes (can be used with switch/case)
    enum navCmds {noCmd=0,escCmd,enterCmd,upCmd,downCmd,leftCmd,rightCmd,idxCmd,selCmd,scrlUpCmd,scrlDownCmd};
    struct navCmd {
      navCmds cmd;
      idx_t param;
      inline navCmd(navCmds cmd):cmd(cmd),param(-1) {}
      inline navCmd(navCmds cmd,idx_t p):cmd(cmd),param(p) {}
      inline bool operator==(navCmds n) const {return cmd==n;}
      inline operator navCmds() const {return cmd;}
    };

    navCmds getCmd(String &name);

    //----------------------------------------------------
    //events generated by the menu system
    enum eventMask {
      noEvent=0,//just ignore all stuff
      activateEvent=1,//this item is about to be active (system event)
      enterEvent=1<<1,//entering navigation level (this menu is now active)
      exitEvent=1<<2,//leaving navigation level
      returnEvent=1<<3,//TODO:entering previous level (return)
      focusEvent=1<<4,//element just gained focus
      blurEvent=1<<5,//element about to lose focus
      selFocusEvent=1<<6,//TODO:child just gained focus
      selBlurEvent=1<<7,//TODO:child about to lose focus
      updateEvent=1<<8,//Field value has been updated
      anyEvent=~0
    };
    //events for the idle function
    enum idleEvent {idleStart,idling,idleEnd};

    //color defs, then indexing by selected and enabled status
    enum colorDefs {
      bgColor=0,
      fgColor,
      valColor,
      unitColor,
      cursorColor,
      titleColor,
      nColors//total number of colors
    };

    //this defines an array of disabled and enabled colors of yet unspecified color format C
    template<typename C>
    struct colorDef {
      C disabled[2];
      C enabled[3];
    };

    enum status {disabledStatus=0,enabledStatus=1};

    #define SYS_FUNC_PARAMS eventMask event, navNode& nav, prompt &item
    #define SYS_FUNC_VALUES event,nav,item
    #define FUNC_PARAMS eventMask event, prompt &item
    #define FUNC_VALUES event,item

    //callback function type
    typedef result (*callback)(FUNC_PARAMS);
    typedef void (*vCall)(FUNC_PARAMS);//now also accept void return

    //functions to hook as common prompt actions
    result doNothing();//just proceed with menu
    result doExit();//do not enter a menu or exit the current one

    //idle function  type
    typedef result (*idleFunc)(menuOut& o,idleEvent);
    result inaction(menuOut& o,idleEvent e);//default idle function

    //template<void (*A)(eventMask event, navNode& nav, prompt &item, Stream &in)> result callCaster(eventMask event, navNode& nav, prompt &item, Stream &in);
    template<void (*A)(eventMask event, navNode& nav, prompt &item)> result callCaster(eventMask event, navNode& nav, prompt &item);
    template<void (*A)(eventMask event, navNode& nav)> result callCaster(eventMask event, navNode& nav);
    template<void (*A)(eventMask event)> result callCaster(eventMask event);
    template<void (*A)()> result callCaster();

    //menu element associated function (called for all element registered events)
    struct actionRaw {callback hFn;};
    class action {
      public:
        callback hFn;//the hooked callback function
        inline action() {}
        //inline action(void (*f)()):hFn((callback)f) {}
        inline action(result (*f)()):hFn((callback)f) {}
        inline action(result (*f)(eventMask)):hFn((callback)f) {}
        inline action(result (*f)(eventMask,navNode&)):hFn((callback)f) {}
        //inline action(result (*f)(eventMask,navNode&,prompt&)):hFn((callback)f) {}
        //inline action(result (*f)(eventMask,navNode&,prompt&,Stream&)):hFn((callback)f) {}
        inline action(callback f):hFn(f) {}
        inline result operator()(FUNC_PARAMS) const {return ((callback)memPtr(hFn))(FUNC_VALUES);}
    };

    extern action noAction;

    //config
    ///////////////////////////////////////////////////////////////////////////
    struct navCode {navCmds cmd;char ch;};
    typedef navCode navCodesDef[10];
    extern const navCodesDef defaultNavCodes;

    struct config {
      config(
        char ecur='>',
        char dcur='-',
        bool inv=false,
        bool n2d=false,
        const navCodesDef &nc=defaultNavCodes,
        bool useUpdateEvent=false,
        bool canExit=false,
        bool numValueInput=true
      ):selectedCursor(ecur),
      disabledCursor(dcur),
      invertFieldKeys(inv),
      nav2D(n2d),
      navCodes(nc),
      useUpdateEvent(useUpdateEvent),
      canExit(canExit),
      numValueInput(numValueInput) {}
      //NOTE:this can be output specific
      char selectedCursor;//='>';
      char disabledCursor;//='-';
      //const char* exitText=exitTextMem;
      //NOTE: this can be input specific
      bool invertFieldKeys;//=false;//TODO: invert for encoder -> test this
      bool nav2D;//=false;//TODO: use left|right keys? -> test this.. this should be device dependent and therefor need generic menu inputs
      const navCodesDef &navCodes;//=defaultNavCodes;
      bool useUpdateEvent;//=false, if false, when field value is changed use enterEvent instead.
      bool canExit;//=true, if false do not exit from main menu
      bool numValueInput;//=true if true fields parse numeric input values otherwise numbers will terminate the edit and be considered accels on the parent menu probably.
      inline char getCmdChar(navCmds cmd) const {return navCodes[cmd].ch;}//return character assigned to this command
    };

    class StringStream:public Stream {
      public:
        const char *src;
        StringStream(const char*s):src(s) {}
        int available() override {return 0!=*src;}
        int read() override {return *src++;}
        int peek() override {return *src?*src:-1;}
        void flush() override {while(*src) src++;}
        size_t write(uint8_t) override {return 0;}
        operator const String() {return String(src);}
    };

    // NOTE: make this a parametrized thing instead of a global reference
    // a parametric thing will envolve a lot of reference passing
    // however some overrides will need to access them but are not allowed to receive them by parameters
    // putting them on the class would bind the instance to the options
    // for its a pointer, user can change the pointer.
    // TODO: distribute them by inputs and outputs
    extern config* options;

    #ifdef DEBUG
      Print& operator<<(Print& o,bool b);
      Print& operator<<(Print& o,navCmds cmd);
      Print& operator<<(Print& o,navCmd cmd);
      Print& operator<<(Print& o,result cmd);
      Print& operator<<(Print& o,eventMask e);
      Print& operator<<(Print& o,colorDefs c);
      Print& operator<<(Print& o,idleEvent e);
      Print& operator<<(Print& o,systemStyles s);
    #endif

  }

#endif
