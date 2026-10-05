// What we are looking at is the complete navigation application code for No tomorrows.
// Basically this is the NMEA2k processor that runs the screen for the boat and collates
// all the navigation data. It also incorporates a GPS chip attached to Serial1 for
// position.

#include <strTools.h>
#include <EEPROM.h>
#include <dispTools.h>
#include <navII.h>
#include <MSP3526_T.h>
#include <navOS.h>
#include <markList.h>

#include <debug.h>

// For MSP3526_T
#define DSP_CS    25
#define SD_CS     4
#define DSP_RST   14
#define SD_Detect 5		// grn
#define LC_DC		9		// Data/command	- YELLOW WIRE 30AWG


// For NMEA2k stuff
#define NAV_DEVICE_ID		6387				// You get 21 bits. Think serial number.
#define NAV_DEFAULT_ADDR	46					// This initial value will be set using the serial monitor.
#define NAV_DEVICE_SYSTEM	DEV_SYSTEM_NAV
#define NAV_DEVICE_FUNCT	DEV_FUNC_GNSS

navII  ourNavApp;

void bootError(const char* errStr) {
    
   screen->fillScreen(&black);   	// Fill the screen black.
   screen->setCursor(10,10);     	// Move cursor to the top left.
   screen->setTextColor(&white); 	// Drawing in white..
   screen->setTextSize(2);       	// Big enough to notice.
   screen->drawText(errStr);     	// Draw the error message.
   digitalWrite(DSP_BACKLITE,true);	// Bring up the screen.
   while(1);                     	// Lock down.
}


navII::navII(void) 
	: NMEA2kBase(NAV_DEVICE_ID,NAV_DEVICE_SYSTEM,NAV_DEVICE_FUNCT) {
	
	barometer		= NULL;
	knotMeter 		= NULL;
	depthSounder	= NULL;
	fuelGauge		= NULL;
	engHdler			= NULL;
	navDataHdlr		= NULL;
	haveMarkLat		= false;
	haveMarkLon		= false;
	//EEPROM.get(UTC_DELTA_E_LOC,timeOffset);
	//EEPROM.get(MAG_CORRECT_LOC,magCorrect);
}


// I really doubt this will ever get called. And even if it does, Adafruit typically
// doesn't make their display destructors virtual. So who knows what'll happen?
// AND.. I NEED TO DO A BETTER JOB OF CLEANUP HERE!! WHERE IS THE LIST ABOVE CLEANED UP?
navII::~navII(void) {
	
	if (barometer) delete(barometer);
	if (knotMeter) delete(knotMeter);
	if (depthSounder) delete(depthSounder);
	if (fuelGauge) delete(fuelGauge);
	if (engHdler) delete(engHdler);
	if (navDataHdlr) delete(navDataHdlr);
	if (screen) {
		delete(screen);
		screen = NULL;
	}
	if (ourGPS) {
		delete(ourGPS);
		ourGPS = NULL;
	}
}
	
	
// Setup, get the hardware running then fire up the UI & OS.
void navII::setup(void) {
	
   bool haveScreen;
   char*	prefsPath;
   
   pinMode(DSP_BACKLITE,OUTPUT);											// First setup and shut down
   digitalWrite(DSP_BACKLITE,LOW);										// our basic hardware.
   //pinMode(VIBE_PIN,OUTPUT);
   //digitalWrite(VIBE_PIN,LOW);
   
   NMEA2kBase::setup(Teensy4CAN);										// Ancestors get setup first. Sets up NMEA process.
	ourGPS = new GPSReader;													// We own the GPS reader, set it up.
	ourGPS->begin();															// Give it a kick to start it.
	ourGPS->setSpew(false);													// Shut up spew!
	Serial1.begin(9600);														// Fire up the GPS's serial port for it.
	while(Serial1.available()) Serial1.read();						// Flush out the GPS Serial data before letting it read nonsense.
	
   haveScreen = false;														// At this point, NMEA & GPS are running. Do the UI.
   screen =  (displayObj*) new MSP3526_T(DSP_CS,DSP_RST);		// Create the display.
   if (screen) {																// Got it?
       if (screen->begin()) {												// See if we can fire it up.
         screen->setRotation(PORTRAIT);								// 
         haveScreen = true;												// Everything seems good here.
      }
   }
   if (!haveScreen) {														// Screen fail?
      Serial.println("NO SCREEN!");										// Send an error out the serial port.
      while(true);															// Lock processor here forever.
   }																				//
   if (!SD.begin(SD_CS)) {													// With icons, we MUST have an SD card.
      Serial.println("NO SD CARD!");									// Send an error out the serial port.
      bootError("No SD card.");											// Since we have a display, display the error.
   }																				//
   ourEventMgr.begin();														// Kickstart our event manager.
   ourOS.begin();																// Fire up our OS sevices.
   
   prefsPath = ourOS.getProgramPath("usrPrefs");					// This has to be called AFTER ourOS.begin().
   if (prefsPath) {															// We got the prefs path?
   	setPrefsFile(prefsPath);											// Set it up!
   	readPrefs();															// Read in the prefs
   	ourGPS->setSpew(streaming);										// And in this case match hardware to saved pref.
   }
   typeFaceSetup();															// Where ever it ends up. Set it up.
}



