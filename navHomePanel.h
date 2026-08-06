#ifndef navHomePanel_h
#define navHomePanel_h

//#include <MSP3526_T.h> 
#include <bmpObj.h> 
#include <navII.h>
#include <lilOS.h>
#include <dispTools.h>

#include <debug.h>

// *****************************************************
//                      navHomePanel
// *****************************************************


class navHomePanel : public homePanel {

	public:
				navHomePanel(void);
	virtual	~navHomePanel(void);

	virtual	void	setup(void);
	virtual	void	loop(void);
	virtual	void	drawSelf(void);
				
};

#endif
