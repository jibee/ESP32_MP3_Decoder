(note: this is a scratchpad to organise my thoughs on how to organise the object control flows)

Components
==========

Sources, sinks and controller
-----------------------------

Bluetooth audio (master source, slave source controller)
Streaming/File audio (slave source, URL controller)
Physical controls (controller)
Alarm clock (controller)
  * Question: can the BT Audio act as a slave source? (from experience yes - how can we do that?)

Plumbing components
-------------------
Source multiplexer:
 - ensures only one source can access the sink
Super controller:
 - centralises commands from controllers and dispatches to the sources as appropriate
 - ensures sources and dependencies are set-up and torn-down as needed

Sound multiplexing
==================


Command multiplexing
====================


