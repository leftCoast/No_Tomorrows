#ifndef dispTools_h
#define dispTools_h

#include <drawObj.h>
#include <rectArrange.h>
#include <colorRect.h>
#include <fontLabel.h>
#include <navII.h>



// ***************  erasableText   ***************


class erasableText :	public fontLabel {

	public:
				erasableText(void);
				erasableText(rect* inRect);
				erasableText(int inX, int inY, int inW,int inH);
	virtual	~erasableText(void);
	
	virtual	void	drawSelf(void);	// Fixing up fontLabel..
};



// ***************   colorCircle   ***************


class colorCircle :	public drawObj,
							public colorObj {

	public:
				colorCircle(rect* inRect);
	virtual	~colorCircle(void);
	
	virtual	void	drawSelf(void);
};



// ***************     valueBox    ***************


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



// ***************     NMEABox     ***************


class NMEABox	: public valueBox {

	public:
				NMEABox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~NMEABox(void);

	virtual	void	setHandler(msgHandler* inHandler);
				
				msgHandler*	ourHandler;
};



// ***************     speedBox    ***************


class speedBox :	public NMEABox {

	public:
				speedBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~speedBox(void);
	
	virtual	void	updateData(void);
};




// ***************     depthBox    ***************


class depthBox :	public NMEABox {

	public:
				depthBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~depthBox(void);
	
	virtual	void	updateData(void);
};






class bearingBox :	public valueBox {

	public:
				bearingBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~bearingBox(void);
	
	virtual	void	updateData(void);
				
};


// ***************   distanceBox   ***************


class distanceBox :	public valueBox {

	public:
				distanceBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~distanceBox(void);
	
	virtual	void	updateData(void);
				
};



// ***************     COGBox      ***************


class COGBox :	public valueBox {

	public:
				COGBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~COGBox(void);
	
	virtual	void	updateData(void);
				
};



// ***************     fixLED      ***************


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


// *************     GPSDateTime     *************

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




// *************     GPSLatLon       *************

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


// *************     iconArrange     *************

class iconArrange :     public rectArrange {

	public:	
				iconArrange(void);
	virtual	~iconArrange(void);

	virtual	void arrangeList(void);
};











	










#endif