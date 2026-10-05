#include <dispTools.h>
#include <strTools.h>
#include <debug.h>

colorObj	darkYellow;

// ***************************************************************************************
// typeFace setup.
// ***************************************************************************************	


void typeFaceSetup(void) {

	typeFace* aTypeFace;
	
	darkYellow.setColor(&yellow);
	darkYellow.blend(&black,50);
	
	aTypeFace = new typeFace(panelTitle);
	if (aTypeFace) {
		aTypeFace->foreColor.setColor(&yellow);
		aTypeFace->backColor.setColor(&black);
		aTypeFace->transperent	= false;
		aTypeFace->justify		= TEXT_CENTER;
		aTypeFace->useFonts		= true;
		aTypeFace->saveFont(AFF_SANS_BOLD_12_OB);
		ourTxtPallette.addTypeFace(aTypeFace);
	}
	aTypeFace = new typeFace(itemLabel);
	if (aTypeFace) {
		aTypeFace->foreColor.setColor(&darkYellow);
		aTypeFace->backColor.setColor(&black);
		aTypeFace->transperent	= false;
		aTypeFace->justify		= TEXT_LEFT;
		aTypeFace->useFonts		= true;
		aTypeFace->saveFont(AFF_SANS_BOLD_9_OB);
		ourTxtPallette.addTypeFace(aTypeFace);
	}
	aTypeFace = new typeFace(bigValues);
	if (aTypeFace) {
		aTypeFace->foreColor.setColor(&yellow);
		aTypeFace->backColor.setColor(&black);
		aTypeFace->transperent	= true;
		aTypeFace->justify		= TEXT_LEFT;
		aTypeFace->useFonts		= true;
		aTypeFace->saveFont(AFF_SANS_BOLD_24_OB);
		ourTxtPallette.addTypeFace(aTypeFace);
	}
	aTypeFace = new typeFace(bigUnits);
	if (aTypeFace) {
		aTypeFace->foreColor.setColor(&darkYellow);
		aTypeFace->backColor.setColor(&black);
		aTypeFace->transperent	= true;
		aTypeFace->justify		= TEXT_LEFT;
		aTypeFace->useFonts		= true;
		aTypeFace->saveFont(AFF_SANS_BOLD_12_OB);
		ourTxtPallette.addTypeFace(aTypeFace);
	}
	aTypeFace = new typeFace(timeDatePos);
	if (aTypeFace) {
		aTypeFace->foreColor.setColor(&yellow);
		aTypeFace->backColor.setColor(&black);
		aTypeFace->transperent	= false;
		aTypeFace->justify		= TEXT_LEFT;
		aTypeFace->useFonts		= true;
		aTypeFace->saveFont(AFF_MONO_12);
		ourTxtPallette.addTypeFace(aTypeFace);
	}
	aTypeFace = new typeFace(tinyLabel);
	if (aTypeFace) {
		aTypeFace->foreColor.setColor(&yellow);
		aTypeFace->backColor.setColor(&black);
		aTypeFace->transperent	= true;
		aTypeFace->justify		= TEXT_LEFT;
		aTypeFace->useFonts		= true;
		aTypeFace->saveFont(AFF_SANS_9_OB);
		ourTxtPallette.addTypeFace(aTypeFace);
	}
	aTypeFace = new typeFace(smallValues);
	if (aTypeFace) {
		aTypeFace->foreColor.setColor(&yellow);
		aTypeFace->backColor.setColor(&black);
		aTypeFace->transperent	= true;
		aTypeFace->justify		= TEXT_RIGHT;
		aTypeFace->useFonts		= true;
		aTypeFace->saveFont(AFF_SANS_BOLD_9_OB);
		ourTxtPallette.addTypeFace(aTypeFace);
	}
}


// ***************************************************************************************
// colorCircle
// ***************************************************************************************


colorCircle::colorCircle(rect* inRect)
	: drawObj(inRect), colorObj() { setColor(LC_RED); }
	
	
colorCircle::~colorCircle(void) {  }
	

void colorCircle::drawSelf(void) {

	int	dia;
	
	dia = (width+height)/2;					// Grab average for radius.
	screen->fillCircle(x,y,dia,this);	// We -are- a colorObj so draw a circle of our color.
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
	setup();
}
	
	
valueBox::~valueBox(void) {

	freeStr(&labelTxt);
	freeStr(&typeTxt);
	if (updateTimer) delete(updateTimer);
}


