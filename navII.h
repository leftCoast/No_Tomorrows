#ifndef navigation_h
#define navigation_h

#include <NMEA2kBase.h>
#include <GPSReader.h>
#include <GPS_NMEA2K.h>
#include <handlers.h>
#include <RTClib.h>


#define	UTC_DELTA_E_LOC	1											// The UTC offset byte's EEPROM address. (int)
#define	MAG_CORRECT_LOC	UTC_DELTA_E_LOC + sizeof(int)		// The magnetic pole's lat lon EEPROM address. (float)

enum engComs {

	getPos = lastCom,
	getCOG,
	getGPSData,
	setMarkLat,
	setMarklon,
	getCourse,
	getDist,
	deltaUTC,
	MCorrect,
	spew
};


class navII : public	NMEA2kBase {

	public:
				navII(void);
	virtual	~navII(void);
	
	virtual	void		setup(void);
	virtual	void		loop(void);
				bool		haveMark(void);
				bool		haveMPole(void);
				float		bearingMark(bool magnetic=false);
				float		distance(void);
				float		COG(bool magnetic=false);
	virtual	void		checkAddedComs(int comVal);
	virtual	bool		addNMEAHandlers(void);			// Without handlers, who are we anyway?
	virtual	void		addCommands(void);
	virtual	void		printHelp(void);					// Print list of commands.		
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
				
				bool						haveMarkLat;
				bool						haveMarkLon;
				globalPos				destMark;
				barometerObj*			barometer;
				waterSpeedObj*			knotMeter;
				waterDepthObj*			depthSounder;
				fluidLevelObj*			fuelGauge;
				engParam*				engHdler;
				PGN0x1F904Handler*	navDataHdlr;
				int						hoursOffUTC;
				float						magCorrect;
};


extern navII  ourNavApp;
			
#endif
