#include <dispTools.h>
#include <strTools.h>

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


	
// *************  NMEABox  *************


NMEABox::NMEABox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec)
	: valueBox(inX,inY,inWidth,inHeight,inLabel,inTypeTxt,inPrec) {  }

	
NMEABox::~NMEABox(void) {  }


void NMEABox::setHandler(msgHandler* inHandler) { ourHandler = inHandler; }

/*
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
*/

		
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

	colorObj	darkYellow;
	
	darkYellow.setColor(&yellow);
	darkYellow.blend(&black,50);
	
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
	
	drawGroup::idle();			// Let our parent have a go.
	if (updateTimer->ding()) {	// If our timer goes off..
		updateData();				// Time to see if we ned to update.
		updateTimer->start();	// Reset the timer.
	}
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


