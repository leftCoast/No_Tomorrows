#ifndef navOS_h
#define navOS_h

#include <lilOS.h>

#define BEEP_PIN		23
#define VIBE_PIN		22
#define DSP_BACKLITE	15

enum  apps {
    
   navHomeApp = HOME_PANEL_ID,
   markListApp,
   calcApp,
   shopListApp,
   nextApp
}; 



// *****************************************************
// ******************   navOS    ******************
// *****************************************************


class navOS : public lilOS {

  public:
            navOS(void);
   virtual  ~navOS(void);
  
   virtual  bool		begin(void);						// The global world is online, do hookups.
   virtual  panel*	createPanel(int panelID);		// Whip up a new panel.
   virtual  void     launchPanel(void);				// Dispose current and launch new panel.
            void     beep(void);							// 
            void     setScr(bool onOff);  			// true for on false for off.
   virtual  void     idle(void);               		// Need to do something in the background?

  // Calls to be overwritten by this version.
   virtual  int         getTonePin(void);
   virtual  const char* getSystemFolder(void);
   virtual  const char* getPanelName(int panelID);
};

extern navOS ourOS;

#endif
