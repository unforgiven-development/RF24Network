/*
 * RF24Network -- examples_RPi/helloworld_rx.cpp
 *
 * (c) 2016 Gerad Munsch <gmunsch@unforgivendevelopment.com>
 * (c) 2014 TMRh20
 *
 * One of the simplest examples of using the RF24Network library
 */

/**
 * One of the simplest examples of using the RF24Network library
 *
 * RECEIVER NODE:
 * - Listens for messages from the transmitter and prints them out.
 */

#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <time.h>


/**
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

// Address of our node in Octal format
const uint16_t this_node = 00;

// Address of the other node in Octal format (01,021, etc)
const uint16_t other_node = 01;

/*
 * Interval (in milliseconds) in which to send "hello world" to the other unit.
 */
const unsigned long interval = 2000;

unsigned long last_sent;            // When did we last send?
unsigned long packets_sent;         // How many have we sent already

/*
 * Structure of the payload
 */
struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

int main (int argc, char** argv) {
	/*
	 * For further information, refer to "RF24.h" header file, which is contained
	 * in the primary "RF24" project repository, or the official datasheet and/or
	 * related documentation for the NRF24L01(+) from Nordic Semiconductor.
	 */

	radio.begin();

	delay(5);
	network.begin(/*channel*/ 90, /*node address*/ this_node);
	radio.printDetails();
	
	while (1) {
    network.update();
    /* Check if we have any data waiting for us */
    while (network.available()) {
      /* If so, retrieve the data, and print it out */
      RF24NetworkHeader header;
      payload_t payload;
      network.read(header,&payload,sizeof(payload));

      printf("Received payload # %lu at %lu \n",payload.counter,payload.ms);
    }

    //sleep(2);
    delay(2000);
    //fclose(pFile);
  }
  return 0;
}