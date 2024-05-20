.. Copyright (c) 2022 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
..
.. SPDX-License-Identifier: GPL-2.0-only

Overview
========
The extension to support NR V2X involved modifications at
all layers of the protocol stack, from the ``NAS`` and down to the ``PHY`` layer.
One of the most important changes towards the implementation of NR V2X communications,
compared to the typical cellular communications available in 5G-LENA simulator,
is the introduction of sidelink, i.e., direct vehicle-to-vehicle communications.
For that, the bearer establishment and ``RRC`` layer have been fully updated
according to NR V2X ``RRC`` specification in TS 38.331. Also, ``MAC`` and ``PHY``
layers have been redesigned to implement NR V2X Mode 2 procedures using sensing-based
Semi Persistent Scheduling (SPS), as described in Section III-B of [cttc-nr-v2x]_,
according to TS 38.321 for the MAC layer, and TS 38.211 and TS 38.212 for the PHY layer. 
Non-persistent grants (dynamic grants) are also supported by the scheduler.
In the simulator, we focus on NR V2X Mode 2 for out-of-coverage
scenarios, with broadcast, groupcast, and unicast communications, and also support both
blind retransmissions (i.e., without sidelink HARQ feedback) and sidelink HARQ-based
retransmissions. In
Table :ref:`tab-nr-v2x-models`, we detail the features and functionalities that
are available in the developed NR V2X system-level simulator.


.. only:: latex

    .. raw:: latex
      
        \clearpage


.. _tab-nr-v2x-models:

.. table:: NR V2X models
   :class: longtable

   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   |                                |                                                  **NR V2X**                                                                            |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Frame structure                | TDD NR-compliant frame structure with slots and OFDM symbols                                                                           |
   |                                | of numerology-dependent length [TS38300]_:                                                                                             |
   |                                |                                                                                                                                        |
   |                                | - Frame: 10 ms, subframe: 1 ms.                                                                                                        |
   |                                | - Each subframe has :math:`2^{\mu}` slots (associated to :math:`15\times2^{\mu}` kHz SCS).                                             |
   |                                | - Numerologies :math:`{\mu}` = 0, 1, 2, 3, 4 are supported.                                                                            |
   |                                | - Each slot is composed of 14 OFDM symbols.                                                                                            |
   |                                |                                                                                                                                        |
   |                                | Support for multiple bandwidth parts [cttc-nr]_:                                                                                       |
   |                                |                                                                                                                                        |
   |                                | - More than one BWP can be configured for sidelink                                                                                     |
   |                                | - Each BWP can have pre-configured multiple sidelink resource pools, but only one pool can be active at time.                          |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Duplex mode                    | TDD                                                                                                                                    |
   |                                |                                                                                                                                        |
   |                                | - The TDD pattern is flexible in length and composition, and can include downlink-only slots, uplink-only slots, or flexible slots     |
   |                                |   (in which downlink and uplink transmissions can occur). An example of TDD pattern is [D F U U U].                                    |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Sidelink Resource pool         | - Sidelink transmission is only allowed in uplink-only slots, and whether an uplink slot is available for sidelink or not is specified |
   |                                |   through the SL bitmap. An example of the SL bitmap is [1 1 1 1 1 1 0 0 0], where 1 indicates that a slot is available for sidelink   |
   |                                |   and 0 that it is not.                                                                                                                |
   |                                | - Within the uplink slots available for sidelink, the symbols available for sidelink are RRC pre-configured and our default structure  |
   |                                |   is as follows: PSCCH can occupy 1 or 2 starting symbols, and depending on the PSCCH allocation, 2nd to 13th or 3rd to 13th symbols   |
   |                                |   are available for PSSCH, and the 14th symbol is left empty as a guard period.                                                        |
   |                                | - In the frequency domain, RRC pre-configures the subchannel size (in number of RBs), and as per this configured size, divides the     |
   |                                |   available bandwidth into a number of available subchannels.                                                                          |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Sidelink data/control channels | - PSSCH and PSCCH are multiplexed in time.                                                                                             |
   |                                | - PSSCH and PSCCH are sent and received quasi-omnidirectionally at the UEs.                                                            |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Error models                   | - NR PHY abstraction for PSSCH and PSCCH channels [cttc-phy-abstraction]_ including support for MCS Table1 and Table2 [TS38214]_.      |
   |                                | - MCS LDPC coding and block segmentation [TS38212]_.                                                                                   |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Modulation                     | OFDM                                                                                                                                   |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Channel Coding                 | LDPC                                                                                                                                   |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | MCS                            | QPSK, 16-QAM, 64-QAM, 256-QAM                                                                                                          |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | HARQ                           | NR PHY abstraction for HARQ includes support for HARQ-IR and HARQ-CC                                                                   |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Retransmissions                | Blind or HARQ acknowledged retransmissions, including up to a pre-configured number with retransmission combining.                     |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Resource allocation            | Sensing-based and random resource selections are supported.                                                                            |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Link adaptation                | Fixed MCS                                                                                                                              |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Antenna models                 | 3GPP-compliant [TR38802]_:                                                                                                             |
   |                                |                                                                                                                                        |
   |                                | - Antenna arrays: 1 uniform planar array per UE, :math:`M{\times}N` antenna elements, no polarization.                                 |
   |                                | - Antenna elements: isotropical and  directional radiation are supported.                                                              |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Channel models                 | 3GPP-compliant [TR37885]_, supporting Urban grid and Highway scenarios, in both sub 6 GHz and mmWave bands.                            |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+

