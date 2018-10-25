//Copyright 2018 George Rennie
#ifndef MIDIMESSAGE_H
#define MIDIMESSAGE_H

#include "MIDIDefs.h"

typedef struct {
  /*! The MI
  DI channel on which the message was recieved.
    \n Value goes from 0 to 15.
    16 represenents an unknown channel
    */
  Channel channel;

  /*! The type of the message
    (see the MIDIType enum for types reference)
    */
  MIDIType type;

  /*! The first data byte.
    \n Value goes from 0 to 127.
    */
  DataByte data1;

  /*! The second data byte.
    If the message is only 2 bytes long, this one is null.
    \n Value goes from 0 to 127.
    */
  DataByte data2;

  /*! This boolean indicates if the message is valid or not.
    There is no channel consideration here,
    validity means the message respects the MIDI norm.
    */
  byte valid;  
} MIDIMessage;
#endif