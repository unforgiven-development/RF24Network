/*
 Copyright (C) 2011 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example: This is almost exactly the same as the Network_Ping example, but with use
 * of the integrated sleep mode and extended timeout periods.
 *
 * SLEEP_MODE: Sleep mode is set with the command radio.sleepNode(<seconds>, <interrupt pin>);
 * The node itself will sleep, with the radio in Standby-I mode, drawing 22uA compared to .9uA in powerdown mode.
 * Sleep mode uses the WatchDog Timer (WDT) to sleep in 1-second intervals, or is awoken by the radio interrupt pin going
 * high, which indicates that a payload has been received. This allows much more efficient power use among the entire network.
 * Max value is 65535 which sleeps for about 18 hours.
 * The node power use can be reduced further by disabling unnessessary systems via the Power Reduction Register(s) (PRR) and by putting
 * the radio into full sleep mode ( radio.powerDown() ), but it will not receive or send until powered up again.
 *
 * EXTENDED TIMEOUTS: This sketch utilizes newly introduced extended timeout periods to ensure reliability in transmission of payloads.
 * Users should be careful not to set too high a value on nodes that will receive data regularly, since payloads may be missed while
 * retrying failed payloads. The maximum retry period at (15,15) = 16 time periods * 250us retry delay * 15 retries = 60ms per payload.
 * Setting this value lower than the calculated retry period will have no effect.
 *
 * Using this sketch, each node will send a ping to every other node in the network every few seconds. 
 * The RF24Network library will route the message across the mesh to the correct node.
 *
 * This sketch is greatly complicated by the fact that at startup time, each
 * node (including the base) has no clue what nodes are alive.  So,
 * each node builds an array of nodes it has heard about.  The base
 * periodically sends out its whole known list of nodes to everyone.
 *
 * To see the underlying frames being relayed, compile RF24Network with
 * #define SERIAL_DEBUG.
 *
 * The logical node address of each node is set below, and are grouped in twos for demonstration.
 * Number 0 is the master node. Numbers 1-2 represent the 2nd layer in the tree (02,05).
 * Number 3 (012) is the first child of number 1 (02). Number 4 (015) is the first child of number 2.
 * Below that are children 5 (022) and 6 (025), and so on as shown below 
 * The tree below represents the possible network topology with the addresses defined lower down
 *
 *     Addresses/Topology                   Node Numbers  (To simplify address assignment in this demonstration)
 *             00        - Master Node         ( 0 )
 *           02  05      - 1st Level children ( 1,2 )
 *          12    15     - 2nd Level children ( 3,4 )
 *         22      25    - 3rd Level children ( 5,6 )
 *        32        35   - 4th Level          ( 7,8 )
 *                   45
 *
 * eg:
 * For node 4 (Address 015) to contact node 1 (address 02), it will send through node 2 (address 05) which relays the payload
 * through the master (00), which sends it through to node 1 (02). This seems complicated, however, node 4 (015) can be a very
 * long way away from node 1 (02), with node 2 (05) bridging the gap between it and the master node.
 *
 * To use the sketch, upload it to two or more units and set the NODE_ADDRESS below. If configuring only a few
 * units, set the addresses to 0,1,3,5... to configure all nodes as children to each other. If using many nodes,
 * it is easiest just to increment the NODE_ADDRESS by 1 as the sketch is uploaded to each device.
 */

#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include "printf.h"

/***********************************************************************
************* Set the Node Address *************************************
/***********************************************************************/
const uint16_t node_address_set[10] = { 00, 02, 05, 012, 015, 022, 025, 032, 035, 045 };
// 0 = Master, 1-2 = Top layer, 2-3 = Children of 1,2 respectively, 3-4 = Children of 2,3 respectively, etc...

uint8_t NODE_ADDRESS = 0;

/***********************************************************************/
/***********************************************************************/


RF24 radio(9,10);                              // CE & CS pins to use (Using 7,8 on Uno,Nano)

RF24Network network(radio); 

uint16_t this_node;                           // Our node address

const unsigned long interval = 99; // ms       // Delay manager to send pings regularly. Because of sleepNode(), this is largely irrelevant.
unsigned long last_time_sent;


const short max_active_nodes = 10;            // Array of nodes we are aware of
uint16_t active_nodes[max_active_nodes];
short num_active_nodes = 0;
short next_ping_node_index = 0;


bool send_T(uint16_t to);                      // Prototypes for functions to send & handle messages
bool send_N(uint16_t to);
void handle_T(RF24NetworkHeader& header);
void handle_N(RF24NetworkHeader& header);
void add_node(uint16_t node);