Architecture
============

UE architectures
================
Figure :ref:`fig-ue-sl-data-plane` and Figure :ref:`fig-ue-sl-control-plane` show
the NR module UE model architecture in the data plane and the control plane,
respectively. For NR V2X implementation, these architectures have not been
changed; however, it is noticeable that we have created sidelink specific
Service Access Point (SAP) provider and user interfaces. This choice is motivated
by the fact that having separate sidelink SAPs makes it easier to extend the
sidelink functionalities without changing the NR or LTE specific SAPs. 


.. _fig-ue-sl-data-plane:

.. figure:: figures/nr-sl-arch-ue-data-plane.*
   :align: center
   :scale: 30%

   NR Sidelink UE data plane architecture
   

.. _fig-ue-sl-control-plane:

.. figure:: figures/nr-sl-arch-ue-ctrl-plane.*
   :align: center
   :scale: 30%

   NR Sidelink UE control plane architecture


NAS
===
The establishment and management of sessions occur at the highest layer on the
control plane, the ``NAS``. The current functionalities of the ``NAS`` layer
involve establishment of ``EPS`` bearers, multiplexing uplink data packets coming
from the upper layers into the appropriate ``EPS`` bearer by using the ``TFT`` classifier.
A ``TFT`` defines the rules for mapping IP packets to the right bearer based on
IP addresses, ports, and type of service parameters.


For sidelink, the modifications are similar to the ones introduced in [nist-d2d]_.
Specifically, ``NAS`` now supports the activation of sidelink bearers, mapping of
IP packets to the sidelink bearers based only on the IP destination address of
the packets, and the transmission/reception of packets in ``NAS`` OFF state to
support out-of-coverage scenarios.


RRC
===
The ``RRC`` is the control plane protocol in charge of setting important parameters
for the session. The modifications in the ``RRC`` include the creation of the
sidelink bearers upon receiving a notification from ``NAS``, and the pre-configuration
of UEs in an out-of-coverage scenario. As mentioned earlier, the model currently
focuses on the broadcast communication, therefore, as per the standard, it supports
the creation of uni-directional sidelink radio bearers [TS38322]_.

Regarding the UEs' pre-configuration, the model implements all RRC Information Element (IE)
needed to configure a UE [TS38331]_. This configuration is of key importance to
perform sidelink communication when the gNB is absent. These IEs are mainly used
for two purposes. The first is to configure the UE's ``PHY`` layer parameters,
e.g., numerology, symbols per slot, bandwidth, and TDD pattern. The second is to
provide the sidelink resource pool(s) information to MAC and PHY layers. It is
also worth mentioning that the model allows the configuration of multiple BWPs for
sidelink, where for each BWP, more than one resource pool can be configured
through ``RRC``. We note that, in spite of supporting multiple resource pools
per BWP, only one pool can be active at one time [TS38321]_. Moreover, differently
from the standard, which uses separate pools for transmission and reception [TS38331]_,
our model uses the same active pool for both. 

