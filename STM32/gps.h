/*
/version	4.0
/author		Arnaud CADOT
*/

#ifndef GUARD_GPS
#define GUARD_GPS

#include "uart.h"
#include "pin.h"

/**
\brief NMEA0183-based GPS handler

This class handles a NMEAA0183-based GPS, using an UART port.
It provides an easy-to-use interface for a subset of the features provided by the protocol, focusing on
the most pertinent data (i.e. position, speed, bearing, horizontal accuracy, time, etc.)
**/
class GPS
{
	public:
		/**
			\brief Enumeration class for cardinal points (North, South, East and West)
		**/
		enum class CardinalPoint
		{
			N, ///< North
			S, ///< South
			E, ///< East
			W  ///< West
		};
		
		/**
			\brief Data structure for a GPS fix
		
			Data structure for a GPS fix. Stores position, speed, bearing, etc.
		**/
		struct GPSData
		{
			/**
				\brief Default constructor (invalid/null GPS data)
			**/
			GPSData();
			
			/**
				\brief Constructor
				\param latitude Latitude, in decimal degrees
				\param longitude Longitude, in decimal degrees
				\param speed Ground speed, in km/h
				\param heading Heading p/r True North, in degrees
				\param altitude Altitude, in meters
				\param horizontalAccuracy Radial horizontal accuracy, in meters
				\param timestamp Timestamp of the GPS fix, in Unix time
			**/
			GPSData(float latitude, float longitude, float speed = 0.0, float heading = 0.0, float altitude = 0.0,
				float horizontalAccuracy = 0.0, uint32_t timestamp = 0);
			
			/**
				\returns \c true if the horizontal accuracy, latitude and longitude are valid, \c false otherwise.
			**/
			bool isValid() const;
			
			/**
				\brief Compute the geodesic (i.e. 'as the crow flies') distance between two points
				\param other Other GPS point
				\returns Geodesic distance between \c this and \c other, in meters
			
				This method computes the geodesic distance between two sets of GPS coordinaters: \c this
				and \c other. The distance is computed in meters using the Haversine formula.
				It uses several trigonometric functions and single-precision floats operations. Consequently,
				special care should be taken on embeded devices with no FPU and/or little computationnal overhead.
			**/
			float distanceTo(const GPSData& other) const;
			
			float			latitude;				///< Latitude, in decimal degrees
			float			longitude;				///< Longitude, in decimal degrees
			float			horizontalAccuracy;		///< Horizontal (meters RMS)
			uint32_t		timestamp;				///< GPS time data (seconds since Epoch)
			float			speed;					///< Ground speed (km/h)
			float			heading;				///< True heading (degrees p/r true North)
			float			altitude;				///< Altitude (m)
		};
	
	public:
		/**
			\brief Constructor.
			\param serial UART to use to communicate with the GPS module
			\param resetPin The pin to use to reset the GPS (optionnal, use Pin() to ignore)
			\param powerPin The pin to use to control the GPS power supply (optionnal, use Pin() to ignore)
		**/
		GPS(Serial& serial, Pin resetPin = Pin(), Pin powerPin = Pin());
		
		/**
			\brief Initialize the pins and start the GPS module.
		**/
		bool init();
		
		/**
			\brief Update method.
			\returns \c true if the GPS just got a fix, \c false otherwise
			
			This update method should be called at regular interval. It allows the class to process the incoming
			data on the UART port, so data may be lost if the delay between two calls to update() is too great.
		**/
		bool update();
				
		/**
			\returns \c true if the object received at least one valid GPS fix from the module, \c false otherwise
			\remark Even if this method returns \c true, it does not mean the GPS data is pertinent, as it may be way
			out of date.
		**/
		bool hasFix() const;
	
		/**
			\brief Get the latest GPS fix
			\returns The latest GPS fix if avaiable, GPSData() otherwise
		
			Get the latest GPS fix. Note this fix is 'recomposed' using different NMEA messages, so different data may
			not be in sync (i.e. speed and position). In normal conditions of utilisation this is not an issue, but take
			care not to disable usefull NMEA messages when this object is running, or some GPS data may stay out of date.
			Also note that the 'timestamp' field of the GPSData structure is the time as given by the GPS receiver, NOT
			the time of reception of the GPS fix (which would make little sense, as the data is composite).
		**/
		GPSData getFix() const;
	
		/**
			\brief Clear the latest fix from memory.
		**/
		void clearFix();
	
		/**
			\brief Send an NMEA message to the GPS module
			\param data The data to be sent (ASCII encoded)
		
			Send a NMEA message to the GPS module, adding the heading '$' and the trailing checksum.
		**/
		void send(const ByteArray& data);
	
		
		/**
			\brief Reset the GPS module
			\returns \c true on error, \c false otherwise
		
			Reset the GPS module using the reset pin given in the constructor.
			If no reset pin were specified, this command does nothing and returns \c true.
		**/
		bool reset();
			
	private:
		Serial&		m_serial;
		ByteArray	m_buffer;
		GPSData		m_currentFix;
		Pin			m_resetPin;

		/**
			\brief Process NMEA GPGGA message
			\param d Raw ASCII NMEA message
		**/
		void processGPGGA(ByteArray& d);
	
		/**
			\brief Process NMEA GPRMC message
			\param d Raw ASCII NMEA message
		**/
		void processGPRMC(ByteArray& d);
	
		/**
			\brief Process NMEA GPVTG message
			\param d Raw ASCII NMEA message
		**/
		void processGPVTG(ByteArray& d);
	
		/**
			\brief Update submethod. Handle UART RX processing. 
		**/
		bool updateRx();
};

//extern GPS gps; ///< GPS singleton instance

#endif