void setup(){
  
  Serial.begin(115200);
  printf_begin();
  printf_P(PSTR("\n\rRF24Network/examples/meshping/\n\r"));

  this_node = node_address_set[NODE_ADDRESS];            // Which node are we?
  
  SPI.begin();                                           // Bring up the RF network
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  network.begin(/*channel*/ 100, /*node address*/ this_node );

/**************************************************************************************************/ 
/*********************** Set the extended timeout period ******************************************/

  network.txTimeout = 500;          // In milliseconds, how long to retry each payload before giving up. This setting only affects this node.
                                    // Useful with the inclusion of sleep mode, but not really required if using interrupts
                            
/***************************************************************************************************/  
}

void loop(){
    
  network.update();                                      // Pump the network regularly

   while ( network.available() )  {                      // Is there anything ready for us?
     
    RF24NetworkHeader header;                            // If so, take a look at it
    network.peek(header);

    
      switch (header.type){                              // Dispatch the message to the correct handler.
        case 'T': handle_T(header); break;
        case 'N': handle_N(header); break;
        default:  printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"),header.type);
                  network.read(header,0,0);
                  break;
      };
    }

  
  unsigned long now = millis();                         // Send a ping to the next node every 'interval' ms
  if ( now - last_time_sent >= interval ){
    last_time_sent = now;


    uint16_t to = 00;                                   // Who should we send to? By default, send to base
    
    
    if ( num_active_nodes ){                            // Or if we have active nodes,
        to = active_nodes[next_ping_node_index++];      // Send to the next active node
        if ( next_ping_node_index > num_active_nodes ){ // Have we rolled over?
	    next_ping_node_index = 0;                   // Next time start at the beginning
	    to = 00;                                    // This time, send to node 00.
        }
    }

    bool ok;

    
    if ( this_node > 00 || to == 00 ){                    // Normal nodes send a 'T' ping
        ok = send_T(to);   
    }else{                                                // Base node sends the current active nodes out
        ok = send_N(to);
    }
    
    if (ok){                                              // Notify us of the result
        printf_P(PSTR("%lu: APP Send ok\n\r"),millis());
    }else{
        printf_P(PSTR("%lu: APP Send failed\n\r"),millis());
        last_time_sent -= 100;                            // Try sending at a different time next time
    }
  }

/*********************** Sleep mode enable *****************************************/
  delay(50);                          // Delay to allow completion of any serial printing
  if(!network.available()){           // If nothing to do at this moment
      network.sleepNode(2,0);         // Sleep this node for 2 seconds or a payload is received (interrupt 0 triggered), whichever comes first
   //Examples:
   // network.sleepNode( seconds, interrupt-pin );
   // network.sleepNode(0,0);         // Sleep this node forever or until a payload is received.
   // network.sleepNode(1,255);       // Sleep this node for 1 second. Do not wake up until then, even if a payload is received ( no interrupt )  
      
      //note: millis() will not increment while asleep
  }
}
/************************************************************************************/

/**
 * Send a 'T' message, the current time
 */
bool send_T(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'T' /*Time*/);
  
  // The 'T' message that we send is just a ulong, containing the time
  unsigned long message = millis();
  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending %lu to 0%o...\n\r"),millis(),message,to);
  return network.write(header,&message,sizeof(unsigned long));
}

/**
 * Send an 'N' message, the active node list
 */
bool send_N(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'N' /*Time*/);
  
  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending active nodes to 0%o...\n\r"),millis(),to);
  return network.write(header,active_nodes,sizeof(active_nodes));
}

/**
 * Handle a 'T' message
 * Add the node to the list of active nodes
 */
void handle_T(RF24NetworkHeader& header){

  unsigned long message;                                                                      // The 'T' message is just a ulong, containing the time
  network.read(header,&message,sizeof(unsigned long));
  printf_P(PSTR("%lu: APP Received %lu from 0%o\n\r"),millis(),message,header.from_node);


  if ( header.from_node != this_node || header.from_node > 00 )                                // If this message is from ourselves or the base, don't bother adding it to the active nodes.
    add_node(header.from_node);
}

/**
 * Handle an 'N' message, the active node list
 */
void handle_N(RF24NetworkHeader& header)
{
  static uint16_t incoming_nodes[max_active_nodes];

  network.read(header,&incoming_nodes,sizeof(incoming_nodes));
  printf_P(PSTR("%lu: APP Received nodes from 0%o\n\r"),millis(),header.from_node);

  int i = 0;
  while ( i < max_active_nodes && incoming_nodes[i] > 00 )
    add_node(incoming_nodes[i++]);
}

/**
 * Add a particular node to the current list of active nodes
 */
void add_node(uint16_t node){
  
  short i = num_active_nodes;                                    // Do we already know about this node?
  while (i--)  {
    if ( active_nodes[i] == node )
        break;
  }
  
  if ( i == -1 && num_active_nodes < max_active_nodes ){         // If not, add it to the table
      active_nodes[num_active_nodes++] = node; 
      printf_P(PSTR("%lu: APP Added 0%o to list of active nodes.\n\r"),millis(),node);
  }
}


