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
	timer.setTime(2000);
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
	
	
	knotGauge = new NMEABox(SPEED_RECT,"Kn","Speed",1);
	if (knotGauge) {
		knotGauge->setup(SPEED);
		viewList.addObj(knotGauge);
	}
	
	depthGauge = new NMEABox(DEPTH_RECT,"Fm","Depth",1);
	if (depthGauge) {
		depthGauge->setup(DEPTH);
		viewList.addObj(depthGauge);
	}
	
	bearingGauge = new GPSBox(BEARING_RECT,"Deg m","Bearing",0);
	if (bearingGauge) {
		bearingGauge->setup(BEARING);
		viewList.addObj(bearingGauge);
	}
	
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


void navHomePanel::loop(void) {

	if (timer.ding()) {					// If the timer dings..
		showPos(&(ourGPS->latLon));	// Tell 'em it's time to refresh screen info.
      timer.start();						// restart the timer.
   }
}


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



// *************     LED      *************


LED::LED(rect* inRect,colorObj* inOnColor,colorObj* inOffColor)
	: colorRect(inRect,inOnColor) {
	
	setColors(inOnColor,inOffColor);
	setState(false);
}
	
	
LED::~LED(void) {  }
	
	
void LED::setColors(colorObj* inOnColor,colorObj* inOffColor) {

	onColor.setColor(inOnColor);
	offColor.setColor(inOffColor);
	needRefresh = true;
}


void LED::setState(bool onOff) {

	if (onOff) {
		setColor(&onColor);
	} else {
		setColor(&offColor);
	}
	ourState = onOff;
}


void LED::drawSelf(void) {

	int	dia;

	dia = (width+height)/2;									// Grab average for radius.
	screen->fillCircle(x,y,dia,(colorObj*)this);		// We -are- a colorObj so draw a circle of our color.
}



// ************* erasableText *************


erasableText::erasableText(void)
	: fontLabel() { }
	
erasableText::erasableText(rect* inRect)
	: fontLabel(inRect) { }
	
	
erasableText::erasableText(int inX, int inY, int inW,int inH)
	: fontLabel(inX,inY,inW,inH) { }
	
	
erasableText::~erasableText(void) { }

	
void erasableText::drawSelf(void) {
	
	rect	aRect(this);
	int	xLoc;
	int	yLoc;
	
	aRect.width = aRect.width+8;				// Why?
	screen->fillRect(&aRect,&backColor);	// Erase the value.
	//screen->drawRect(&aRect,&green);		// GREEN for debugging.
	screen->setTextWrap(false);				// Wrap is not a good plan ever.
	screen->setTextColor(&textColor);		// Already erased, use transparent.
	screen->setFont(ourFont);					// Load our font.
	screen->setTextSize(1);						// Does it need this? I don't know.
	xLoc = x + fontXOffset;						// Offsets for funky font tweaks.
	yLoc = y + fontYOffset;						//
	screen->setCursor(xLoc,yLoc);				// POint to this location.. 
	screen->drawText(buff);						// And draw!
	screen->setFont(NULL);						// Unload the fons data.
}



// *************  NMEABox  *************


NMEABox::NMEABox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: valueBox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) {  }

	
NMEABox::~NMEABox(void) {  }


float NMEABox::checkData(void) {

	float	value;
	switch(dataChoice) {
		case RPM_VAL	: value = ourNavApp.engHdler->RPM;			break;
		case FUEL		: value = ourNavApp.fuelGauge->level;		break;
		case SPEED		: 
			value = ourNavApp.knotMeter->knots;
			if (value>99||value<0) {
				value = NAN;
			}
		break;
		case DEPTH		: 
			value = ourNavApp.depthSounder->feet/6.0;
			if (value>99||value<0) {
				value = NAN;
			}
		break;
		case BARO		:
			value = baroSmoother.addData(ourNavApp.barometer->inHg);
			if (value>33||value<20) {
				value = NAN;
			}
		break;
		default		:	value = NAN;
	}
	return value;
}


// *************  GPSBox  *************


GPSBox::GPSBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: valueBox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) {  }

	
GPSBox::~GPSBox(void) {  }