void valueBox::setup(void) {
	
	valueLabel = new erasableText();
	if (valueLabel) {
		valueLabel->x = 5;
		valueLabel->y = 0;
		valueLabel->width = 190;
		ourTxtPallette.setTypeFace(valueLabel,bigValues);
		valueLabel->setPrecision(prec);							// we only know this now.
		valueLabel->setValue("-- ");
		addObj(valueLabel);
	}
	unitsLabel = new fontLabel();
	if (unitsLabel) {
		unitsLabel->x = 200;
		unitsLabel->y = 14;
		unitsLabel->width = 80;
		ourTxtPallette.setTypeFace(unitsLabel,bigUnits);
		unitsLabel->setValue(labelTxt);
		freeStr(&labelTxt);
		addObj(unitsLabel);
	}
	typeLabel = new fontLabel();
	if (typeLabel) {
		typeLabel->x = 5;
		typeLabel->y = 45;
		typeLabel->width = 140;
		ourTxtPallette.setTypeFace(typeLabel,itemLabel);
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
		savedIntVal = round(value * factor);			// Setup an integer version of the value.
		valueLabel->setValue(savedIntVal/factor);		// Set the new value to the screen.
		isNanNow = false;										// And we are no longer showing a NAN.
	} else {														// Else.. 
		newIntVal = round(value * factor);				// Setup an integer version of the value.
		if (newIntVal!=savedIntVal) {						// If it's different than the saved integer of what we have now..
			valueLabel->setValue(newIntVal/factor);	// Set the new value to the screen.
			savedIntVal = newIntVal;						// Update the saved int value.
		}
	}
}


void valueBox::idle(void) {
	
	drawGroup::idle();			// Let our parent have a go.
	if (updateTimer->ding()) {	// If our timer goes off..
		updateData();				// Time to see if we ned to update.
		updateTimer->start();	// Reset the timer.
	}
}



// *************  NMEABox  *************


NMEABox::NMEABox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: valueBox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) {  }

	
NMEABox::~NMEABox(void) {  }


void NMEABox::setHandler(msgHandler* inHandler) { ourHandler = inHandler; }



// *************  speedBox *************


speedBox::speedBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: NMEABox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) {  }
	
	
speedBox::~speedBox(void) {  }
	
	
void speedBox::updateData(void) {

	waterSpeedObj* speedo;
	
	if (ourHandler) {
		speedo = (waterSpeedObj*)ourHandler;
		setValue(speedo->knots);
	}
}



// *************  depthBox *************


depthBox::depthBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: NMEABox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) {  }
	
	
depthBox::~depthBox(void) {  }

	
void depthBox::updateData(void) {

	waterDepthObj* sounder;
	
	if (ourHandler) {
		sounder = (waterDepthObj*)ourHandler;
		setValue(sounder->feet/6.0);
	}
}



// ************  bearingBox ************

	
bearingBox::bearingBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: valueBox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) { }
	
	
bearingBox::~bearingBox(void) {  }


void bearingBox::updateData(void) {
	
	double	bearing;
	
	bearing = NAN;
	if (ourGPS->valid) {								// Got all the bits?
		if (ourNavApp.haveMark()) {
			bearing = ourNavApp.bearingMark(true);
		}	
	}
	setValue(bearing);																	
}



// ************ distanceBox ************

	
distanceBox::distanceBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: valueBox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) { }
	
	
distanceBox::~distanceBox(void) {  }


void distanceBox::updateData(void) {
	
	double	distance;
	
	distance = NAN;
	if (ourGPS->valid) {								// Got all the bits?
		if (ourNavApp.haveMark()) {
			distance = ourNavApp.distance();
		}	
	}
	setValue(distance);																	
}



// ************ COGBox ************

	
COGBox::COGBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: valueBox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) { }
	
	
COGBox::~COGBox(void) {  }


void COGBox::updateData(void) {
	
	double	COG;
	
	COG = NAN;
	if (ourGPS->valid && ourGPS->groudSpeedKnots>=1) {		// Got all the bits?
		COG = ourNavApp.COG(true);	
	}
	setValue(COG);																	
}


// ************ barometerBox ************

	
barometerBox::barometerBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: NMEABox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) { }
	
	
barometerBox::~barometerBox(void) {  }


void barometerBox::updateData(void) {
	
	barometerObj* baro;
	
	if (ourHandler) {
		baro = (barometerObj*)ourHandler;
		setValue(baro->inHg);
	}																	
}


// *************     fixLED      *************


fixLED::fixLED(int inX,int inY)
	: drawGroup(inX,inY,40,18) {
	
	GPSFix	= false;
	setup();
}
	
