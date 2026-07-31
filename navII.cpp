// What we are looking at is the complete navigation application code for No tomorrows.
// Basically this is the NMEA2k processor that runs the screen for the boat and collates
// all the navigation data. It also incorporates a GPS chip attached to Serial1 for
// position.

//#include <displayObj.h>
#include <strTools.h>
#include <EEPROM.h>

#include <navII.h>
#include <MSP3526_T.h>
#include <navOS.h>

//#include <debug.h>

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
   digitalWrite(DSP_LED,true);	// Bring up the screen.
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
	EEPROM.get(UTC_DELTA_E_LOC,hoursOffUTC);
	EEPROM.get(MAG_CORRECT_LOC,magCorrect);
}


// I really doubt this will ever get called. And even if it does, Adafruit typically
// doesn't make their display destructors virtual. So who knows what'll happen?
// AND.. I NEED TO DO A BETTER JOB OF CLEANUP HERE!! WHERE IS THE LIST ABOVE CLEANED UP?
navII::~navII(void) {
	
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
   
   pinMode(DSP_LED,OUTPUT);											// First setup and shut down
   digitalWrite(DSP_LED,LOW);											// our basic hardware.
   pinMode(VIBE_PIN,OUTPUT);
   digitalWrite(VIBE_PIN,LOW);
   
   NMEA2kBase::setup();														// Ancestors get setup first. Sets up NMEA process.
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
   //Serial.println("I think everything seemed to go ok..");
}