Finally, for the sake of simplifying the implementation we have taken some
assumptions for the configuration of a TDD pattern, sidelink bitmap and the
resource reservation period in the simulator. In particular, one should
always respect the following conditions while configuring these parameters.

 #. The size of a sidelink bitmap should always be a multiple of the number of
    UL slots in a TDD pattern because if its size is not multiple of the number
    of UL slots, we can not construct the physical bitmap which could be applied
    repeatedly throughout a simulation.

 #. The resultant physical bitmap size should always be the multiple of resource
    reservation period used in the simulation. This is adopted to guarantee that
    once a slot is selected for sidelink communications, it is available for
    sidelink after the resource reservation period in ms and the sensing-based
    resource selection mechanism of neighboring UEs can operate adequately,
    which receive the resource reservation period of other UEs in SCI stage 1.

.. _sec-pdcp:

PDCP
====
The changes introduced in ``PDCP`` layer are in line with LTE sidelink [nist-d2d]_.
In particular, when it comes to sidelink, it is no longer possible to uniquely
identify a logical channel only based on its LCID. With sidelink communications,
UEs independently assign the LCIDs to logical channels for each
destination (i.e., Layer 2 group ID) to which they are transmitting. Thus, it is
impossible for UEs to identify the packets if multiple transmitting UEs select
the same LCID for the same group. To solve this, two more identifiers, i.e.,
source Layer-2 ID and destination Layer-2 ID, are included to identify the
transmitting UE [TS38300]_.



RLC
===
The ``RLC`` layer in the simulator already supports the so called Unacknowledged (UM),
which is the RLC mode used for sidelink broadcast communications. The only
modifications made to the ``RLC`` layer are identical to the ``PDCP`` layer.


MAC
===
The UE's ``MAC`` layer has been extensively modified to transmit and receive
sidelink transmissions. In the following, we explain these procedures in detail.
The sidelink implementation is found in class ``NrSlUeMac`` while the uplink
and downlink are found in the base class ``NrUeMac``.  Furthermore, the scheduling
implementations are separated into different classes, and the base class
``NrSlUeMacScheduler`` defines the interface between schedulers and the
``NrSlUeMac``.

MAC transmitting procedure
##########################
In out-of-coverage scenarios, UEs are required to perform the autonomous resource
selection following Mode 2, which could be based on sensing-based or random
selection procedures, as described in [TS38321]_, respectively. 
The UE ``MAC`` layer is extended to provide all the information needed by
a scheduler to perform resource selection. For example, the information related
to all the LCs of destinations the UE is interested in transmitting to,
the total number of available subchannels, :math:`N_{\max\_\text{reserve}}`, the
maximum number of PSSCH transmissions, and most importantly, the ``RLC`` sidelink
Buffer Status Reports (BSRs) of each LC that indicate how much sidelink traffic
needs to be transmitted. The second important addition is the buffering of the
sensing data reported by the UE's ``PHY`` layer. This buffer behaves like a
sensing window at the time of a sensing-based resource selection. It contains
the sensing information for the interval [:math:`n-T_\text{0}, n-T_\text{proc,0}`),
where :math:`n` is the slot at which the resource selection is triggered, and
:math:`T_\text{0}` is configured by the ``RRC`` while :math:`T_\text{proc,0}`
is a ``MAC`` layer parameter. In what follows we will dive into the details of
UE's ``MAC`` layer operation to perform sensing-based resource selection.

Resource selection is triggered on slot boundaries when the scheduler is made
aware of transport blocks (TBs) buffered at the RLC layer, for which no active
grants are already configured to handle the TBs.
At slot :math:`n`, when a resource selection is triggered for a destination,
 the scheduler first triggers the calculation of available resources within