fixLED::~fixLED(void) {  }
	
	
void fixLED::setColors(colorObj* inOnColor,colorObj* inOffColor) {

	onColor.setColor(inOnColor);
	offColor.setColor(inOffColor);
	setNeedRefresh();
}


void fixLED::idle(void) { 
	
	drawGroup::idle();				// Just in case..
	if (theLED) {
		if (GPSFix!=ourGPS->valid) {
			if (ourGPS->valid) { 
				theLED->setColor(&onColor);
			} else {
				theLED->setColor(&offColor);
			}
			GPSFix = ourGPS->valid;
			setNeedRefresh();
		}
	}
}


void fixLED::setup(void) {

	fontLabel*	fixText;
	rect			ledRect(28,3,12,12);
		
	fixText = new fontLabel(0,0,20,18);
	ourTxtPallette.setTypeFace(fixText,tinyLabel);
	fixText->setValue("Fix");
	addObj(fixText);
	
	theLED = new colorCircle(&ledRect);
	addObj(theLED);
	hookup();
}


void fixLED::drawSelf(void) { /*screen->drawRect(this,&cyan);*/ }



// *************     GPSDateTime      *************


GPSDateTime::GPSDateTime(int inX,int inY)
	: erasableText(inX,inY,242,32) {
	
	timer.setTime(250);			// How often to check the clock.
	savedStamp = NULL;
	heapStr(&savedStamp," ");	// A string defualt.
	ourTxtPallette.setTypeFace(this,timeDatePos);
	hookup();
}
	
		
GPSDateTime::~GPSDateTime(void) {  }


void GPSDateTime::idle(void) {

	char	outStr[40];
	
	if (timer.ding()) {
		DateTime	timeStamp(ourGPS->year,ourGPS->month,ourGPS->day,ourGPS->hours,ourGPS->min,ourGPS->sec);
		TimeSpan	deltaTime(0,timeOffset,0,0);
		if (ourGPS->valid) {
			timeStamp = timeStamp + deltaTime;
			sprintf(outStr,"%02d/%02d/%4d  %02d:%02d",
			timeStamp.month(),
			timeStamp.day(),
			timeStamp.year(),
			timeStamp.hour(),
			timeStamp.minute());
		} else {
			sprintf(outStr,"--/--/----  --:--");
		}
		if (strcmp(savedStamp,outStr)) {
			heapStr(&savedStamp,outStr);			
			setValue(outStr);
		}
		timer.start();
	}	
}



// *************     GPSLatLon      *************


GPSLatLon::GPSLatLon(int inX,int inY)
	: drawGroup(inX,inY,240,44) {
	
	timer.setTime(250);			// How often to check the GPS.
	savedLat = NULL;
	heapStr(&savedLat," ");		// A string defualt.
	savedLon = NULL;
	heapStr(&savedLon," ");		// Another string defualt.
	setup();
}
	
		
GPSLatLon::~GPSLatLon(void) {  }


void GPSLatLon::setup(void) {

	latLabel = new erasableText(0,0,width,height);				// Create the label
	ourTxtPallette.setTypeFace(latLabel,timeDatePos);
	addObj(latLabel);															// Hook it up.
	lonLabel = new erasableText(0,24,width,height);	//
	ourTxtPallette.setTypeFace(lonLabel,timeDatePos);
	addObj(lonLabel);															//
	hookup();																	// Fire up the machine.
}


void GPSLatLon::idle(void) {

	char	outStr[40];
	char*	posStr;
			
	if (timer.ding()) {
		if (ourGPS->valid) {
			posStr = ourGPS->latLon.getLatStr(intDeg_floatMin_quad);
			if (strcmp(savedLat,posStr)) {
				latLabel->setValue(posStr);
				heapStr(&savedLat,posStr);
			}
			posStr = ourGPS->latLon.getLonStr(intDeg_floatMin_quad);
			if (strcmp(savedLon,posStr)) {
				lonLabel->setValue(posStr);
				heapStr(&savedLon,posStr);
			}
		} else {
			sprintf (outStr,"     ---.------");
			if (strcmp(savedLat,outStr)) {
				latLabel->setValue(outStr);
				heapStr(&savedLat,outStr);
			}
			if (strcmp(savedLon,outStr)) {
				lonLabel->setValue(outStr);
				heapStr(&savedLon,outStr);
			}
		}
		timer.start();
	}
}


void GPSLatLon::drawSelf(void) {

	//screen->drawRect(this,&red);
}
