#ifndef navHomePanel_h
#define navHomePanel_h

//#include <adafruit_2050.h>

#include <MSP3526_T.h>
#include <fontLabel.h>
//#include <label.h>
#include <colorRect.h> 
#include <bmpObj.h> 
#include <rectArrange.h>
#include <navII.h>
#include <lilOS.h>
#include <debug.h>

// *****************************************************
//                      iconArrange
// *****************************************************

class iconArrange :     public rectArrange {

	public:	
				iconArrange(void);
	virtual	~iconArrange(void);

	virtual	void arrangeList(void);
};



// *********** erasableText ***********


class erasableText :	public fontLabel {

	public:
				erasableText(void);
				erasableText(rect* inRect);
				erasableText(int inX, int inY, int inW,int inH);
	virtual	~erasableText(void);
	
	virtual	void	drawSelf(void);	// Fixing up fontLabel..
};



// *************** LED  ***************


class LED :	public colorRect {

	public:
				LED(rect* inRect,colorObj* inOnColor,colorObj* inOffColor);
	virtual	~LED(void);
	
	virtual	void	setColors(colorObj* inOnColor,colorObj* inOffColor);
	virtual	void	setState(bool onOff);
	virtual	void	drawSelf(void);
	
				colorObj	offColor;
				colorObj	onColor;
				bool		ourState;
};



// ************* valueBox *************


// You get a number and units.
class valueBox	: public drawGroup {

	public:
				valueBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~valueBox(void);

	virtual	void	setup(int inDataChoice=0);
	virtual	void	drawSelf(void);
	virtual	void	setValue(float inValue);
	virtual	float	checkData(void)=0;
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
				int				dataChoice;
};



// *************  NMEABox  *************


class NMEABox	: public valueBox {

	public:
				NMEABox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~NMEABox(void);

	virtual	float	checkData(void);
};



// *************  GPSBox  *************


class GPSBox	: public valueBox {

	public:
				GPSBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~GPSBox(void);

	virtual	float	checkData(void);
};


// *****************************************************
//                      navHomePanel
// *****************************************************


class navHomePanel : public homePanel {

	public:
				navHomePanel(void);
	virtual	~navHomePanel(void);

	virtual	void	setup(void);
	virtual	void	loop(void);
	         void	showPos(globalPos* fix);
	virtual	void	drawSelf(void);
	
				LED*				fixLED;
				erasableText*	timeLabel;
				erasableText*	latLabel;
				erasableText*	lonLabel;
				NMEABox*			knotGauge;
				NMEABox*			depthGauge;
				GPSBox*			bearingGauge;
				GPSBox*			distanceGauge;
				NMEABox*			barometerGauge;
				char*				savedStamp;
				timeObj     	timer;
};

#endif