// During loop..
void navII::loop() {
   
   NMEA2kBase::loop();	// Let our ancestors do their thing.
   ourOS.loop();			// ourOS gets a kick to pass on to the current panel.
}


void navII::setMark(navMark* newMark) {

	gPosPack inPos;
	
	inPos = newMark->getPos();
	destMark.setPos(&inPos);
	haveMarkLat = inPos.latValid;
	haveMarkLon = inPos.lonValid;
}


bool navII::haveMark(void) {

	if (haveMarkLat && haveMarkLon) {
		return destMark.valid();
	}
	return false;
}


float navII::bearingMark(bool magnetic) {
	
	float	bearingVal;
	
	bearingVal = NAN;																		// Well, assume failure.
	if (haveMark()) {																		// If we -have- a mark.
		if (ourGPS->valid) {																// And we have a valid fix..
			bearingVal = (float)ourGPS->latLon.trueBearingTo(&destMark);	// Calculate the true bearing to the mark.
			if (bearingVal<0) bearingVal = NAN;										// Got a negative? Fail.
			else if (bearingVal>360) bearingVal = NAN;							// Got more than 360? Fail.
			else if (magnetic) {															// It's a good bearing. But if magnetic though..
				bearingVal = bearingVal - magCorrect;								// We'll subtract the correction.
				if (bearingVal>=360) {													// If it's bigger n 360 now..
					bearingVal = bearingVal - 360;									// Calculate the real magnetic bearing.
				} else if (bearingVal<0) {												// If it's less n zero now..
					bearingVal = bearingVal + 360;									// Calculate the real magnetic bearing.
				}																				//
			}																					//
		}																						//
	}																							//
	return bearingVal;																	// Return the result.
}


float navII::distance(void) {
	
	float	distanceVal;

	distanceVal = NAN;
	if (haveMark()) {
		if (ourGPS->valid) {
			distanceVal = (float)ourGPS->latLon.distanceTo(&destMark);
			if (distanceVal<0) distanceVal = NAN;
		}
	}
	return distanceVal;
}


float navII::COG(bool magnetic) {
	
	float	COG;

	COG = NAN;											// Assume it'll fail.
	if (ourGPS->valid) {								// if GPS thinks we have a good fix.
		COG = (float)ourGPS->trueCourse;			// Grab the data.
		if (COG<0) {									// It's negative?!
			COG = NAN;									// Bummer data, make it a NAN.
		} else if (magnetic) {						// Else it's a good course, if magnetic though..
			COG = COG - magCorrect;					// We'll subtract the correction.
			if (COG>360) {								// If it's bigger n 360 now..
				COG = COG - 360;						// Calculate the real magnetic bearing.
			} else if (COG<0) {						// If it's less n zero now..
				COG = COG + 360;						// Calculate the real magnetic bearing.
			}																				//
		}
	}
	return COG;
}