a future selection window, according to the algorithm and constraints specified
in Section 8.1.4 of [TS38214]_.  Although the standard describes this calculation
of available resources as being performed at the ``PHY`` layer, in this ns-3
model, it is actually implemented in the ``NrSlUeMac`` based on sensing data
that has been provided by the ``NrUePhy``.  Then, for an active
pool configured by the ``RRC``, it computes the candidate resources
for sidelink transmission based on the selection window
parameters, :math:`T_\text{1}`, :math:`T_\text{2,min}`, and :math:`T_\text{2}`,
the sensing data, and the ``MAC`` history of its own transmissions.
Since the final resources must be selected based on sensing information, the MAC
follows the standard procedure described in Section 8.1.4 of [TS38214]_, to
filter out the resources from the total available ones, which could be occupied
by the other transmitting UEs. Once the filtered candidate resources' list is
ready, the ``MAC`` layer forwards it to the scheduler.  One scheduler implementation
is provided, called a ``Fixed MCS`` scheduler, which uses a configured MCS
value for all transmissions.  The scheduler determines the necessary resource
size for a single-slot resource (i.e., one or more subchannels) to encode the pending
TB(s) for the logical channel, taking into account also the 5-bytes-overhead for
2nd-stage SCI,
and then requests the ``NrSlUeMac`` to provide candidate resources within the
desired selection window.  Once these candidate resources are returned, the
scheduler selects one (if no retransmissions are configured) or more than one
(if retransmissions are configured) resources.  If the grant is semi-persistent
(SPS), the scheduler draws a random counter known as Sidelink Resource Reselection
Counter (SLRRC) based on the user configured Resource Reservation Period (PRSVP)
value, which is used to compute :math:`C_\text{resel}`.  If :math:`K` denotes
the total number of available resources, and :math:`N_\text{PSSCH,maxTx}` is the
maximum number of PSSCH configured transmissions, then 
 
.. math::

    N_{selected}=\begin{Bmatrix}
                   N_\text{PSSCH,maxTx} & \text{if}\ K \geq N_\text{PSSCH,maxTx}\\ 
                   K, & \text{otherwise}
                 \end{Bmatrix}

If feedback-based HARQ retransmissions are configured, the above
:math:`N_\text{selected}` transmissions may be further constrained, because each
retransmission must be spaced apart in time from the previous transmission such
as to allow enough time for positive ACK feedback to be returned; the scheduler
enforces this constraint.

After randomly selecting the required number of slots, it then randomly the selects
the required number of contiguous subchannels computed using a fixed MCS
strategy. 

Note that a fixed MCS scheduler is more applicable to broadcast communications;
an adaptive MCS scheduler may be more suitable for unicast and groupcast communications
where the CSI from the receiving UE can be acquired through the PSSCH.  
The C++ class design allows for other to develop more sophisticated schedulers.

Once the grant has been scheduled (including retransmissions, and, for SPS grants,
the configured value of of PRSVP and the already drawn counters, i.e., SLRRC and
:math:`C_\text{resel}`), the grant is passed to the `` the ``NrSlUeMac``, which prepares
a sidelink allocation valid for
the first resource reservation period, deciding also aspects like which slots from
the :math:`N_\text{selected}` have to carry the 1st-stage-SCI, the New Data
Indicator (NDI), and the Redundancy Version (RV) number of each slot.

After using these grants for a number of transmissions equal to the SLRRC,
a resource reselection is triggered at the scheduler. That is, once SLRRC
reaches zero, the UE either keeps the previous selection or selects new resources
based on the pre-configured probability of resource keep. Finally, if
:math:`C_\text{resel}` reaches zero, the resource reselection is triggered,
independently of this probability. As already discussed, our model also supports
a random resource selection. The only difference between the two approaches is
that the random resource selection procedure does not filter the slots from the
available ones before giving the list to the scheduler, so that the sensing
information is not used.

Before forwarding the sidelink packets to the ``PHY`` layer, a check is
performed at the beginning of each slot to ensure the availability of a valid
grant for that slot. If there is, the ``MAC`` layer prepares two packet bursts,
one for the 1st-stage-SCI and the second for the 2nd-stage-SCI plus data, and
assigns a HARQ process ID to the data packet. It also saves this data packet
into a HARQ buffer. We note that the
model allows to configure multiple (no limit for research purposes) sidelink/HARQ
processes to allow continuous flow of data. After this, both packet bursts are
forwarded to the lower layer. Upon receiving these packet bursts, the ``PHY``,
places them in a queue to be transmitted on the configured PSCCH and PSSCH symbols.

Sidelink HARQ operation
#######################