float GPSBox::checkData(void) {
  
	float	value;

	value = NAN;
	if (ourGPS->valid) {
		switch(dataChoice) {
			case BEARING	: value = ourNavApp.bearingMark(true);	break;
			case DIST		: value = ourNavApp.distance();			break;
			default			: value = NAN;									break;
		}
	}
	return value;
}


		
// ************* valueBox *************	


valueBox::valueBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt, int inPrec)
	: drawGroup(inX,inY,inWidth,inHeight) {
	
	labelTxt	= NULL;
	heapStr(&labelTxt,inLabel);			// Save off copy of the units text.
	typeTxt = NULL;							// DO the NULL thing..
	heapStr(&typeTxt,inTypeTxt);			// Save off copy of the type text.
	prec = inPrec;								// We'll need this later.
	factor = pow(10,inPrec);				// Calculate the multiplication factor.
	isNanNow = true;							// We'll start at NAN. Because  we really have no value.
	updateTimer = new timeObj(1000);		// refresh once a second as default.
	dataChoice = 0;
}
	
	
valueBox::~valueBox(void) {

	freeStr(&labelTxt);
	freeStr(&typeTxt);
	if (updateTimer) delete(updateTimer);
}


void valueBox::setup(int inDataChoice) {

	colorObj	darkYellow;
	
	darkYellow.setColor(&yellow);
	darkYellow.blend(&black,50);
	
	dataChoice = inDataChoice;
	valueLabel = new erasableText();
	if (valueLabel) {
		valueLabel->setFont(AFF_SANS_BOLD_24_OB);
		valueLabel->x = 5;
		valueLabel->y = 10;
		valueLabel->width = 120;
		valueLabel->setColors(&yellow,&black);
		valueLabel->setPrecision(prec);
		valueLabel->setJustify(TEXT_RIGHT);
		valueLabel->setValue("-- ");
		addObj(valueLabel);
	}
	unitsLabel = new fontLabel();
	if (unitsLabel) {
		unitsLabel->setFont(AFF_SANS_BOLD_12_OB);
		unitsLabel->x = 150;
		unitsLabel->y = 24;
		unitsLabel->width = 80;
		unitsLabel->setColors(&darkYellow,&black);
		unitsLabel->setValue(labelTxt);
		freeStr(&labelTxt);
		addObj(unitsLabel);
	}
	typeLabel = new fontLabel();
	if (typeLabel) {
		typeLabel->setFont(AFF_SANS_BOLD_9_OB);
		typeLabel->x = 5;
		typeLabel->y = 54;
		typeLabel->width = 120;
		typeLabel->setColors(&darkYellow,&black);
		typeLabel->setValue(typeTxt);
		freeStr(&typeTxt);
		addObj(typeLabel);
	}
	hookup();
}


void valueBox::drawSelf(void) { 

	//screen->drawRect(this,&green);
}


void valueBox::setValue(float value) {
	
	int	newIntVal;
	
	if (isnan(value)&&isNanNow) return;					// If we got a nan, it's already showing a nan.. Bail.
	else if (isnan(value)) {								// Else if we got a NAN and it's showing a value..
		valueLabel->setValue("-- ");						// Set label to dashes.
		isNanNow = true;										// We are NOW showing NAN.
	} else if (!isnan(value)&&isNanNow) {				// Else if we got a value and it's showing a NAN..
		savedIntVal = round(value * factor);			// Setup a integer version of the value.
		valueLabel->setValue(savedIntVal/factor);		// Set the new value to the screen.
		isNanNow = false;										// And we are no longer showing a NAN.
	} else {														// Else.. 
		newIntVal = round(value * factor);				// Setup a integer version of the value.
		if (newIntVal!=savedIntVal) {						// If it's different than the saved integer of what we have now..
			valueLabel->setValue(newIntVal/factor);	// Set the new value to the screen.
			savedIntVal = newIntVal;
		}
	}
}


void valueBox::idle(void) {

	float	value;
	
	drawGroup::idle();			// Let our parent have a go.
	if (updateTimer->ding()) {	// If our timer goes off..
		value = checkData();		// Grab fresh data value.
		setValue(value);			// Update the display.
		updateTimer->start();	// Reset the timer.
	}
}





	