void 	navII::addCommands(void) {

	NMEA2kBase::addCommands();
	cmdParser.addCmd(getPos,"pos");
	cmdParser.addCmd(getCOG,"cog");
	cmdParser.addCmd(getGPSData,"gpsdata");
	cmdParser.addCmd(setMarkLat,"setlat");
	cmdParser.addCmd(setMarklon,"setlon");
	cmdParser.addCmd(getCourse,"bearing");
	cmdParser.addCmd(getDist,"dist");
	cmdParser.addCmd(deltaUTC,"utc");
	cmdParser.addCmd(MCorrect,"mcorrect");
	cmdParser.addCmd(spew,"spew");
}


void navII::checkAddedComs(int comVal) {

	switch(comVal) {
		case getPos			: doGetPos();									break;
		case getCOG			: doGetCOG();									break;
		case getGPSData	: doGetData();									break;
		case setMarkLat	: haveMarkLat = doSetLat(&destMark);	break;
		case setMarklon	: haveMarkLon = doSetLon(&destMark);	break;
		case getCourse		: doGetBearing();								break;
		case getDist		: doGetDist();									break;
		case deltaUTC		: doUTC();										break;
		case MCorrect		: doMCorrect();								break;
		case spew			: doSpew();										break;
		default				: printHelp();									break;
	}
}



