(note: this is a scratchpad to organise my thoughs on how to organise the object control flows)

Components
==========

Sources, sinks and controller
-----------------------------

Bluetooth audio (master source, slave source controller)
Streaming/File audio (slave source, URL controller)
Physical controls (controller)
Alarm clock (controller)
  * Question: can the BT Audio act as a slave source? (from experience yes - however the ESP sdk does not support that yet)

Plumbing components
-------------------
Source multiplexer:
 - ensures only one source can access the sink
Super controller:
 - centralises commands from controllers and dispatches to the sources as appropriate
 - ensures sources and dependencies are set-up and torn-down as needed

Sound multiplexing
==================

All sources implement the Source class:
- Bluetooth audio directly
- MP3, AAC and FLAC through the Decoder class

All sources must signal that media reading ended by (1) releasing the Sink and (2) signalling the command multiplexer that playing ended.

Command multiplexing
====================
This is a two way lane:
 1. Sources must signal the multiplexer when:
   1. Playing started
   2. Playing is in progress, for example every few sample blocks
   3. Underun occurs
   4. Playing stopped

 2. Sources must
   1. Have a play method; starting play
   2. Have a stop method; stopping play
   3. Optionally have a seek method