Hybrid Automatic Repeat ReQuest (HARQ) for NR Sidelink provides improved
probability of transport block delivery at the expense of additional
transmissions on the channel.  HARQ combines forward error correction (FEC)
techniques with automatic repeat request (ARQ) retransmission techniques
using positive acknowledgments.

HARQ is used in 4G LTE and 5G NR, on both the uplink and downlink.  Additional
modes of operation are defined for NR sidelink.  HARQ is a somewhat
complicated process with elements located at different parts of the NR
stack.  Using typical terminology, a HARQ Entity manages a number of HARQ
Processes and Buffers that interact with the scheduler, MAC, and PHY layers.

Sidelink HARQ is mainly implemented in the following ``nr`` module source files:

- nr-sl-ue-mac.cc
- nr-sl-ue-mac-harq.cc

The following capabilities are implemented, following 3GPP TS 38.321, section
5.22.1.3:

- A sidelink HARQ entity with a configurable number of sidelink HARQ processes;
  the default for mode 2 is four SPS processes (fixed by a constant in the code),
  and 16 overall processes (configurable by an attribute in ``NrUeMac``).

- Each sidelink process supports one transport block (TB), and a HARQ process
  ID is assigned to the MAC PDU, and the sidelink process is associated with
  a HARQ buffer.

- HARQ feedback can be enabled or disabled for groupcast, unicast, or broadcast.

- For groupcast, the positive-negative acknowledgment option is implemented;
  all receivers will return either a positive or negative acknowledgment
  (negative acknowledgment relies on successful SCI decode).

- Support of configuring a feedback channel (PSFCH) with a periodicity of
  one, two, or four sidelink slots.  The PSFCH can deliver HARQ feedback
  from a receiver to a sender using an idealized (i.e., no error model)
  channel.  The PSFCH logically occupies one symbol of a slot, but due to
  guard interval, three symbols for that slot become unusable for PSSCH.

- Support of MinTimeGapPsfch parameter of either two or three slots, to
  ensure that HARQ feedback is delayed due to notional processing delays.

- Scheduling support to ensure that constraints are met around scheduling
  future resources for HARQ retransmissions.

PSFCH slots
###########

The pattern of slots that correspond to sidelink slots is established by
the TDD pattern establishing the UL slots, and the sidelink bitmap that
is overlaid on the UL slots.  PSFCH, if enabled, is present in every
one, two, or four sidelink slots.  If PSFCH periodicity is set to zero slots,
HARQ feedback will be disabled (although blind transmissions are still
possible if HARQ is enabled in the `SidelinkInfo` structure of the TFT).

A slot with PSFCH will consume three symbols; one for the PSFCH slot
itself, one for the AGC (which duplicates the PSFCH), and an additional
guard slot.

The scheduler needs to be aware of the SL pattern as well as of the
PSFCH pattern, to determine which slots are candidates for possible
retransmisssions, because the scheduler must allow for enough time
for a transmission of a transport block to be acknowledged before
attempting a HARQ retransmission.  Additionally, the receiver must
know the PSFCH pattern in order to schedule a feedback transmission.
In ns-3, the class ``NrSlCommResourcePool`` was extended with a
``SlotHasPsfch()`` method, complementing the ``IsSidelinkSlot()`` method,
to allow ns-3 models to determine whether any given slot
is a PSFCH-enabled slot.

Retransmission Scheduling Constraints
#####################################

According to TS 38.321 section 5.22.1, when the scheduler must configure
a grant based on sensing and/or random selection, and when HARQ is
enabled and the bandwidth pool is configured for PSFCH, the scheduler
will select resources for the sidelink data such that the following
constraints are honored:

1. the packet delay budget is not exceeded,

2. the maximum number of transmissions is not exceeded,

3. the resources selected can be indicated by a prior SCI transmission, and

4. the time gap between selected resources is at least as large as the
   minimum time gap between such resources.

The standard describes this minimum time gap as follows:

.. code-block:: shell

  -	a time gap between the end of the last symbol of a PSSCH transmission of the first resource and the start of the first symbol of the corresponding PSFCH reception determined by sl-MinTimeGapPSFCH and sl-PSFCH-Period for the pool of resources; and
  -	a time required for PSFCH reception and processing plus sidelink retransmission preparation including multiplexing of necessary physical channels and any TX-RX/RX-TX switching time.
  NOTE 4:	How to determine the time required for PSFCH reception and processing plus sidelink retransmission preparation is left to UE implementation.

