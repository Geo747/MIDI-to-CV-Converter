/*Copyright 2019 George Rennie

  Things to note:
  When no note is playing a value of 128 is returned as both note and velocity.
  Velocities and Notes must be in the range 0-127 as with MIDI. If more than
  one note on message on the same note is sent before receiving a note off
  message then the previous note ons will be forgotten from the stack and the
  note will be removed on the first corresponding note off message reveived
*/

#include "PolyToMono.h"

/*Data array to keep all the addresses and stuff with the first dimension 
  used to determine channel. Each index in the second dimension represents
  one MIDI note with the last index being a no note indicator. The third 
  dimension contains preceding note to that 2nd dimension index, then 
  following note and then that notes velocity
*/
static uint8_t data[MIDI_CHANNELS][129][3];
static uint8_t saveNote[MIDI_CHANNELS];

static uint8_t mAccuracy[] = {1, 1};

void polyToMonoSetup(void) {
  for (uint8_t i = 0; i < MIDI_CHANNELS; i++) {
    //Init all places in the data array to 128 (i.e. the no note value)
    for (uint8_t j = 0; j < 129; j++) {
      for (uint8_t k = 0; k < 3; k++) {
        data[i][j][k] = 128;
      }
    }

    //Init all savenote places to no note value
    saveNote[i] = 128;
  }
}

void polyToMonoSetAccuracy(uint8_t accuracy, uint8_t channel) {
  if (CHANNNEL_USED(channel)) {
    mAccuracy[channel - BOTTOM_MIDI_CHANNEL] = accuracy;
  }
}

//Returns true if the note is in the bounds set by the current accuracy
static inline uint8_t noteInRange(uint8_t note, uint8_t channel) {
  return(note >= LOWEST_NOTE[mAccuracy[channel - BOTTOM_MIDI_CHANNEL]] && 
    note <= HIGHEST_NOTE[mAccuracy[channel - BOTTOM_MIDI_CHANNEL]]);
}

void polyToMonoNoteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
  if (!noteInRange(note, channel)) { return; }

  uint8_t chanIndex = channel - BOTTOM_MIDI_CHANNEL;

  /*If the note doesnt have a velocity of 128 (i.e. it is already 
    in the stack), and it isnt the last place on the stack
  */
  if ((data[chanIndex][note][2] != 128) && (note != saveNote[chanIndex])) {

    /*Patch the references for the two notes either side of the note's
      original place as it is now being moved to the front of the stack
    */
		uint8_t prevNote = data[chanIndex][note][0];
		uint8_t nextNote = data[chanIndex][note][1];
		data[chanIndex][prevNote][1] = nextNote;
		data[chanIndex][nextNote][0] = prevNote;
	}

  //Save the velocity of the note
	data[chanIndex][note][2] = velocity;
  //If its a repeat of current held note do nothing more
	if (note == saveNote[chanIndex]) { return; }
  //The next note for the previous note is the new note
	data[chanIndex][saveNote[chanIndex]][1] = note;
   //The last note for the new note is the previous note
	data[chanIndex][note][0] = saveNote[chanIndex];
	saveNote[chanIndex] = note;
}

void polyToMonoNoteOff(uint8_t note, uint8_t channel) {
  if (!noteInRange(note, channel)) { return; }

  uint8_t chanIndex = channel - BOTTOM_MIDI_CHANNEL;
  
  //Velocity off for note
  data[chanIndex][note][2] = 128;
  
  //If it is last note on stack move pointer back one.
	if (note == saveNote[chanIndex]) {
		saveNote[chanIndex] = data[chanIndex][note][0]; //Update savechannel

    /*This line below was to fix a bug which I can no longer 
      reproduce for unknown reason.
      The bug was originally created by a sequency of
      F2on, E2on, F2off, E2off, E2on, F2on, F2off, E2off.
      This caused the last note to not turn off properly and 
      the data array to be corrupted
    */
   
    //data[channel][note][0] = 128; //Set previous note to 0 
	}
	else {
    //Patch references for preceding and following notes
    //So they reference each other and not the note that is being removed
		uint8_t prevNote = data[chanIndex][note][0]; 
		uint8_t nextNote = data[chanIndex][note][1]; 
		data[chanIndex][prevNote][1] = nextNote;
		data[chanIndex][nextNote][0] = prevNote;
	}
}

/*Consider changing this to more time 
  consuming iterated fill that sets everything to 128?
  Meaning there cant be bugs where while loop doesnt escape
*/
void polyToMonoAllNotesOff(uint8_t channel) {
  uint8_t chanIndex = channel - BOTTOM_MIDI_CHANNEL;

  while (saveNote[chanIndex] != 128) {
    //Set last note on stack velocity  to 0
    data[chanIndex][saveNote[chanIndex]][2] = 128;

    /*As the velocity has been set to 128 (off) the values left in 
      its references are irrelevant
    */
		saveNote[chanIndex] = data[chanIndex][saveNote[chanIndex]][0];
  }
}

uint8_t polyToMonoCurrentNote(uint8_t channel) {
  return saveNote[channel - BOTTOM_MIDI_CHANNEL];
}

uint8_t polyToMonoCurrentVelocity(uint8_t channel) {
  uint8_t chanIndex = channel - BOTTOM_MIDI_CHANNEL;
  return data[chanIndex][saveNote[chanIndex]][2];
}

uint8_t polyToMonoIsNoteOn(uint8_t channel) {  
  if (saveNote[channel - BOTTOM_MIDI_CHANNEL] == 128) { return 0; }
  return 1;
}