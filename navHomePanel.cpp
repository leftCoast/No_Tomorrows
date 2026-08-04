#include <navHomePanel.h>
#include "navOS.h"
#include <rectArrange.h>
#include <runningAvg.h>

//#include <debug.h>

#define APP_ICON_H	40
#define APP_ICON_Y	4

// What flavor data is the GPS databox wanting to display?
#define BEARING	0
#define DIST		1
#define COG			2
#define SOG			3

// What flavor of data is the NMEA databox wanting to display?
#define	RPM_VAL	0
#define	FUEL		1
#define	SPEED		2
#define	DEPTH		3
#define	BARO		4

#define LED_RECT					306,3,12,12
#define FIX_RECT					275,1,30,16
				
#define SPEED_RECT				30,30,250,64	//10,30,250,64
#define DEPTH_RECT				30,100,250,64	//10,105,250,64
#define BEARING_RECT				30,170,250,64	//10,180,250,64
#define DISTANCE_RECT			30,240,250,64	//10,255,250,64
#define BOROMETER_RECT			30,310,250,64	//10,330,250,64

#define LATT_X						10
#define LATT_Y						387				// 420
#define LATT_W						70
#define LATT_H						36
#define LONT_X						LATT_X
#define LONT_Y						LATT_Y+30
#define LONT_W						LATT_W
#define LONT_H						LATT_H

#define LAT_X						90
#define LAT_Y						LATT_Y				// 420
#define LAT_W						175
#define LAT_H						36
#define LON_X						LAT_X
#define LON_Y						LONT_Y
#define LON_W						LAT_W
#define LON_H						LAT_H

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
	savedStamp = NULL;
	heapStr(&savedStamp," ");
	//timer.setTime(2000);
}


navHomePanel::~navHomePanel(void) {freeStr(&savedStamp); }


void navHomePanel::setup(void) {

	rect			iconBar;
	iconArrange	spreader;
	int			defX;
	int			defY;

	rect			LEDRect;
	fontLabel*	latText;
	fontLabel*	lonText;
	fontLabel*	fixText;
		
	fixText = new fontLabel(FIX_RECT);
	fixText->setColors(&yellow,&black);
	fixText->setFont(AFF_SANS_9_OB);
	fixText->setTextSize(1);
	fixText->setValue("Fix");
	viewList.addObj(fixText);
	
	LEDRect.setRect(LED_RECT);
	fixLED = new LED(&LEDRect,&green,&red);
	viewList.addObj(fixLED);
	
	timeLabel = new erasableText(10,0,250,32);
	timeLabel->setColors(&yellow,&black);
	timeLabel->setFont(AFF_MONO_12);
	timeLabel->setTextSize(1);
	viewList.addObj(timeLabel);
	
	latText = new fontLabel(LATT_X,LATT_Y,LATT_W,LATT_H);
	latText->setColors(&yellow,&black);
	latText->setFont(AFF_MONO_12);
	timeLabel->setTextSize(1);
	latText->setValue("Lat :");
	viewList.addObj(latText);
	
	latLabel = new erasableText(LAT_X,LAT_Y,LAT_W,LAT_H);
	latLabel->setColors(&yellow,&black);
	latLabel->setFont(AFF_MONO_12);
	timeLabel->setTextSize(1);
	viewList.addObj(latLabel);
	
	lonText = new fontLabel(LONT_X,LONT_Y,LONT_W,LONT_H);
	lonText->setColors(&yellow,&black);
	lonText->setFont(AFF_MONO_12);
	timeLabel->setTextSize(1);
	lonText->setValue("Lon :");
	viewList.addObj(lonText);
	
	lonLabel = new erasableText(LON_X,LON_Y,LON_W,LON_H);
	lonLabel->setColors(&yellow,&black);
	lonLabel->setFont(AFF_MONO_12);
	timeLabel->setTextSize(1);
	viewList.addObj(lonLabel);
	
	
	speedBox* knotGauge = new speedBox(SPEED_RECT,"Kn","Speed",1);
	if (knotGauge) {
		knotGauge->setHandler(ourNavApp.knotMeter);
		viewList.addObj(knotGauge);
	}
	
	depthBox* depthGauge = new depthBox(DEPTH_RECT,"Fm","Depth",1);
	if (depthGauge) {
		depthGauge->setHandler(ourNavApp.depthSounder);
		viewList.addObj(depthGauge);
	}
	
	bearingBox* bearingGauge = new bearingBox(BEARING_RECT,"Deg m","Bearing",0);
	if (bearingGauge) {
		viewList.addObj(bearingGauge);
	}
	/*
	distanceGauge = new GPSBox(DISTANCE_RECT,"N mi","Distance",1);
	if (distanceGauge) {
		distanceGauge->setup(DIST);
		viewList.addObj(distanceGauge);
	}
	barometerGauge = new NMEABox(BOROMETER_RECT,"InHg","Air pressure",2);
	if (barometerGauge) {
		barometerGauge->setup(BARO);
		viewList.addObj(barometerGauge);
	}
	*/
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


void navHomePanel::showPos(globalPos* fix) {

	char		outStr[40];
	double	value;
	char		qStr[3];
	DateTime	timeStamp(ourGPS->year,ourGPS->month,ourGPS->day,ourGPS->hours,ourGPS->min,ourGPS->sec);
	TimeSpan	deltaTime(0,ourNavApp.hoursOffUTC,0,0);
	
	if (ourGPS->valid) {
		fixLED->setState(true);
		timeStamp = timeStamp + deltaTime;
		sprintf(outStr,"%02d/%02d/%4d  %02d:%02d",
		timeStamp.month(),
		timeStamp.day(),
		timeStamp.year(),
		timeStamp.hour(),
		timeStamp.minute());
		if (strcmp(savedStamp,outStr)) {
			timeLabel->setValue(outStr);
			heapStr(&savedStamp,outStr);
		}
		
		strcpy(qStr," N");
		if (fix->getLatQuad()==south) {
			strcpy(qStr," S");
		}
		value = fix->getLatAsDbl();
		if (value<0) value = -value;
		sprintf (outStr,"%*f%s",10,value,qStr);
		latLabel->setValue(outStr);
		
		strcpy(qStr," W");
		if (fix->getLonQuad()==east) {
			strcpy(qStr," E");
		}
		value = fix->getLonAsDbl();
		if (value<0) value = -value;
		sprintf (outStr,"%*f%s",10,value,qStr);
		lonLabel->setValue(outStr);
	} else {
		fixLED->setState(false);
		sprintf(outStr,"--/--/----  --:--");
		timeLabel->setValue(outStr);
		sprintf (outStr,"---.------");
		latLabel->setValue(outStr);
		lonLabel->setValue(outStr);
	}
}








	