This essentially means that the scheduler must ensure that there exists
enough time for HARQ feedback to be generated, sent back to the
transmitter, and processed, before the next retransmission must be prepared.
If HARQ feedback indicates that no further retransmissions are needed,
the future resources reserved for that sidelink data will go unused.
This is a lost sending opportunity but will reduce possible interference.
Additionally, note that the number of slots required to wait for
this minimum time gap depends on the specific TDD patterns of future
SL and PSFCH-enabled slots, and varies over time.
The ``NrSlUeMacSchedulerFixedMcs`` scheduler class contains logic to
select slots for HARQ retransmissions subject to this scheduling
constraint.

It is possible that HARQ feedback may be delayed due to an internal
collision on the receiver-- because the system is half-duplex, the receiver
must make a priority decision whether to transmit or receive on a given
PSFCH symbol.  It could be the case that the scheduler plans for a
given PSFCH to deliver feedback, but the receiver decides not to send it.
The current ns-3 model does not yet implement handling for such collisions,
but even if/when it is implemented, the penalty for deferring the sending
of HARQ feedback may be an unnecessary retransmission by the sender,
which should not be much of a concern.

HARQ Scope and Limitations
##########################

* No support for 'negative-only' groupcast HARQ mode (a.k.a. Option 1).
* PSSCH channel error model is not sidelink-based, but reuses uplink error model
* PSFCH channel is ideal (no error model)
* Receiver-side collisions on PSFCH half-duplex operation is not supported
* Need to set the RBs correctly in varTtiInfo
* Need to add NrSpectrumPhy::TxFbTrace

.. _NrSidelinkHarqReferences:

Sidelink HARQ References
########################

The sidelink HARQ operation is mostly specified in Section
5.22.1.3 of the MAC layer specification 3GPP TS 38.321 [TS38321]_.
Aspects of the |ns3| HARQ implementation are described in [Ali21]_.  Aspects
of the |ns3| NR error model and strategy for combining multiple transmissions
at the physical layer are described in [Lag20]_.

.. [Ali21] Z. Ali, S. Lagén, L. Giupponi and R. Rouil, "3GPP NR V2X Mode 2: Overview, Models and System-Level Evaluation," in IEEE Access, vol. 9, pp. 89554-89579, 2021

.. [Lag20] S. Lagen, K. Wanuga, H. Elkotby, S. Goyal, N. Patriciello and L. Giupponi, "New Radio Physical Layer Abstraction for System-Level Simulations of 5G Networks," ICC 2020 - 2020 IEEE International Conference on Communications (ICC), 2020, pp. 1-7.

.. [TS38321] 3GPP TS 38.321, TSG RAN; NR; Medium Access Control (MAC) Protocol Specification, Release 17, v17.0.0, Mar. 2022.


MAC receiving procedure
#######################

The UE's ``MAC`` layer, upon receiving the PSSCH packet burst from the ``PHY``,
first retrieves the 2nd-stage-SCI to read the source Layer-2 ID and the
destination Layer-2 ID of the received packet. As mentioned in section :ref:`sec-pdcp`,
these identifiers are used to map the received packet to its logical channel.
If a bearer for the received packet is already established, the data packet is
forwarded to the upper layers. Otherwise, the ``MAC`` asks the ``RRC`` to
establish the bearer for the reception. Once this is done, the packet is forwarded
to the upper layers.

PHY
===
Similarly to the ``MAC`` layer, the ``PHY`` functionality can also be divided
into transmitting and receiving procedures. In the following, we describe them
in detail.

PHY transmitting procedure
##########################
The 5G-LENA simulator accurately models (as per the standard) the
numerology-dependent slot and OFDM symbol granularity. The state-machine of the
``PHY`` layer is mainly determined by the definition of the concept of start slot
event and variable TTI. When the start slot event is triggered, the processing
follows a logical order that involves the MAC and then the scheduler, before
returning the control to the PHY. For sidelink, once the control gets back to the
``PHY``, the PHY checks if the ``MAC`` has provided an allocation for the current
slot. This allocation further consists of variable TTI allocations. The
variable TTI means that the number of allocated symbols to physical sidelink
channels (i.e., PSCCH and PSSCH) is variable, based on the sidelink configuration.
Upon finding the allocation for the slot, the ``PHY`` layer transmits PSCCH and
PSSCH PDU on their respective symbols whose duration depend on the configured
numerology and CP.

