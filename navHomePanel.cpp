#include <navHomePanel.h>
#include "navOS.h"
#include <rectArrange.h>
#include <markList.h>

//#include <debug.h>

#define APP_ICON_H	40
#define APP_ICON_Y	4

#define DATE_TIME_X				10
#define DATE_TIME_Y				0
#define FIX_X						275
#define FIX_Y						0
			
#define SPEED_RECT				30,33,250,64	//10,30,250,64
#define DEPTH_RECT				30,103,250,64	//10,105,250,64
#define BEARING_RECT				30,173,250,64	//10,180,250,64
#define DISTANCE_RECT			30,243,250,64	//10,255,250,64
#define COG_RECT					30,313,250,64	//10,330,250,64

#define LAT_LON_X					40
#define LAT_LON_Y					390	



// *****************************************************
//                      navHomePanel
// *****************************************************


navHomePanel::navHomePanel(void)
	  : homePanel() {
	
	ourOS.setScr(false);
	if (haveMarkSet) {
		ourNavApp.setMark(&selectedMark);
		haveMarkSet = false;
	}
}


navHomePanel::~navHomePanel(void) {  }


void navHomePanel::setup(void) {

	rect			iconBar;
	iconArrange	spreader;
	int			defX;
	int			defY;
	
	fixLED* fixLight = new fixLED(FIX_X,FIX_Y);
	fixLight->setColors(&green,&red);
	addObj(fixLight);
	
	GPSDateTime* clock = new GPSDateTime(DATE_TIME_X,DATE_TIME_Y);
	addObj(clock);
	
	GPSLatLon* latLonReadout = new GPSLatLon(LAT_LON_X,LAT_LON_Y);
	addObj(latLonReadout);
	
	speedBox* knotGauge = new speedBox(SPEED_RECT,"Kn","Speed",1);
	if (knotGauge) {
		knotGauge->setHandler(ourNavApp.knotMeter);
		addObj(knotGauge);
	}
	
	depthBox* depthGauge = new depthBox(DEPTH_RECT,"Fm","Depth",1);
	if (depthGauge) {
		depthGauge->setHandler(ourNavApp.depthSounder);
		addObj(depthGauge);
	}
	
	bearingBox* bearingGauge = new bearingBox(BEARING_RECT,"Deg m","Bearing",0);
	if (bearingGauge) {
		addObj(bearingGauge);
	}
	
	distanceBox* distanceGauge = new distanceBox(DISTANCE_RECT,"N mi","Distance",2);
	if (distanceGauge) {
		addObj(distanceGauge);
	}
	
	COGBox* COGGauge = new COGBox(COG_RECT,"Deg m","COG",0);
	if (COGGauge) {
		addObj(COGGauge);
	}
	
	
	// Setting up the spreader for the icon list across the bottom of the display.
	
	iconBar.x = 0;											// Set at the left edge.
	iconBar.y = screen->height() - APP_ICON_H;	// Set to display bottom - height of icon.
	iconBar.width = screen->width();					// Set to width of the display.
	iconBar.height = APP_ICON_H;						// Set to height of icon.
	spreader.settings(&iconBar,10,20);				// Setup the spreader's limits.
	
	defX = 0;	// It no longer really matters where we initially put the icons.
	defY = 0;	// The spreader will arrange them for us.
	
	appIcon*  markEditer = new appIcon(defX++, defY++, markListApp, iconPath(markListApp));
	addObj(markEditer);
	spreader.addRect(markEditer);
	
	appIcon*  calc = new appIcon(defX++, defY++, calcApp, iconPath(calcApp));
	addObj(calc);
	spreader.addRect(calc);
}


void navHomePanel::drawSelf(void) { screen->fillScreen(&black);  ourOS.setScr(true);}


void navHomePanel::loop(void) {  }









	


