/************************************************************************************
 *   router.h : contains structures needed by the event-trapping setup routines on
 *              both the MDSP & SDSPs; these fields are input in the eventTrapSetup
 *      primitive. Some control registers on the router, while others inform the
 *      SDSPs what tasks are going to handle the incoming data.
 *
 *      Trap Parameters:
 *
 *          config:  Router register which configures the trap-- which type of events
 *                   will it trap? Can be any of: TRAP_CFG_ IDLE, ATLAS_EVT, TIM_EVT,
 *                   ROD_EVT or SLINK_EVT. If SLINK_EVT, the router will trap all
 *                   event types.
 *
 *          match:   Router register. The appropriate field in the event header is
 *                   compared against this to determine if the event will be trapped
 *                   or not. For example match= 21, config= ATLAS_EVT will catch only
 *                   events with ATLAS ID 21.
 *
 *          exclusionFlag: Status to be determined.
 *
 *          function: Indicates which tasks on the SDSP(s) will process events coming
 *                    from this trap. This is a bitfield and thus is additive. For
 *                    example, to set up both histogramming & event copying (trapping)
 *                    set function= TRAP_FXN_HISTOGRAM +TRAP_FXN_TRAP.
 *
 *          modulus & remainder:  Router registers which determine the frequency of
 *                                the trap. Events are trapped if the # of events
 *                   meeting the cfg & match criterion defined above modulo the
 *                   modulus register equals the remainder register. For example,
 *                   (mod., rem.)= (5,3) will trap the third event out of every 5
 *                   events. If the modulus = 1, the router will trap every matching
 *                   event, and if modulus = 0, it will trap a single event and then
 *                   the trap will reset itself.

 ************************************************************************************/
#ifndef ROUTER_STRUCT
#define ROUTER_STRUCT

typedef struct RouterTrapParams {
	UINT8 config, match, exclusionFlag, function;
	UINT8 modulus, remainder, unused[2];
} RouterTrapParams;

#define  TRAP_CFG_IDLE        0
#define  TRAP_CFG_ATLAS_EVT   1
#define  TRAP_CFG_TIM_EVT     2
#define  TRAP_CFG_ROD_EVT     3
#define  TRAP_CFG_SLINK_EVT   4

#define  TRAP_FXN_HISTOGRAM   1
#define  TRAP_FXN_TRAP        2
#define  TRAP_FXN_OCCUPANCY   4
#define  TRAP_FXN_ERRORCNT    8
#define  TRAP_FXN_RESYNCH     16

#endif