PHY receiving procedure
#######################

To receive the sidelink transmissions, one of the key enhancements of the ``PHY``
is the introduction of the ability to handle collisions/interference, also
introduced in [nist-d2d]_. The interference model available in the 5G-LENA
simulator was designed for a typical cellular communication. Its design assumes
that a UE is interested in transmitting or receiving only from its serving gNB,
and assumes no interference from the UEs served by the same gNB. Transmissions
from other gNBs/UEs are simply considered as interference. In case of sidelink,
especially in broadcast or groupcast, a UE is interested in transmitting to or
receiving from multiple surrounding UEs. In this context, UEs in out-of-coverage
scenarios or ``UE-selected'' mode can select the same (or overlapping) resources
because the allocation is uncoordinated. Therefore, to determine which packet
will be successfully decoded, the new implementation keeps track of the SINR
values for each sidelink transmission.

As described earlier, currently our model supports the transmission and reception
of timely multiplexed PSCCH and PSSCH. Thus, the ``PHY`` first receives signal(s)
(i.e., 1st-stage-SCI) transmitted over PSCCH. This signal is used for two purposes:
1) to measure the RSRP required for the sensing-based resource selection,
2) to retrieve the information about the possible PSSCH transmission and retransmissions.
The RSRP is computed using the 3 Resource Elements (REs) per RBs, carrying the
1st-stage-SCI, since the simulator does not explicitly include PSCCH DMRS. Moreover,
for the sensing-based resource selection, the ``PHY`` measures the RSRP of each
correctly decoded 1st-stage-SCI, from all the surrounding UEs. On the other hand,
after computing the RSRP, if it is from the transmitter of interest, it reads
the information encoded in the 1st-stage-SCI to receive the PSSCH transmission
and its possible retransmissions.


Concerning the error model used for the reception of PSCCH and PSSCH transmission,
we use the existing data plane error model in 5G-LENA [cttc-phy-abstraction]_,
since the MCS defined for PSSCH are the same as the ones defined for PDSCH/PUSCH.
Also, we adopt such an error model for the PSCCH, using MCS0.



Channel Models
==============
TR 37.885 [TR37885]_ defines the system-level evaluation methodology for 5G V2X use
cases, including the description and modeling of scenarios, deployment, mobility,
antenna, traffic, and channel models. For channel modeling, TR 37.885 extends the
geometry-based stochastic channel modeling framework introduced in TR 38.901 [TR38901]_
for typical cellular communications, by adding the possibility to model wireless
channel in vehicular environments and sidelink communications in which both the
transmitter and the receiver are in motion.

 
Two key scenarios are used for NR V2X evaluation:

  #. Urban grid, which targets urban environments with a grid of buildings and
     roads with four lanes (two in each direction) between the buildings, and
 
  #. Highway, which targets highway environments with a highway composed of a
     total of six lanes, considering three lanes in each opposite direction.

For each scenario, TR 37.885 specifies new channel condition models, propagation
models, and fast fading parameters capturing the characteristics of each environment. 

The developed ns-3 NR V2X module includes the channel and
antenna models for both V2X Urban grid and Highway scenarios, as defined
in the standard [v2x-channel-ns-3]_.


NR V2X KPI management framework
===============================
For the sake of computing KPIs for the NR V2X simulations, we have developed a
KPI Management framework, which is comprised on several APIs that can be
categorized as follows:

 #. The APIs responsible of listening to the specific ns-3 traces and store the
    information in a SQLITE database in the form of tables.
    
 #. The APIs responsible to read the SQLITE tables written by the category-1
    APIs, and compute V2X specific KPIs. These KPIs are then stored
    in the same database by creating new tables specific to each KPI. 
    
The table :ref:`tab-nr-v2x-kpis` lists all these APIs and their functionalities.

.. tabularcolumns:: |p{4.5cm}|c|p{8cm}|

.. _tab-nr-v2x-kpis:

