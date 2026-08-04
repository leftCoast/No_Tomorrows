#ifndef dispTools_h
#define dispTools_h


#include <fontLabel.h>
#include <colorRect.h>
#include <drawObj.h>
#include <navII.h>



// *********** erasableText ***********


class erasableText :	public fontLabel {

	public:
				erasableText(void);
				erasableText(rect* inRect);
				erasableText(int inX, int inY, int inW,int inH);
	virtual	~erasableText(void);
	
	virtual	void	drawSelf(void);	// Fixing up fontLabel..
};



// ************* valueBox *************


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



// *************  NMEABox  *************


class NMEABox	: public valueBox {

	public:
				NMEABox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~NMEABox(void);

	virtual	void	setHandler(msgHandler* inHandler);
				
				msgHandler*	ourHandler;
};



// *************  speedBox *************

class speedBox :	public NMEABox {

	public:
				speedBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~speedBox(void);
	
	virtual	void	updateData(void);
};



// *************  depthBox *************


class depthBox :	public NMEABox {

	public:
				depthBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~depthBox(void);
	
	virtual	void	updateData(void);
};



// ************  bearingBox ************


class bearingBox :	public valueBox {

	public:
				bearingBox(int inX,int inY,int inWidth,int inHeight,const char* inLabel,const char* inTypeTxt,int inPrec);
	virtual	~bearingBox(void);
	
	virtual	void	updateData(void);
				
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











	










#endif