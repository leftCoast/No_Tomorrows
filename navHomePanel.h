#ifndef navHomePanel_h
#define navHomePanel_h

//#include <adafruit_2050.h>

#include <MSP3526_T.h>
#include <fontLabel.h>
#include <colorRect.h> 
#include <bmpObj.h> 
#include <rectArrange.h>
#include <navII.h>
#include <lilOS.h>
#include <dispTools.h>

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
				//NMEABox*			knotGauge;
				//NMEABox*			depthGauge;
				//GPSBox*			bearingGauge;
				//GPSBox*			distanceGauge;
				//NMEABox*			barometerGauge;
				char*				savedStamp;
				//timeObj     	timer;
};

#endif
