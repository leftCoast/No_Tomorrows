#include <navHomePanel.h>
#include "navOS.h"
#include <rectArrange.h>
#include <runningAvg.h>

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

#define LAT_LON_X					44
#define LAT_LON_Y					390	



// *****************************************************
//                      iconArrange 
// *****************************************************


iconArrange::iconArrange(void)
  : rectArrange() {  }


iconArrange::~iconArrange(void) { }


void iconArrange::arrangeList(void) {

	int				    xLoc;
	int				    space;
	rectListObj*  trace;

	if (minWidth()<=areaRect.width) {							// If we can make it fit.
		if (maxWidth()<=areaRect.width) {						// No matter, it'll fit..
			xLoc = areaRect.width - maxWidth();					// Streach 'em out.
			space = maxWSpace;										// Choose max space.
		} else {															// Too many?
			xLoc = areaRect.width - minWidth();					// Shrink 'em up.
			space = minWSpace;										// Choose narrow.
		}																	//
		xLoc = (xLoc + areaRect.x)/2;								// Don't forget the offset..
		trace = (rectListObj*)getFirst();						// Grab the first one on the list.
		while(trace) {													// For ever rect we can find..
			trace->ourRect->x = xLoc;								// Set this rect's x location.
			trace->ourRect->y = areaRect.y + APP_ICON_Y;		// Set this rect's y location.
			xLoc = xLoc + trace->ourRect->width + space;		// Calcualte the next rect's location.
			trace = (rectListObj*)trace->getNext();			// Hop to the next rect on the list.
		}																	//
	}																		// If they don't fit? Leave 'em be.
}

runningAvg	baroSmoother(10);

// *****************************************************
//                      navHomePanel
// *****************************************************


navHomePanel::navHomePanel(void)
	  : homePanel() {
	ourOS.setScr(false);
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
	
	distanceBox* distanceGauge = new distanceBox(DISTANCE_RECT,"N mi","Distance",1);
	if (distanceGauge) {
		addObj(distanceGauge);
	}
	
	COGBox* COGGauge = new COGBox(COG_RECT,"Deg m","COG",0);
	if (COGGauge) {
		addObj(COGGauge);
	}
	
	iconBar.x = 0;
	iconBar.y = screen->height() - APP_ICON_H;
	iconBar.width = screen->width();
	iconBar.height = APP_ICON_H;
	spreader.settings(&iconBar,10,20);
	
	defX = 0;
	defY = 0;
	
	appIcon*  markEditer = new appIcon(defX++, defY++, markListApp, iconPath(markListApp));
	addObj(markEditer);
	spreader.addRect(markEditer);
	
	appIcon*  calc = new appIcon(defX++, defY++, calcApp, iconPath(calcApp));
	addObj(calc);
	spreader.addRect(calc);

	appIcon* shoppingList = new appIcon(defX++, defY++, shopListApp, iconPath(shopListApp));
	addObj(shoppingList);
	spreader.addRect(shoppingList);
}


void navHomePanel::drawSelf(void) { screen->fillScreen(&black);  ourOS.setScr(true);}


void navHomePanel::loop(void) {  }









	


