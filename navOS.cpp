
#include <lilOS.h>
#include <navOS.h>
#include <navHomePanel.h>

#include <rpnCalc.h>
//#include <sTerm.h>
#include <shopList.h>


// *****************************************************
//                     navOS
// *****************************************************


// Our global OS.
navOS ourOS;


// Create it..
navOS::navOS(void) {}


// Delete it..
navOS::~navOS(void) {}


// Possibly we'll need to do something during startup.
bool navOS::begin(void) {

	bool	success;
	
	success = false;
	if (lilOS::begin()) {				// Sets up the global OSPtr.
		setScr(false);						// Make false is off.
		pinMode(BEEP_PIN, OUTPUT);		// If we get a beep in, set it up here.
		digitalWrite(BEEP_PIN, HIGH);	// Means off.
		success = true;					// Looks good from here.
	}											//
	return success;						// report!
}


// Used to create our custom panels..
panel* navOS::createPanel(int panelID) {

  switch (panelID) {
    case navHomeApp: return new navHomePanel();
    case calcApp: return new rpnCalc(calcApp);
    case shopListApp: return new shopList(shopListApp);
    default: return NULL;
  }
}


void navOS::beep() {

  tone(BEEP_PIN, 500, 35);
  digitalWrite(VIBE_PIN,HIGH);
  delay(10);
  digitalWrite(VIBE_PIN,LOW);
}


void navOS::launchPanel(void) {

  beep();
  lilOS::launchPanel();
}


// false off, true on.
void navOS::setScr(bool onOff) { digitalWrite(DSP_LED,onOff); }                                              


// Things we do behind close doors..
void navOS::idle(void) { }


// Calls to be overwritten by user version.
int navOS::getTonePin(void) { return BEEP_PIN; }

const char* navOS::getSystemFolder() { return "/system/"; }


const char* navOS::getPanelName(int panelID) {

    switch (panelID) {
      case calcApp: return "rpnCalc";
      case shopListApp: return "shopList";
      default: return NULL;
    }
  }
