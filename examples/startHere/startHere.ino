//************************************************************
// this is a simple example that uses the easyMesh library
//
// 1. blinks led once for every node on the mesh
// 2. blink cycle repeats every BLINK_PERIOD
// 3. sends a silly message to every node on the mesh at a random time betweew 1 and 5 seconds
// 4. prints anything it recieves to Serial.print
// 
//
//************************************************************
#include <painlessMesh.h>

// some gpio pin that is connected to an LED... 
// on my rig, this is 5, change to the right number of your LED.
#define   LED             5       // GPIO number of connected LED

#define   BLINK_PERIOD    1000000 // microseconds until cycle repeat
#define   BLINK_DURATION  100000  // microseconds LED is on for

#define   MESH_SSID       "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

painlessMesh  mesh;

uint32_t sendMessageTime = 0;

void setup() {
  Serial.begin(115200);
    
  pinMode( LED, OUTPUT );

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_SSID, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  randomSeed( analogRead( A0 ) );  
}

void loop() {
  mesh.update();

  // run the blinky
  bool  onFlag = false;
  uint32_t cycleTime = mesh.getNodeTime() % BLINK_PERIOD;
  for ( uint8_t i = 0; i < ( mesh.connectionCount() + 1); i++ ) {
    uint32_t onTime = BLINK_DURATION * i * 2;    

    if ( cycleTime > onTime && cycleTime < onTime + BLINK_DURATION )
      onFlag = true;
  }
  digitalWrite( LED, onFlag );

  // get next random time for send message
  if ( sendMessageTime == 0 ) {
    sendMessageTime = mesh.getNodeTime() + random( 1000000, 5000000 );
  }

  // if the time is ripe, send everyone a message!
  if ( sendMessageTime != 0 && sendMessageTime < mesh.getNodeTime() ){
    String msg = "Hello from node ";
    msg += mesh.getNodeId();
    mesh.sendBroadcast( msg );
    sendMessageTime = 0;
  }
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}
