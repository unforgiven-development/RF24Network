/*
 * RF24Network -- examples_RPi/rx-test.cpp
 *
 * (c) 2016 Gerad Munsch <gmunsch@unforgivendevelopment.com>
 * (c) ????
 *
 * An example of receiving data with RF24Network
 */

/* === INCLUDES === */
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
//#include <rrd.h>
#include <time.h>

/*
 * g++ -L/usr/lib main.cc -I/usr/include -o main -lrrd
 */

//using namespace std;

/* -------------------------------- SPI Init -------------------------------- */

// CE Pin, CSN Pin, SPI Speed

// Setup for GPIO 22 CE and GPIO 25 CSN with SPI Speed @ 1Mhz
//RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_18, BCM2835_SPI_SPEED_1MHZ);

// Setup for GPIO 22 CE and CE0 CSN with SPI Speed @ 4Mhz
//RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_4MHZ);

// Setup for GPIO 22 CE and CE1 CSN with SPI Speed @ 8Mhz
RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);

RF24Network network(radio);

// Address of our node
const uint16_t this_node = 00;

// Address of the other node
const uint16_t other_node = 01;

// Structure of our payload
struct payload_power_t {
  unsigned long nodeId;
  float power;
  float current;
};

struct payload_weather_t {
  unsigned long nodeId;
  float temperature;
  float humidity;
  unsigned long lux;
};

int main (int argc, char** argv) {
	/*
	 * For further information, refer to "RF24.h" header file, which is contained
	 * in the primary "RF24" project repository, or the official datasheet and/or
	 * related documentation for the NRF24L01(+) from Nordic Semiconductor.
	 */

  //char filenameTemp[] = "/root/weather/weather_sensor.rrd";
  //char filenameCurrent[] = "/home/pi/temperature.txt";
  //char command[] = "update";
  //char values[100];

  char temperatureFile[] = "temperature.txt";

  radio.begin();
  //radio.setDataRate(RF24_250KBPS);
  radio.setRetries(7, 7);

  delay(5);
  //network.begin(RF24_CHANNEL, RF24_ADDRESS);
  network.begin(100, this_node);
  radio.printDetails();

	while (1) {
    //FILE * pFile;
    //pFile = fopen ("/root/temp-exterior.txt","a");
    network.update();
    /* Check if we have any data waiting for us */
    while (network.available()) {
      /* If so, retrieve the data, and print it out */
      RF24NetworkHeader header;
      network.peek(header);

      if (header.type == 'T') {
        float message;
        network.read(header, &message, sizeof(float));
        printf("RCVD: %.3f \n\r", message);
				FILE *myFile;
				myFile = fopen(temperatureFile, "a+"); //Append, new EOF

				if (!myFile) {
          printf("File not opened\n\r");
				} else {
					//fwrite(msg, sizeof(float), 1, myFile);
					fprintf(myFile, "%.3f\n\r", message);
					//putc('\n', myFile);
					//putc('\r', myFile);
					fclose(myFile);
				}
			}
/*******************************************************************************
			// sensor de temperatura y humedad
			if (header.from_node == 1) {
				payload_weather_t payload;
				network.read(header, &payload, sizeof(payload));
				rrd_argv[0] = command;
				rrd_argv[1] = filenameTemp;
				rrd_argv[3] = NULL;
				rrd_argc = 3;
				printf("N:%.2f:%.2f:%lu\n", payload.temperature, payload.humidity, payload.lux);
				sprintf(values, "N:%.2f:%.2f:%lu", payload.temperature, payload.humidity, payload.lux);
				rrd_argv[2] = values;
				rrd_update(rrd_argc, rrd_argv);
			}

			if (header.from_node == 2) {
				payload_power_t payload;
				network.read(header, &payload, sizeof(payload));
				rrd_argv[0] = command; 
				rrd_argv[1] = filenameCurrent; 
				rrd_argv[3] = NULL;
				rrd_argc = 3;
				printf("N:%.2f:%.2f\n", payload.power, payload.current);
				sprintf(values, "N:%.2f:%.2f", payload.power, payload.current);
				rrd_argv[2] = values;
				rrd_update(rrd_argc, rrd_argv);
			}

			time_t timer;
			char timeBuffer[25];
			struct tm* tm_info;
			time(&timer);
			tm_info = localtime(&timer);
			strftime(timeBuffer, 25, "%Y/%m/%d %H:%M:%S", tm_info);
			fprintf(pFile, "%s;%lu;%.2f;%.2f\n", timeBuffer, payload.nodeId, payload.data1, payload.data2);
*******************************************************************************/
		}
		delay(2000);
		//fclose(pFile);
	}

	return 0;
}