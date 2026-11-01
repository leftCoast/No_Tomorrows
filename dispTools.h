#ifndef dispTools_h
#define dispTools_h

#include <drawObj.h>
#include <rectArrange.h>
#include <typeFace.h>
#include <colorRect.h>
#include <fontLabel.h>
#include <modalKbd.h>
#include <alertObj.h>
#include <handlers.h>
#include <GPSReader.h>




// ***************************************************************************************
// typeFace setup
// ***************************************************************************************



enum typeFaceChoice {
	panelTitle,
	itemLabel,
	bigValues,
	bigUnits,
	timeDatePos,
	tinyLabel,
	smallValues,
	editText
};	

extern colorObj	darkYellow;
extern void			typeFaceSetup(void);



// ***************************************************************************************
// colorCircle
// ***************************************************************************************


class colorCircle :	public drawObj,
							public colorObj {

	public:
				colorCircle(rect* inRect);
	virtual	~colorCircle(void);
	
	virtual	void	drawSelf(void);
};



// ***************************************************************************************
// valueBox
// ***************************************************************************************


// You get a number and units.
class valueBox	: public drawGroup {

	public:
				valueBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~valueBox(void);

	virtual	void	setup(void);
	virtual	void	drawSelf(void);
	virtual	void	setValue(float value);
	virtual	void	updateData(void)=0;
	virtual	void	idle(void);
			
				char*				labelTxt;
				char*				typeTxt;
				int				prec;
				float				factor;
				erasableText*	valueLabel;
				fontLabel*		unitsLabel;
				fontLabel*		typeLabel;
				int				savedIntVal;
				timeObj*			updateTimer;
				bool				isNanNow;
};



// ***************************************************************************************
// NMEABox
// ***************************************************************************************


class NMEABox	: public valueBox {

	public:
				NMEABox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~NMEABox(void);

	virtual	void	setHandler(msgHandler* inHandler);
				
				msgHandler*	ourHandler;
};



// ***************************************************************************************
// speedBox
// ***************************************************************************************


class speedBox :	public NMEABox {

	public:
				speedBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~speedBox(void);
	
	virtual	void	updateData(void);
};



// ***************************************************************************************
// depthBox
// ***************************************************************************************


class depthBox :	public NMEABox {

	public:
				depthBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~depthBox(void);
	
	virtual	void	updateData(void);
};



// ***************************************************************************************
// bearingBox
// ***************************************************************************************


class bearingBox :	public valueBox {

	public:
				bearingBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~bearingBox(void);
	
	virtual	void	updateData(void);
				
};



// ***************************************************************************************
// distanceBox
// ***************************************************************************************


class distanceBox :	public valueBox {

	public:
				distanceBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~distanceBox(void);
	
	virtual	void	updateData(void);
				
};



// ***************************************************************************************
// COGBox
// ***************************************************************************************


class COGBox :	public valueBox {

	public:
				COGBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~COGBox(void);
	
	virtual	void	updateData(void);
				
};



// ***************************************************************************************
// barometerBox
// ***************************************************************************************


class barometerBox :	public NMEABox {

	public:
				barometerBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~barometerBox(void);
	
	virtual	void	updateData(void);
				
};



// ***************************************************************************************
// fixLED
// ***************************************************************************************


class fixLED :	public drawGroup {

	public:
				fixLED(int inX,int inY);
	virtual	~fixLED(void);
	
	virtual	void	setup(void);
	virtual	void	setColors(colorObj* inOnColor,colorObj* inOffColor);
	virtual	void	idle(void);
	virtual	void	drawSelf(void);
	
				colorObj			onColor;
				colorObj			offColor;
				colorCircle*	theLED;
				bool				GPSFix;
};



// ***************************************************************************************
// GPSDateTime
// ***************************************************************************************


class GPSDateTime :	public erasableText,
						public idler {

	public:
				GPSDateTime(int inX,int inY);
	virtual	~GPSDateTime(void);
	
	virtual	void idle(void);
	//virtual	void drawSelf(void);
	
				timeObj	timer;
				char*		savedStamp;
};



// ***************************************************************************************
// GPSLatLon
// ***************************************************************************************


class GPSLatLon :	public drawGroup {

	public:
				GPSLatLon(int inX,int inY);
	virtual	~GPSLatLon(void);
	
				void	setup(void);
	virtual	void idle(void);
	virtual	void drawSelf(void);
	
				timeObj			timer;
				erasableText*	latLabel;
				erasableText*	lonLabel;
				char*				savedLat;
				char*				savedLon;
};


// **************************************************************
// valueEditDbox
// **************************************************************


class valueField;


class valueEditDbox :	public alertObj,
									public kbdUser {

public:
			valueEditDbox(valueField* inField,const char* alertMsg,screenTypes inType=sType240x320);
virtual	~valueEditDbox(void);
				
			char*			getStr(void);

			valueField*	ourValueField;
			editLabel*	editField;
};



// **********************************************************************
// valueField - A clickable field to edit a value. Brings up modal Dbox.
// **********************************************************************

enum valueType {

	aString,
	aNumber
	//aCalculation,
	//anObject
};
	
class valueField :	public erasableText,
							public listener {

	public:
				valueField(rect* inRect,valueType inType,const char* inMsg);
	virtual	~valueField(void);
	
	virtual	void	doAction(void);
	virtual	void	updateValue(const char* newValue);
	virtual	void	handleCom(stdComs comID);
	
				valueType				ourType;
				char*						ourMsg;
				valueEditDbox*	ourDbox;
				eventSet					savedEventSet;
};

#endif