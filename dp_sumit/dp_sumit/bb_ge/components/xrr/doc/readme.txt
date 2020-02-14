XRR Scheduling:

When the REX receives packet from the LEX or the device, XICS will interrupt XRR if the packet arrives in the one of the following queues:
- REX_SQ_SCH_PERIODIC
- REX_SQ_CPU_DEV_RESP
- REX_SQ_SCH_MSA
- REX_SQ_SCH_ASYN_INB

XRR will read the header of the packet by copying it from the cache into the RdFrmHdr registers.

Based on the REXSCH algorithm, the REX has issue one of the following operations:
- Write: Write a new packet header into the SchFrmHdr register, and schedule the packet upstream or downstream.
- Update: Copy the same packet header from the RdFrmHdr register to the SchFrmHdr. After this, either send it to a static queue for scheduling, or save the packet to a dynamic queue (to be sent later).
- Update with new action: Copy the same packet header from the RdFrmHdr register to the SchFrmHdr, and replace the action field. After this, either send it to a static queue for scheduling, or save the packet to a dynamic queue (to be sent later).
- Update with new response: Copy the same packet header from the RdFrmHdr register to the SchFrmHdr, and replace the response field. After this, either send it to a static queue for scheduling, or save the packet to a dynamic queue (to be sent later).
- Update with new modifier: Copy the same packet header from the RdFrmHdr register to the SchFrmHdr, and replace the modifier field. After this, either send it to a static queue for scheduling, or save the packet to a dynamic queue (to be sent later).
- Copy: Copy a packet header from a source dynamic queue and send it to a static queue for scheduling. This operation is used after one of the Update operations has saved the packet to a dynamic queue.