.. table:: NR V2X KPI management framework APIs
   :class: longtable
  
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | Class Name               | Category | Functionalities                                                                   |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UeMacPscchTxOutputStats  |     1    | Listens and stores the                                                            |
   |                          |          | SlPscchScheduling trace of NrUeMac                                                |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UeMacPsschTxOutputStats  |     1    | Listens and stores the                                                            |
   |                          |          | SlPsschScheduling trace of NrUeMac                                                |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UePhyPscchRxOutputStats  |     1    | Listens and stores the                                                            |
   |                          |          | RxPscchTraceUe trace of NrSpectrumPhy                                             |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UePhyPsschRxOutputStats  |     1    | Listens and stores the                                                            |
   |                          |          | RxPsschTraceUe trace of NrSpectrumPhy                                             |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UeRlcRxOutputStats       |     1    | Listens and stores the                                                            |
   |                          |          | RxRlcPduWithTxRnti trace of NrUeMac                                               |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UeToUePktTxRxOutputStats |     1    | Listen and store the application level                                            |
   |                          |          | traces of type TxWithSeqTsSize and RxWithSeqTsSize                                |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | V2xKpi                   |     2    | This class reads the specific tables of a                                         |
   |                          |          | given DB written by category 1 APIs to compute                                    |
   |                          |          | the following V2X KPIs:                                                           |
   |                          |          |                                                                                   |
   |                          |          |  - Average Packet Inter-Reception (PIR) (range-based and not range based)         |
   |                          |          |                                                                                   |
   |                          |          |  - Average Average Packet Reception Ratio (PRR) (range-based and not range based) |
   |                          |          |                                                                                   |
   |                          |          |  - Throughput                                                                     |
   |                          |          |                                                                                   |
   |                          |          |  - Simultaneous PSSCH Transmissions                                               |
   |                          |          |                                                                                   |
   |                          |          |  - PSSCH TB RX stats                                                              |
   +--------------------------+----------+-----------------------------------------------------------------------------------+







.. [cttc-nr-v2x] Zoraze Ali, Sandra Lagen, Lorenza Giupponi and Richard Rouil, "3GPP NR V2X Mode 2: Overview, Models and System-Level Evaluation," in IEEE Access, vol. 9, pp. 89554-89579, 2021.

.. [cttc-nr] Natale Patriciello, Sandra Lagen, Biljana Bojovic, and Lorenza Giupponi, “An E2E simulatorfor 5G NR networks,” Simulation Modelling Practice and Theory, vol. 96, p. 101933, 2019.

.. [cttc-phy-abstraction] Sandra Lagen, K. Wanuga, H. Elkotby, S. Goyal, Natale Patriciello, and Lorenza Giupponi, “New Radio Physical Layer Abstraction for System-Level Simulations of 5G Networks,” in IEEE International Conference on Communications, June 2020.

.. [nist-d2d] Richard Rouil, F. J. Cintrón, A. B. Mosbah, and S. Gamboa, “Implementation and Validation of an LTE D2D Model for ns-3,” in Workshop on Ns-3, 2017.

.. [v2x-channel-ns-3] Tommaso Zugno, Matteo Drago, Sandra Lagen, Zoraze Ali, and Michele Zorzi, “Extending the ns-3  spatial  channel  model  for  vehicular  scenarios,”  inWorkshop  on  Ns-3,2021

.. [TR38802] 3GPP TR 38.802, Study on New Radio (NR) Access Technology; Physical Layer Aspects (Release 14), v14.2.0, Sep. 2017.

.. [TR37885] 3GPP TR 37.885, Study on Evaluation Methodology of New Vehicle-to-Everything  (V2X) Use Cases for LTE and NR (Rel. 15), V15.3.0, Jun.2019.

.. [TS38321] 3GPP TS 38.321, TSG RAN; NR; Medium Access Control (MAC) Protocol Specification, Release 15, v16.3.0, Jan. 2021.

.. [TS38322] 3GPP TS 38.322, TSG RAN; NR; Radio Link Control (RLC) protocol specification, Release 16, v16.2.0, Jan. 2021.

.. [TR38901] 3GPP TR 38.901, Study on Channel Model for Frequencies from 0.5 to 100 GHz (Rel. 15), V15.0.0, Jun. 2019.
