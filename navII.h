#ifndef navII_h
#define navII_h

#include <NMEA2kBase.h>
#include <handlers.h>
#include <navigation.h>
#include <dispTools.h>
#include <navPrefs.h>

//#define	UTC_DELTA_E_LOC	1											// The UTC offset byte's EEPROM address. (int)
//#define	MAG_CORRECT_LOC	UTC_DELTA_E_LOC + sizeof(int)		// The magnetic pole's lat lon EEPROM address. (float)

enum engComs {

	getPos = lastCom,
	getCOG,
	getGPSData,
	getCourse,
	getDist,
	deltaUTC,
	MCorrect,
	spew,
	readPrefsFile,
	lastNavIICom
};


class navII : public	NMEA2kBase {

	public:
				navII(void);
	virtual	~navII(void);
	
	virtual	void		setup(void);
	virtual	void		loop(void);
	virtual	void		checkAddedComs(int comVal);
	virtual	bool		addNMEAHandlers(void);			// Without handlers, who are we anyway?
	virtual	void		addCommands(void);
	virtual	void		printHelp(void);					// Print list of commands.		
				//void 		posTypeTest(void);
				void		doGetPos(void);
				void		doGetCOG(void);
				void		doGetData(void);
				bool		doSetLat(globalPos* inPos);
				bool		doSetLon(globalPos* inPos);
				void		doGetBearing(void);
				void		doGetDist(void);
				void		doUTC(void);
				void		doMCorrect(void);
				void		doSpew(void);
				void 		doReadPrefs(void);
				
				fluidLevelObj*			fuelGauge;
				engParam*				engHdler;
				PGN0x1F904Handler*	navDataHdlr;
};


extern navII ourNavApp;
			
#endif
