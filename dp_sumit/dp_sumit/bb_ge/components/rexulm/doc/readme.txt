REXULM COMPONENT
================

This is the Rex ULM component!  The component responsible for maintaining the device's USB state.

OVERVIEW
--------
The Rex ULM tries get the Rex to mimic what the upstream is doing on the Lex.  For the most part events such as suspends and resumes are taken from the Lex ULM message and applied directly to the ULM driver, however it isn't always possible to process events right away.

The following cases differ from a pure mimic of the upstream port
 * Device speed prefetching.  When a device is first plugged in the Rexulm will prefetch the speed, and then inform the Lex of a device connect at the specified speed.  The LexULM then knows what speed to negiotate with the upstream port.
 * Event extensions.  As the Rex ULM tries to keep in sync with the upstream port, it will extend resets and resumes until it gets notification that the upstream port has finished its operation.  The Rex ULM will then get a notification from the ULM driver after the operation is complete on the Rex
 * Delayed events.  If the event at the Lex ULM can not be done right away at the Rex, then the event is delayed.  This is done by taking note of the Lex ULM state, and when the current operation is complete the Rex ULM will start the event to get the Rex to follow the upstream port operation
  * Exception.  We won't wait for a suspend done interrupt, when a bus reset message is received from Lex.  Instead the bus reset is kicked off right away
 * Upstream port disconnects.  When the upstream port disconnects, there may or may not be a need to mimic the behaviour on the Rex.  If the device has been enumerated, there is a possiblity that if it is a hub, the downstream ports could be powered, or it could be configured to have remote wakeup enabled.  In this case the RexULM tracks whether or not a device could have been enumerated, and if it has the device will go through a re-connection, otherwise the device is just placed directly into suspend without any need to re-prefetch the speed.
 * When a message is received from upstream that upstream is going into operating state, the RexULM will continue to extend the current operation, until either SOF packets are received from upstream, or a new operation is started.  This helps remove race condition bugs where upstream never sends any packets, and the upstream goes into suspend after 3ms.  The RexULM will never send traffic until it receives the suspend message, but may decide to send some traffic (SOF packets) to the device before the suspend, to work around buggy devices.  This could lead to a race condition as the downstream device will go into suspend, just the upstream message is received, and the RexULM starts to send SOF packets out.


A Note On the Lex ULM/Rex ULM interaction
-----------------------------------------
The Lex ULM software doesn't track any state, and only operates on events.  This is apparent in the upstream port disconnects.  On this event the Lex will disconnect from the host, and inform the Rex.  The Lex will not keep knowledge of the device from Rex, and will wait for the Rex to inform the Lex of a device connection at speed X, before the LexULM will enable the ULM RTL connection to the upstream port.

 
INPUT
-----
The Rex ULM takes input from
1) USB interrupts via the ULM driver component
2) The LinkMgr for messages when the link goes up or down
3) Messages from the LexULM (or VHub) via the project message delivery
4) 100ms debounce timer via LEON timers, configured by connect ISR
5) Connect/Disconnect timer to allow for VBus to drain, so downstream devices can properly reset themselves
6) Idle task to check for SOF packets from the Lex.  This is to indicate that the upstream is really operating, and not just going into a suspend.
7) Idle task before suspending to ensure SOF's are transmitted before a suspend.  Works around buggy devices.

Each entry point in the software for each of these inputs can output a time marker to assist in debugging.  To enable this feature ensure REXULM_USE_TIME_MARKERS is defined in options.h, and note that the debug level is currently at minor event, so it the logging level will need to be adjusted at runtime to use this feature.


OUTPUT
------
The Rex ULM operates on the following
1) the ULM driver to set the USB state of the device
2) the XCSR message delivery system to send USB messages to the Lex
3) starting/stopping the connect debounce timer
4) enable/disable of the Rex Scheduler
5) the XRex SOF output forcing


STATE
-----
The Rex ULM contains the global structure variable "struct rexulmState rex".  This global variable tracks all state in the Rex ULM.  This structure uses no accessor functions and is accessible directly by all C files in the Rex ULM component.


DESIGN
------
The Rex ULM is designed as an event driven system.  Every event follows the same path
1) check input & current state
2) based on input & current state take action including updating the current state
4) call _REXULM_UpdateSystemState
5) return void.  The callee of Rex ULM events is not responsible for any knowledge of the Rex ULM message handling

_REXULM_UpdateSystemState updates system state such as the LEDs and cleans up bogus conditions such as having the upstream port connected when the device is not connected (result of Lex/Rex messages passing on the wire)

The connect/disconnect timer is the one exception to this, as it doesn't actually update any state at all

PREFETCHING SPEED
-----------------
When a device connects the Rex ULM will prefetch the speed before imforming the Lex ULM of the new device.  This requires at least 100ms of debouncing time (from the USB Spec) of inactivity before the Rex ULM issues a bus reset.  After the timer has expired the RexULM issues a bus reset, waits for the ULM driver to report a negiotated speed, suspends the device, and then informs the Lex of the new device at the specified speed.


REX SCHEDULER STARTING/STOPPING
-------------------------------
The Rex Scheduler only needs to run while the device is in operating mode, however the Rex Scheduler should be started early to allow the hardwire time to synchronize its SOF timing with the host SOFs.

The behaviour of the Rex Scheduler is different between LG1 and GE, so the following Rex Scheduler functions are called to let the Rex Scheduler implementation decide what to do at each point

void REXSCH_LexResumeDone(void)
void REXSCH_RexResumeDone(void)

boolT REXSCH_LexBusResetDone(void) // Return TRUE to stop extending the operation
boolT REXSCH_RexBusResetNeg(UsbSpeedT) // Return TRUE to stop extending the operation
void REXSCH_RexBusResetDone(void)


The Rex Scheduler stopping conditions are:
1) Any event that takes the rex.downstream out of operating state.


SOURCE HIERACHY
---------------
// Initialization
init.c

// Event handlers
lexmsgs.c       // Handles messages from the Lex
lexrexlink.c    // Handles the Lex/Rex link up/down messages
usbevent.c      // Handles the USB messages from the ULM
timer.c         // Handles the debouce timer from a connect & disconnect/connect timer
idle.c          // Handles to checking for SOF packets from the Lex

// Helper C files
state.c         // Contains global rex structure, and state related functions
utils.c         // Various helper functions for common operations

// local header files
rexulm_loc.h    // Local header file exposing all functionality within the Rex ULM component
rexulm_cmd.h    // ICMD file for the Rex ULM component
rexulm_log.h    // ILOG file for the Rex ULM component

