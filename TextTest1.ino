#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include "esp_adc_cal.h"
#include "bmp.h"
#include "ClosedCube_SHT31D.h"


// TFT Pins has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
// #define TFT_MOSI				19
// #define TFT_SCLK				18
// #define TFT_CS					5
// #define TFT_DC					16
// #define TFT_RST				23
// #define TFT_BL					4	// Display backlight control pin.


#define ADC_EN					14  // ADC_EN is the ADC detection enable port.
#define ADC_PIN				34


TFT_eSPI tft = TFT_eSPI( 135, 240 ); // Invoke custom library.
ClosedCube_SHT31D sht3xd;
char buff[512];
int vref = 1100;
int loopCount = 0;


// This function puts the ESP into shallow sleep, which saves power compared to the traditional delay().
void espDelay( int ms )
{
	esp_sleep_enable_timer_wakeup( ms * 1000 );
	esp_sleep_pd_config( ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON );
	esp_light_sleep_start();
}


String showVoltage()
{
	String voltage;
	static uint64_t timeStamp = 0;
	if( millis() - timeStamp > 1000 )
	{
		timeStamp = millis();
		uint16_t v = analogRead( ADC_PIN );
		float battery_voltage = (( float )v / 4095.0 ) * 2.0 * 3.3 * ( vref / 1000.0 );
		voltage = "Voltage :" + String( battery_voltage ) + "V";
		//Serial.println( voltage );
		//tft.fillScreen( TFT_BLACK );
		//tft.setTextDatum( MC_DATUM );
		//tft.drawString( voltage,	tft.width() / 2, tft.height() / 2 );
	}
	return voltage;
}


void wifi_scan()
{
	tft.setTextColor( TFT_GREEN, TFT_BLACK );
	tft.fillScreen( TFT_BLACK );
	tft.setTextDatum( MC_DATUM );
	tft.setTextSize( 1 );

	tft.drawString( "Scan Network", tft.width() / 2, tft.height() / 2 );

	WiFi.mode( WIFI_STA );
	WiFi.disconnect();
	delay( 100 );

	int16_t n = WiFi.scanNetworks();
	tft.fillScreen( TFT_BLACK );
	if( n == 0 )
	{
		tft.drawString( "no networks found", tft.width() / 2, tft.height() / 2 );
	}
	else
	{
		tft.setTextDatum( TL_DATUM );
		tft.setCursor( 0, 0 );
		Serial.printf( "Found %d net\n", n );
		for ( int i = 0; i < n; ++i )
		{
				sprintf( buff,
						"[%d]:%s(%d )",
						i + 1,
						WiFi.SSID( i ).c_str(),
						WiFi.RSSI( i ) );
				tft.println( buff );
		}
	}
	// WiFi.mode( WIFI_OFF );
}


void printResult( String text, SHT31D result, String voltage )
{
	if( result.error == SHT3XD_NO_ERROR )
	{
		Serial.print( text );
		Serial.print( ": T=" );
		Serial.print( result.t );
		Serial.print( "C, RH=" );
		Serial.print( result.rh );
		Serial.println( "%" );
		Serial.println( voltage );

				tft.fillScreen( TFT_BLACK );
		tft.setTextDatum( MC_DATUM );
		// drawString cannot print a float, so it needs to be inserted into a String.
		String tempBuffer;
		tempBuffer += F( "Temp : " );
		tempBuffer += String( result.t );
		// This display does not handle the degree symbol well.
		tempBuffer += F( "°C");
		// Draw this line 16 pixels above middle.
		tft.drawString( tempBuffer,  tft.width() / 2, tft.height() / 2 - 16 );
		String humidityBuffer;
		humidityBuffer += F( "Humidity : " );
		humidityBuffer += String( result.t );
		humidityBuffer += F( "%");
		// Draw this line centered vertically and horizontally.
		tft.drawString( humidityBuffer,  tft.width() / 2, tft.height() / 2 );
		// Draw this line 16 pixels below middle.
		tft.drawString( voltage,	tft.width() / 2, tft.height() / 2 + 16 );
	}
	else
	{
		Serial.print( text );
		Serial.print( ": [ERROR] Code #" );
		Serial.println( result.error );
	}
}


void setup()
{
	Wire.begin();
	Serial.begin( 115200 );
	Serial.println( "Start" );
	sht3xd.begin( 0x44 ); // I2C address: 0x44 or 0x45

	Serial.print( "Serial #" );
	Serial.println( sht3xd.readSerialNumber() );
	if( sht3xd.periodicStart( SHT3XD_REPEATABILITY_HIGH, SHT3XD_FREQUENCY_10HZ ) != SHT3XD_NO_ERROR )
		Serial.println( "[ERROR] Cannot start periodic mode" );

	/*
	ADC_EN is the ADC detection enable port.
	If the USB port is used for power supply, it is turned on by default.
	If it is powered by battery, it needs to be set to high level.
	*/
	pinMode( ADC_EN, OUTPUT );
	digitalWrite( ADC_EN, HIGH );

	tft.init();
	tft.setRotation( 1 );
	tft.fillScreen( TFT_BLACK );
	tft.setTextSize( 2 );
	tft.setTextColor( TFT_GREEN );
	tft.setCursor( 0, 0 );
	tft.setTextDatum( MC_DATUM );
	tft.setTextSize( 1 );

	/*
	// TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
	if( TFT_BL > 0 )
	{
		// Set backlight pin to output mode.
		pinMode( TFT_BL, OUTPUT );
		// Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
		digitalWrite( TFT_BL, TFT_BACKLIGHT_ON );
	}
	*/

	tft.setSwapBytes( true );
	tft.pushImage( 0, 0,	240, 135, ttgo );
	espDelay( 5000 );


	tft.setRotation( 0 );
	tft.fillScreen( TFT_RED );
	espDelay( 1000 );
	tft.fillScreen( TFT_BLUE );
	espDelay( 1000 );
	tft.fillScreen( TFT_GREEN );
	espDelay( 1000 );


	tft.fillScreen( TFT_BLACK );
	tft.setTextDatum( MC_DATUM );


	tft.drawString( "LeftButton:", tft.width() / 2, tft.height() / 2 - 16 );
	tft.drawString( "[WiFi Scan]", tft.width() / 2, tft.height() / 2 );
	tft.drawString( "RightButton:", tft.width() / 2, tft.height() / 2 + 16 );
	tft.drawString( "[Voltage Monitor]", tft.width() / 2, tft.height() / 2 + 32 );
	tft.drawString( "RightButtonLongPress:", tft.width() / 2, tft.height() / 2 + 48 );
	tft.drawString( "[Deep Sleep]", tft.width() / 2, tft.height() / 2 + 64 );
	tft.setTextDatum( TL_DATUM );
}


void loop()
{
	loopCount++;

	String voltage = showVoltage();

	printResult( "Periodic Mode", sht3xd.periodicFetchData(), voltage );
	espDelay( 5000 );

	//wifi_scan();
	//delay( 5000 );
}