#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"


SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play

WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

int pirPin = 8;

//#define DEBOUNCE 100  // button debouncer

// this handy function will return the number of bytes currently free in RAM, great for debugging!   
int freeRam(void)
{
  extern int  __bss_end; 
  extern int  *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
} 

void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

void setup() {
  // set up serial port

  pinMode(8, INPUT);
    digitalWrite(8, LOW);

  Serial.begin(9600);
  putstring_nl("Wookiebox");

  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!

  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  // pin13 LED
  pinMode(13, OUTPUT);



  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }

  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);

  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }

  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?

  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }

  // Whew! We got past the tough parts.
  putstring_nl("Ready!");
}

void loop() {
  //putstring(".");            // uncomment this to see if the loop isnt running
  if(digitalRead(pirPin) == HIGH){
    putstring_nl("motion!");
    int roarnumber = random(3); //and edit this too.  One higher than the number of roars.
    switch (roarnumber)
    {
    case 0:
      putstring_nl("0.wav");
      playcomplete("0.WAV");    //edit these
      break;                    //when you
    case 1:                     //get the 
      putstring_nl("1.wav");    //sound
      playcomplete("1.WAV");    //files.
      break;
    case 2:
    putstring_nl("2.wav");
      playcomplete("2.WAV");
      break;
  /*  case 3:
      playcomplete("3.WAV");
      break;
    case 4:
      playcomplete("4.WAV");    //unneeded for now
      break;
    case 5:
      playcomplete("5.WAV");
      break;
    case 6:
      playcomplete("6.WAV");
      break;
    case 7:
      playcomplete("7.WAV");
      break;
    case 8:
      playcomplete("8.WAV");
      break;
    case 9:
      playcomplete("9.WAV");
      break;
    case 10:
      playcomplete("p.WAV");     // I modified the standard demo sketch.  Don't need this.
    */}
    delay(10000);
  }

}


// Plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
    // do nothing while its playing
  }
  // now its done playing
}

void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); 
    Serial.print(name); 
    return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); 
    return;
  }

  // ok time to play! start playback
  wave.play();
}