// Allocate and add the handlers for our different NMEA messages we are going to handle or
// create.
bool navII::addNMEAHandlers(void) {
	
	barometer		= new barometerObj(CANBrd);
	knotMeter 		= new waterSpeedObj(CANBrd);
	depthSounder	= new waterDepthObj(CANBrd);
	fuelGauge		= new fluidLevelObj(CANBrd);
	engHdler			= new engParam(CANBrd);
	navDataHdlr		= new PGN0x1F904Handler(CANBrd);
	
	if (addGPSHandlers(CANBrd)) {
		if (barometer) {
			CANBrd->addMsgHandler(barometer);
			if (knotMeter) {
				CANBrd->addMsgHandler(knotMeter);
				if (depthSounder) {
					CANBrd->addMsgHandler(depthSounder);
					if (fuelGauge) {
						CANBrd->addMsgHandler(fuelGauge);						
						if (engHdler) {
							CANBrd->addMsgHandler(engHdler);
							if (navDataHdlr) {
								CANBrd->addMsgHandler(navDataHdlr);
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}


void navII::printHelp(void) {

	NMEA2kBase::printHelp();
	Serial.println(F("                                   Navigation commands."));
	Serial.println(F("           ----------------------------------------------------------------------"));
	Serial.println(F("Pos         Shows our current GPS position."));
	Serial.println(F("COG         Shows our current course over ground."));
	Serial.println(F("GPSData     Shows us data about our GPS fix."));
	Serial.println(F("setLat      Set latitude of mark."));
	Serial.println(F("setLon      Set longitude of mark."));
	Serial.println(F("bearing     Get TRUE course from here to mark."));
	Serial.println(F("dist        Get nautical miles from here to mark."));
	Serial.println(F("UTC         Get our time delta from UTC. Or, if a value is added, set it."));
	Serial.println(F("mCorrect    Get or set correction value from true to magnetic course."));
	Serial.println(F("spew        spew toggles GPS data spewing. Adding on or off works too."));
}


/*
// This is just for checking to see if all the different ways of formatting a position are
// valid. There's quite a few so this prints a position out to the Serial monitor in
// every way possible.
void navII::posTypeTest(void) {
		
	char* 	latStr = NULL;
	char* 	lonStr = NULL;
	gPosPack	aPos;
	
	Serial.println("--- Start ---");
	aPos = destMark.getPos();
	showGPosPack(&aPos);
	Serial.println("---------");
	for (posFormat i=floatDeg;i<=quad_intDeg_intMin_floatSec;i=i+1) {
		Serial.println("---------");
		heapStr(&latStr,destMark.getLatStr(i));
		Serial.println(latStr);
		heapStr(&lonStr,destMark.getLonStr(i));
		Serial.println(lonStr);
		Serial.println("parsing");
		aPos = ourPosParser.parsePos(latStr,lonStr);
		showGPosPack(&aPos);
		Serial.println("---------");
	}
	freeStr(&latStr);
	freeStr(&lonStr);			
}	
*/


// This one seems to have had issues overwriting the reused string, while the first one
// was being sent to the host computer. So, I tried doing a local copy. That seems to have
// solved the issue.
void navII::doGetPos(void) {
	
	char* outStr = NULL;					
	
	Serial.print(F("Latitude          : "));											// Send to first bit..
	heapStr(&outStr,ourGPS->latLon.getLatStr(intDeg_floatMin_quad));			// Save off a local copy of the string.
	Serial.println(outStr);																	// Send out the local copy, while..
	Serial.print(F("Longitude         : "));											// Send out the second label.
	heapStr(&outStr,ourGPS->latLon.getLonStr(intDeg_floatMin_quad));			// Save off a copy of the second string.
	Serial.println(outStr);																	// Send out the local copy.
	freeStr(&outStr);																			// Release the local string memory.
}


void navII::doGetCOG(void) {
	
	float	COG;
	
	COG =  ourGPS->trueCourse;
	Serial.print(F("COG True          : "));
	Serial.print(COG,1);
	Serial.println(F(" Deg."));
	Serial.print(F("Mag. deviation    : "));
	Serial.print(ourGPS->magVar,3);
	Serial.print(F(" "));
	if (ourGPS->vEastWest=='E') {
		Serial.println(F(" East"));
	} else {
		Serial.println(F(" West"));
	}
	Serial.print(F("COG Magnetic      : "));
	Serial.print(COG,1);
	Serial.println(F(" Deg."));
}


void navII::doGetData(void) {

	Serial.print(F("Date              : "));
   Serial.print(ourGPS->month);
   Serial.print(F("/"));
	Serial.print(ourGPS->day);
	Serial.print(F("/"));
	Serial.println(ourGPS->year);
	
	Serial.print(F("Time              : "));
	Serial.print(ourGPS->hours);
	Serial.print(F(":"));
	Serial.print(ourGPS->min);
	Serial.print(F(":"));
	Serial.print(ourGPS->sec,0);
	Serial.println(F(" Zulu."));
	
	doGetPos();
	doGetCOG();
	
	Serial.print(F("Speed over ground : "));
	Serial.print(ourGPS->groudSpeedKnots,1);
	Serial.println(F(" kn."));
	
	Serial.print(F("Fix quality       : "));
	switch(ourGPS->qualVal) {
		case fixInvalid	: Serial.println(F("Fix invalid."));					break;
		case fixByGPS		: Serial.println(F("Fix by GPS."));						break; 
		case fixByDGPS		: Serial.println(F("Fix by differentail GPS."));	break;  
	}
	
	Serial.print(F("Number satellites : "));
	Serial.println(ourGPS->numSatellites);
	
	Serial.print(F("Altitude          : "));
	Serial.println(ourGPS->altitude,2);
	Serial.print(F("Geoid height      : "));
   Serial.println(ourGPS->GeoidalHeight);
   Serial.print(F("Age of data       : "));
   Serial.println(ourGPS->ageOfDGPSData);
   Serial.print(F("GPS Station ID    : "));
   Serial.println(ourGPS->DGPSStationID);		
				
	Serial.print(F("GPS mode          : "));
	switch(ourGPS->operationMode) {
		case manual	: Serial.println(F("Manual mode."));
		case automatic	: Serial.println(F("Automatic mode."));
	}
	
   Serial.print(F("Fix type          : "));
	switch(ourGPS->fixType) {
		case noFix	: Serial.println(F("No fix available."));	break;
		case twoD	: Serial.println(F("2D fix only."));		break;
		case threeD	: Serial.println(F("3D Fix."));				break;
	}
	Serial.print(F("Satellite IDs     :\t"));
	for(int i=0;i<11;i++) {
		if (ourGPS->SVID[i]) {
			Serial.print(ourGPS->SVID[i]);
		} else {
			Serial.print(F(".."));
		}
		Serial.print(F("\t"));
	}
	Serial.println();
	
	Serial.print(F("PDOP              :"));Serial.println(ourGPS->PDOP,1);
	Serial.print(F("HDOP              :"));Serial.println(ourGPS->HDOP,1);
	Serial.print(F("VDOP              :"));Serial.println(ourGPS->VDOP,1);
	
	int		numItems;
	satData*	dataNode;
	
	numItems = ourGPS->satInViewList.getCount();
	Serial.println(F("-------------------------------"));
	Serial.print(F("Satelites in view : "));Serial.println(numItems);
	Serial.println(F("....."));
	for (int i=0;i<numItems;i++) {
		dataNode = (satData*)ourGPS->satInViewList.getByIndex(i);
		if (dataNode) {
			Serial.print(F("Satellite ID      : "));Serial.println(dataNode->PRNNum);
			Serial.print(F("Elevation         : "));Serial.println(dataNode->elevation);
			Serial.print(F("Azimuth           : "));Serial.println(dataNode->azimuth);
			Serial.print(F("Sig / Noise       : "));Serial.println(dataNode->SigToNoise);
			if (i==numItems-1) {
				Serial.println(F("-------------------------------"));
			} else {
				Serial.println(F("....."));
			}
		}
	}
}


bool navII::doSetLat(globalPos* inPos) {
	
	gPosPack	aPos;
	
	if (cmdParser.numParams()) {												// We got any params..
		aPos = ourPosParser.parsePos(cmdParser.getParamBuff()," ");	// Toss the entire parameter buff in here..
		if (aPos.latValid) {														// If this passed the sanity text..
			inPos->setLat(&aPos);													// Write it to our destination mark.
			Serial.print(F("Latitude was set to : "));						// Tell the user everything was ok.
			Serial.println(inPos->getLatStr());									// Show the value.
			return true;																// Tell 'em it worked.
		} else {																			// Else we give them a kick to do better next time.
			Serial.println(F("We're looking for either, latitude value & quadrant, (N/S kinda' thing)."));
			Serial.println(F("Or, latitude degree value, minute value and then quadrant."));
			Serial.println(F("I can't make what you typed match any of these."));
		}
	}
	return false;
}


bool navII::doSetLon(globalPos* inPos) {

	gPosPack	aPos;
	
	if (cmdParser.numParams()) {												// We got any params..
		aPos = ourPosParser.parsePos(" ",cmdParser.getParamBuff());	// Toss the entire parameter buff in here..
		if (aPos.lonValid) {														// If this passed the sanity text..
			inPos->setLon(&aPos);													// Write it to our destination mark.
			Serial.print(F("Latitude was set to : "));						// Tell the user everything was ok.
			Serial.println(inPos->getLonStr());									// Show the value.
			return true;																// Tell 'em it worked.
		} else {																			// Else we give them a kick to do better next time.
			Serial.println(F("We're looking for, longitude value & quadrant, (E/W kinda' thing)."));
			Serial.println(F("Or, longitude degree value, minute value and then quadrant."));
			Serial.println(F("I can't make what you typed match any of these."));
		}
	}
	return false;
}


void navII::doGetBearing(void) {

	float	bearDegT;
	
	if (haveMark()) {
		if (ourGPS->qualVal!=fixInvalid) {
			bearDegT = bearingMark();
			Serial.print(bearDegT,0);
			Serial.println(F(" Degrees true."));
		} else {
			Serial.println(F("We don't have a valid position fix."));
		}
	} else {
		Serial.println(F("We don't have a marker to aim at."));
	}
}


void navII::doGetDist(void) {

	float	dist;
	
	if (haveMark()) {
		if (ourGPS->qualVal!=fixInvalid) {
			dist = ourGPS->latLon.distanceTo(&destMark);
			Serial.print(dist,1);
			Serial.println(F(" Nautical miles."));
		} else {
			Serial.println(F("We don't have a valid position fix."));
		}
	} else {
		Serial.println(F("We don't have a marker to aim at."));
	}
}


void navII::doUTC(void) {
	
	int	UTCOffset;
	
	if (cmdParser.numParams()==0) {									// If we're looking at no params..
		Serial.print(F("Time offset from UTC : "));				// Show 'em what we have.
		Serial.println(timeOffset);									//
	} else if (cmdParser.numParams()==1) {							// If we got a param..
		UTCOffset = atoi(cmdParser.getNextParam());				// Decode it as an integer.
		if (UTCOffset>=-12&&UTCOffset<=12) {						// Sanity check.
			timeOffset = UTCOffset;									// We can use this value.
			savePrefs();													// Save them in the file.
			Serial.print(F("Time offset from UTC set to : "));	// Tell 'em
			Serial.println(timeOffset);								//
		}																		// 
	} else {																	// Really? Just tell em what we want.
		Serial.println(F("Looking for either no param. I'll show you the offset."));
		Serial.println(F("Or one param and I'll set that as offset for you."));
	}
}


void navII::doMCorrect(void) {
	
	float	value;
	
	if (cmdParser.numParams()==0) {																		// If we're looking at no params..
		Serial.print(F("Magnetic correction from true : "));										// We tell 'em..
		Serial.println(magCorrect);																		// What we have.
	} else if (cmdParser.numParams()==1) {																// If we got one param..
		value = atof(cmdParser.getNextParam());														// Decode it as a float.
		if (value<=180&&value>=-180) {																	// Sanity check.
			magCorrect = value;																				// We can use this value.
			savePrefs();																						// Save them in the file.
			Serial.print(F("Magnetic correction  set to : "));										// Tell 'em
			Serial.println(magCorrect);																	//
		} else {																									// Else wacky value?
			Serial.println(F("Sorry, looking for a value between -180 & 180 degrees."));	// Tell 'em no.
		}																											// 
	} else {																										// Else the wrong number of params.
		Serial.println(F("Looking for either no param. I'll show you the correction."));	// Tell 'em.
		Serial.println(F("Or one param and I'll set that as correction for you."));		// At length.
	}																												//
}


// Turn GPS raw text data out the serial port on or off. (PC or Mac)		
void navII::doSpew(void) {

	if (cmdParser.numParams()==0) {								// If we're looking at no params..
		ourGPS->setSpew(!(ourGPS->spew));						// We toggle spewing.
	} else if (cmdParser.numParams()==1) {						// If we're looking at one param..
		if (!strcmp(cmdParser.getNextParam(),"on")) {		// If we get "on"..
			ourGPS->setSpew(true);									// We force it to spew data.
		} else {															// Else, anything else..
			ourGPS->setSpew(false);									// We shut the spewing off.
		}																	//
	}																		//
	streaming = ourGPS->spew;										// The user just set what she wants. Make it so.
	savePrefs();														// And save it.
	if (!ourGPS->spew) Serial.print(F("Spewing off."));	// We ONLY say when it's off. Else it gets into spew stream.
}
		
		
		
		