// During loop..
void navII::loop() {
   
   NMEA2kBase::loop();	// Let our ancestors do their thing.
   ourOS.loop();			// ourOS gets a kick to pass on to the current panel.
	fillNavPGN();			// Check data and send it off to the NMEA2K stuff.
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
			else if (bearingVal>360)  bearingVal = NAN;							// Got more than 360? Fail.
			else if (magnetic) {															// Else it's a good bearing, if magnetic though..
				bearingVal = bearingVal + magCorrect;								// We'll add the correction.
				if (bearingVal>360) {													// If it's bigger n 360 now..
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


void navII::fillNavPGN(bool inMagnetic) {

	if (haveMark() && ourGPS->valid) {
		navDataHdlr->distToWP = distance();								// Set distance in NM.
		navDataHdlr->courseToWP = bearingMark(inMagnetic);			// Set the course, magnetic or true.
		navDataHdlr->magnetic = inMagnetic;								// Tell 'em what we chose.
		navDataHdlr->perpCrossed = true;//false;								// Crossed the perpendicular? Donno'.
		navDataHdlr->inMinRange = true;//false;									// Close enough to say we're there? Donno'.
		navDataHdlr->greatCircle = true;									// We only do great circle.k
		navDataHdlr->ETAHours = 0;											// NEED TO WORK ON THIS ONE.
		navDataHdlr->ETADate = 0;											// NEED TO WORK ON THIS ONE. Days since 1 January 1970! :D
		navDataHdlr->bearingFromStart = navDataHdlr->courseToWP;	// Only doing one WP so this IS the start.
		navDataHdlr->bearingFromFix = navDataHdlr->courseToWP;	// Ditto.
		navDataHdlr->startWPNum = 0;										// Not using WP numbers at this time.
		navDataHdlr->endWPNum = 0;											// Ditto.
		navDataHdlr->endPos.copyPos(&destMark);						// Where we want to go.
		navDataHdlr->knMadeGood = 0;										// NEED TO SETUP LOG.
	} else {
		navDataHdlr->distToWP = 0;
		navDataHdlr->courseToWP = 0;
		navDataHdlr->magnetic = 0;
		navDataHdlr->perpCrossed = 0;
		navDataHdlr->inMinRange = 0;
		navDataHdlr->greatCircle = 0;
		navDataHdlr->ETAHours = 0;
		navDataHdlr->ETADate = 0;				
		navDataHdlr->bearingFromStart = 0;
		navDataHdlr->bearingFromFix = 0;
		navDataHdlr->startWPNum = 0;
		navDataHdlr->endWPNum = 0;
		navDataHdlr->knMadeGood = 0;
	}
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
	
	barometer		= new barometerObj(llamaBrd);
	knotMeter 		= new waterSpeedObj(llamaBrd);
	depthSounder	= new waterDepthObj(llamaBrd);
	fuelGauge		= new fluidLevelObj(llamaBrd);
	engHdler			= new engParam(llamaBrd);
	navDataHdlr		= new PGN0x1F904Handler(llamaBrd);
	
	if (addGPSHandlers(llamaBrd)) {
		if (barometer) {
			llamaBrd->addMsgHandler(barometer);
			if (knotMeter) {
				llamaBrd->addMsgHandler(knotMeter);
				if (depthSounder) {
					llamaBrd->addMsgHandler(depthSounder);
					if (fuelGauge) {
						llamaBrd->addMsgHandler(fuelGauge);						
						if (engHdler) {
							llamaBrd->addMsgHandler(engHdler);
							if (navDataHdlr) {
								llamaBrd->addMsgHandler(navDataHdlr);
								return true;
							}
						}
					}
				}
			}
		}
		//return false;
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
	Serial.println(F("spew			spew toggles GPS data spewing. Adding on or off works too."));
}


// This one seems to have had issues overwriting the reused string, while the first one
// was being sent to the host computer. So, I tried doing a local copy. That seems to have
// solved the issue.
void navII::doGetPos(void) {
	
	char* outStr = NULL;					
	
	Serial.print(F("Latitude          : "));			// Send to first bit..
	heapStr(&outStr,ourGPS->latLon.showLatStr());	// Save off a local copy of the string.
	Serial.println(outStr);									// Send out the local copy, while..
	Serial.print(F("Longitude         : "));			// Send out the second label.
	heapStr(&outStr,ourGPS->latLon.showLonStr());	// Save off a copy of the second string.
	Serial.println(outStr);									// Send out the local copy.
	freeStr(&outStr);											// Release the local string memory.
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

	int			degInt;
	double		degDou;
	double		min;
	globalPos	localPos;
	bool			validPos;
	
	if (!inPos) return false;										// Sanity, pass in a NULL? Get a fail.
	validPos = false;													// Ain't valid yet.
	if (cmdParser.numParams()==2) {								// If we're looking at 2 params..
		degDou = atof(cmdParser.getNextParam());				// Grab the first param and decode it as a double.
		localPos.setLat(degDou);									// Drop it into our local position object.
		localPos.setLatQuad(cmdParser.getNextParam());		// Next param should be text version of quad. Drop it in as well.
		validPos = localPos.valid();								// Check to see if this is a sane latitude value.
	} else if (cmdParser.numParams()==3) {						// Else, if it has 3 parameters.. Different format here.
		degInt = atoi(cmdParser.getNextParam());				// Grab first param and decode as an integer degree value.
		min =  atof(cmdParser.getNextParam());					// Second param decoded as an double minute value.
		localPos.setLatValue(degInt,min);						// Stuff these two, degree & minute values into our local position object.
		localPos.setLatQuad(cmdParser.getNextParam());		// Third param should be text version of our quadrant.
		validPos = localPos.valid();								// Sanity check the position.
	}																		//
	if (validPos) {													// If this passed the sanity text..
		inPos->copyLat(&localPos);									// Write it to our destination mark.
		Serial.print(F("Latitude was set to : "));			// We basically tell the user everything was ok.
		Serial.println(inPos->showLatStr());					// Show the value.
		return true;													// Tell 'em it worked.
	} else {																// Else we give them a kick to do better next time.
		Serial.println(F("We're looking for either, latitude value & quadrant, (N/S kinda' thing)."));
		Serial.println(F("Or, latitude degree value, minute value and then quadrant."));
		Serial.println(F("I can't make what you typed match any of these."));
	}
	return false;
}


bool navII::doSetLon(globalPos* inPos) {

	int			degInt;
	double		degDou;
	double		min;
	globalPos	localPos;
	bool			validPos;
	
	if (!inPos) return false;										// Sanity, pass in a NULL? Get a fail.
	validPos = false;													// Ain't valid yet.
	if (cmdParser.numParams()==2) {								// If we're looking at 2 params..
		degDou = atof(cmdParser.getNextParam());				// Grab the first param and decode it as a double.
		localPos.setLon(degDou);									// Drop it into our local position object.
		localPos.setLonQuad(cmdParser.getNextParam());		// Next param should be text version of quad. Drop it in as well.
		validPos = localPos.valid();								// Check to see if this is a sane latitude value.
	} else if (cmdParser.numParams()==3) {						// Else, if it has 3 parameters.. Different format here.
		degInt = atoi(cmdParser.getNextParam());				// Grab first param and decode as an integer degree value.
		min =  atof(cmdParser.getNextParam());					// Second param decoded as an double minute value.
		localPos.setLonValue(degInt,min);						// Stuff these two, degree & minute values into our local position object.
		localPos.setLonQuad(cmdParser.getNextParam());		// Third param should be text version of our quadrant.
		validPos = localPos.valid();								// Sanity check the position.
	}																		//
	if (validPos) {													// If this passed the sanity text..
		inPos->copyLon(&localPos);									// Write it to our destination mark.
		Serial.print(F("Longitude was set to : "));				// We basically tell the user everything was ok.
		Serial.println(inPos->showLonStr());					// Show the value.
		return true;													// Success!
	} else {																// Else we give them a kick to do better next time.
		Serial.println(F("We're looking for, longitude value & quadrant, (E/W kinda' thing)."));
		Serial.println(F("Or, longitude degree value, minute value and then quadrant."));
		Serial.println(F("I can't make what you typed match any of these."));
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
		Serial.println(hoursOffUTC);									//
	} else if (cmdParser.numParams()==1) {							// If we got a param..
		UTCOffset = atoi(cmdParser.getNextParam());				// Decode it as an integer.
		if (UTCOffset>=-12&&UTCOffset<=12) {						// Sanity check.
			hoursOffUTC = UTCOffset;									// We can use this value.
			EEPROM.put(UTC_DELTA_E_LOC,hoursOffUTC);				// We save this byte in EEPROM for next time.
			Serial.print(F("Time offset from UTC set to : "));	// Tell 'em
			Serial.println(hoursOffUTC);								//
		}																		// 
	} else {																	// Really? Just tell em what we want.
		Serial.println(F("Looking for either no param. I'll show you the offset."));
		Serial.println(F("Or one param and I'll set that as offset for you."));
	}
}


void navII::doMCorrect(void) {
	
	float	value;
	
	if (cmdParser.numParams()==0) {																	// If we're looking at no params..
		Serial.print(F("Magnetic correction from true : "));										// We tell 'em..
		Serial.println(magCorrect);																	// What we have.
	} else if (cmdParser.numParams()==1) {															// If we got one param..
		value = atof(cmdParser.getNextParam());													// Decode it as a float.
		if (value<=180&&value>=-180) {																// Sanity check.
			magCorrect = value;																			// We can use this value.
			EEPROM.put(MAG_CORRECT_LOC,magCorrect);												// We save this value in EEPROM for next time.
			Serial.print(F("Magnetic correction  set to : "));										// Tell 'em
			Serial.println(magCorrect);																//
		} else {																								// Else wacky value?
			Serial.println(F("Sorry, looking for a value between -180 & 180 degrees."));	// Tell 'em no.
		}																										// 
	} else {																									// Else the wrong number of params.
		Serial.println(F("Looking for either no param. I'll show you the correction."));	// Tell 'em.
		Serial.println(F("Or one param and I'll set that as correction for you."));		// At length.
	}																											//
}


// Turn GPS raw text data out the serial port on or off. (PC or Mac)		
void navII::doSpew(void) {

	if (cmdParser.numParams()==0) {							// If we're looking at no params..
		ourGPS->setSpew(!(ourGPS->spew));					// We toggle spewing.
	} else if (cmdParser.numParams()==1) {					// If we're looking at one param..
		if (!strcmp(cmdParser.getNextParam(),"on")) {	// If we get "on"..
			ourGPS->setSpew(true);								// We force it to spew data.
		} else {														// Else, anything else..
			ourGPS->setSpew(false);								// We shut the spewing off.
		}																//
	}																	//
	if (!ourGPS->spew) Serial.print(F("Spewing off."));	// We ONLY say when it's off. Else it gets into spew stream.
}
		
		
		
		
