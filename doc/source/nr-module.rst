Example Module Documentation
----------------------------

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

..
   The document is written following RST formatting. Please, check your writing on grammarly; to ease it, use the auto-wrapping feature of your text editor, without entering new lines unless you would like to start a new paragraph.

.. toctree::
   :maxdepth: 3
   :caption: Table of Contents

Introduction
------------
The 3rd Generation Partnership Project (3GPP) has devoted significant efforts to standardize the fifth-generation (5G) New Radio (NR) access technology [TS38300]_, which is designed to be extremely flexible from its physical layer definition and up to the architecture. The objective is to be able to work in a wide range of frequency bands and address many different use cases and deployment options.

As the NR specification is developed and evolves, a network simulator that is capable of simulating emerging NR features is of great interest for both scientific and industrial communities. In recent years a lot of effort has been made by New York University (NYU) Wireless and the University of Padova to develop a simulator that will allow simulations of communications in millimeter-wave (mmWave) bands, the heart of future 5G cellular wireless systems. Hence, a new mmWave simulation tool has been developed as a new module of |ns3|. A complete description of the mmWave module is provided in [end-to-end-mezz]_. The mmWave module source code is still not part of the standard ns-3 distribution and is available at a different repository [mmwave-module]_. In the mmWave module, the physical (PHY) layer and medium access control (MAC) are a modified version of the |ns3| 'LTE' PHY and MAC layers, supporting a mmWave channel, propagation, beamforming, and antenna models. The MAC layer supports Time Division Duplexing (TDD), and a Time Division Multiple Access (TDMA) MAC scheduling, with enhanced Hybrid Automatic Repeat and reQuest (HARQ) for low latency applications. The higher layers are mostly based on |ns3| 'LTE' module functionalities but are extended to support features such as dual connectivity and low latency radio link control (RLC) layer.

In this document, we describe the implementation that we have initiated to generate a 3GPP-compliant NR module able to provide |ns3| simulation capabilities in the bands above and below 6 GHz, aligned with 3GPP NR Release-15, following the description in [TS38300]_. The work has been initially funded by InterDigital Communications Inc, and continues with funding from the Lawrence Livermore National Lab (LLNL) and a grant from the National Institute of Standards and Technologies (NIST).

The 'NR' module is a hard fork of the 'mmWave' simulator,  focused on targeting the 3GPP Release-15 NR specification. As such, it incorporates fundamental PHY-MAC NR features like a flexible frame structure by means of multiple numerologies support, bandwidth parts (BWPs), Frequency Division Multiplexing (FDM) of numerologies, Orthogonal Frequency-Division Multiple Access (OFDMA), flexible time- and frequency- resource allocation and scheduling, Low-Density Parity Check (LDPC) coding for data channels, modulation and coding schemes (MCSs) with up to 256-QAM, and dynamic TDD, among others. The NR module still relies on higher layers and core network (RLC, PDCP, RRC, NAS, EPC) based on |ns3| 'LTE' module, thus providing an NR non-standalone (NSA) implementation.

The source code for the 'NR' module lives currently in the directory ``src/nr``.

Over time, extensions found in this module may migrate to the existing |ns3| main development tree.

The rest of this document is organized into five major chapters:

2. **Design:**  Describes the models developed for |ns3| extension to support NR features and procedures.
3. **Usage:**  Documents how users may run and extend the NR test scenarios.
4. **Validation:**  Documents how the models and scenarios have been verified and validated by test programs.
5. **Open Issues and Future Work:**  Describes topics for which future work on model or scenario enhancements is recommended, or for which questions on interpretations of standards documents may be listed.


Design
------
In this section, we present the design of the different features and procedures that we have developed following 3GPP Release-15 NR activity. For those features/mechanisms/layers that still have not been upgraded to NR, the current design following LTE specifications is also indicated.


Architecture
************
The 'NR' module has been designed to perform end-to-end simulations of 3GPP-oriented cellular networks. The end-to-end overview of a typical simulation with the 'NR' module is drawn in Figure:ref:`fig-e2e`. In dark gray, we represent the existing, and unmodified, ns-3 and LENA components. In light gray, we describe the NR components. On one side, we have a remote host (depicted as a single node in the Figure, for simplicity, but there can be multiple nodes) that connects to an SGW/PGW (Service Gateway and Packet Gateway), through a link. Such a connection can be of any technology that is currently available in ns-3. It is showed through a single link, but there are no limits on the topology, including any number of remote hosts. Inside the SGW/PGW, the ``EpcSgwPgwApp`` encapsulates the packet using the GTP protocol. Through an IP connection, which represents the backhaul of the NR network (again, described with a single link in the Figure, but the topology can vary), the GTP packet is received by the gNB. There, after decapsulating the payload, the packet is transmitted inside the NR stack through the entry point represented by the class ``NRGnbNetDevice``. The packet, if received correctly at the UE, is passed to higher layers by the class ``NRUeNetDevice``. The path crossed by packets in the UL case is the same as the one described above but on the contrary direction.

.. _fig-e2e:

.. figure:: figures/drawing2.*
   :align: center
   :scale: 80 %

   End-to-end class overview

Concerning the RAN, we detail what is happening between ``NRGnbNetDevice`` and ``NRUeNetDevice`` in Figure :ref:`fig-ran`. The ``NRGnbMac`` and ``NRUeMac`` MAC classes implement the LTE module SAP provider and user interfaces, enabling the communication with the LTE RLC layer. The module supports RLC TM, SM, UM, and AM modes. The MAC layer contains the scheduler (``NRMacScheduler`` and derived classes). Every scheduler also implements an SAP for LTE RRC layer configuration (``LteEnbRrc``). The ``NrPhy`` classes are used to perform the directional communication for both downlink (DL) and uplink (UL), to transmit/receive the data and control channels. Each ``NrPhy`` class writes into an instance of the ``NrSpectrumPhy`` class, which is shared between the UL and DL parts.

.. _fig-ran:

.. figure:: figures/drawing1.png
   :align: center
   :scale: 80 %

   RAN class overview

Interesting blocks in Figure :ref:`fig-ran` are the ``NRGnbBwpM`` and ``NRUeBwpM`` layers. 3GPP does not explicitly define them, and as such, they are virtual layers. Still, they help construct a fundamental feature of our simulator: the multiplexing of different BWPs. NR has included the definition of 3GPP BWPs for energy-saving purposes, as well as to multiplex a variety of services with different QoS requirements. Component carrier concept was already introduced in LTE, and persists in NR through our general BWP concept, as a way to aggregate carriers and so improve the system capacity. In the 'NR' simulator, it is possible to divide the entire bandwidth into different BWPs. Each BWP can have its PHY and MAC configuration (e.g., specific numerology, scheduler rationale, and so on). We added the possibility for any node to transmit and receive flows in different BWPs, by either assigning each bearer to a specific BWP or distributing the data flow among different BWPs, according to the rules of the manager. The introduction of a proxy layer to multiplex and demultiplex the data was necessary to glue everything together, and this is the purpose of these two new classes (``NRGnbBwpM`` and ``NRUeBwpM``).

Note: The 3GPP definition for "Bandwidth Part" (BWP) is made for energy-saving purposes at the UE nodes. As per the 3GPP standard, the active 3GPP BWP at a UE can vary semi-statically, and multiple 3GPP BWPs can span over the same frequency spectrum region. In this text, and through the code, we use the word BWP to refer to various things that are not in line with the 3GPP definition.

First of all, we use it to indicate the minimum piece of spectrum that can be modeled. In this regard, a BWP has a center frequency and a bandwidth, plus some characteristics for the 3GPP channel model (e.g., the scenario). Each device can handle multiple BWPs, but such BWPs must be orthogonal in frequency (i.e., they must span over different frequency spectrum regions, that can be contiguous or not, depending on the user-made configuration).

Secondly, the 'NR' module is communicating through each BWP with a PHY and a MAC entity, as well as with one spectrum channel and one antenna instance. In other words, for every spectrum bandwidth part, the module will create a PHY, a MAC, a Spectrum channel, and an antenna. We consider, in the code, that this set of components form a BWP. Moreover, we have a router between the RLC queues and the different MAC entities, which is called the BWP manager.

Summarizing, our BWP terminology can refer to orthogonal 3GPP BWPs, as well as to orthogonal 3GPP Component Carriers, and it is up to the BWP manager to route the flows accordingly based on the behavior the user wants to implement. Our primary use case for bandwidth part is to avoid interferences, as well as to send different flow types through different BWPs, to achieve a dedicated-resource RAN slicing.

Identifying components
**********************

Often, you will need to identify the part from which some messages come from, or to be able to read the output traces correctly. Each message will be associated with one couple -- ccId and bwpId. The meaning of these names does not reflect the natural sense that we could give to these words. In particular, the definition is the following:

* the bwpId is the index of an imaginary vector that holds all the instances of BWP (as a pair MAC/PHY) in the node. It is assigned at the creation time by the helper, and the BWP with ID 0 will be the primary carrier;
* the ccId is a number that identifies the pair MAC/PHY uniquely in the entire simulation.

All the nodes in the simulation will have the same BWP amount. Each one will be numbered from 0 to n-1, where n is the total number of spectrum parts. For example:

.. _tab-example-spectrum:

.. table:: An example spectrum division

   +------------+------------+---------------+---------------+---------------+
   |                                Band 1                                   |
   +------------+------------+---------------+---------------+---------------+
   |   CC 0                  |       CC 1                    |      CC2      |
   +------------+------------+---------------+---------------+---------------+
   |          BWP0           |              BWP1             |      BWP2     |
   +------------+------------+---------------+---------------+---------------+


The ccId numbering is, for some untrained eyes, weird. But that is because some numbers in the sequence are used to identify the Cell ID. For example, let's consider a scenario in which we split the spectrum into three parts. We have four GNBs, and the numbering will be the following:

.. _tab-example-bwp:

.. table:: CcId numbering example

   +------------+------------+---------------+---------------+---------------+
   |    GNB     |  Cell ID   | CcId for BWP0 | CcId for BWP1 | CcId for BWP2 |
   +============+============+===============+===============+===============+
   |   GNB 0    |      1     |       2       |       3       |      4        |
   +------------+------------+---------------+---------------+---------------+
   |   GNB 1    |      5     |       6       |       7       |      8        |
   +------------+------------+---------------+---------------+---------------+
   |   GNB 2    |      9     |       10      |       11      |      12       |
   +------------+------------+---------------+---------------+---------------+
   |   GNB 3    |      13    |       14      |       15      |      16       |
   +------------+------------+---------------+---------------+---------------+

If we would use this as a simulation scenario, the messages that come from the CcId 2, 6, 10, 14, would refer to the same portion of the spectrum. These IDs, internally at the GNB, would be translated into the BWP 0 in all the cases. The BWP 1 will be associated with the CcId 3, 7, 11, 15 (respectively), and everything else comes easily.

PHY layer
*********
This section describes in details the different models supported and developed at PHY layer.


Frame structure model
=====================
In NR, the 'numerology' concept is introduced to flexibly define the frame structure in such a way that it can work in both sub-6 GHz and mmWave bands. The flexible frame structure is defined by multiple numerologies formed by scaling the subcarrier spacing (SCS) of 15 kHz. The supported numerologies (0, 1, 2, 3, 4)  correspond to SCSs of 15 kHz, 30 kHz, 60 kHz, 120 kHz, and 240 kHz. The SCS of 480 kHz is under study by 3GPP, but the simulator can support it. Theoretically, not all SCS options are supported in all carrier frequencies and channels. For example, for sub 6 GHz, only 15 kHz, 30 kHz, and 60 kHz are defined. Above 6 GHz, the supported ones are 60 kHz, 120 kHz, and 240 kHz. Also, for numerology 2 (i.e., SCS = 60 KHz), two cyclic prefixes (CP) overheads are considered: normal and extended. For the rest of the numerologies, only the normal overhead is taken into consideration.

.. _tab-numerologies-3gpp:

.. table:: Numerologies defined in 3GPP NR Release-15

   ===========   =========================   =============
   Numerology    Subcarrier spacing in kHz   Cyclic prefix
   ===========   =========================   =============
    0            15                          normal
    1            30                          normal
    2            60                          normal, extended
    3            120                         normal
    4            240                         normal
   ===========   =========================   =============

In the time domain, each 10 ms frame is split in time into ten subframes, each of duration of 1 ms. Every subframe is split in time into a variable number of slots, and each slot is composed of a fixed number of OFDM symbols. In particular, the length of the slot and the number of slots per subframe depend on the numerology, and the length of the OFDM symbol varies according to the numerology and CP. The number of OFDM symbols per slot is fixed to 14 symbols for normal CP, and to 12 OFDM symbols for extended CP.

In the frequency domain, the number of subcarriers per physical resource block (PRB) is fixed to 12, and the maximum number of PRBs, according to Release-15, is 275. With a particular channel bandwidth, the numerology defines the size of a PRB and the total number of PRBs usable by the system. PRBs are grouped into PRB groups at MAC scheduling time.

Figure :ref:`fig-frame` shows the NR frame structure in time- and frequency- domains for numerology 3 with normal CP and a total channel bandwidth of 400 MHz.

.. _fig-frame:

.. figure:: figures/numerologies-1.*
   :align: center
   :scale: 35 %

   NR frame structure example

The implementation in the 'NR' module currently supports the NR frame structures and numerologies shown in Table :ref:`tab-numerologies`. Once the numerology is configured, the lengths of the symbol, the slot, the SCS, the number of PRBs within the bandwidth, and the number of slots per subframe, are dynamically determined in a runtime, based on Table :ref:`tab-numerologies`.

.. _tab-numerologies:

.. table:: Implemented NR numerologies

   ===========   ==================   ==================     ================   =========================   ================
   Numerology    Slots per subframe   Symbol length (μs)     Slot length (ms)   Subcarrier spacing in kHz   Symbols per slot
   ===========   ==================   ==================     ================   =========================   ================
    0            1                         71.42                1                  15                          14
    1            2                         35.71                0.5                30                          14
    2            4                         17.85                0.25               60                          14
    3            8                         8.92                 0.125              120                         14
    4            16                        4.46                 0.0625             240                         14
    5            32                        2.23                 0.03125            480                         14
   ===========   ==================   ==================     ================   =========================   ================

In the 'NR' module, to support a realistic NR simulation, we accurately model (as per the standard) the numerology-dependent slot and OFDM symbol granularity. We use the event scheduling feature of |ns3| to model the time advancement. Starting from time 0, we insert (and process) events that represent the advancement of the time. One of such events is the starting slot boundary, where the processing follows a logical order that involves the MAC, then the scheduler, before returning the control to the PHY. Here, allocations are extracted, and, for each assignment, a new event is inserted in the simulator. The last added event is the end of the slot, which in turn will invoke a new starting slot boundary event.

Two factors influence the processing of events and allocations. The first is the availability of the channel: in NR, the channel is always available for transmission, while in the unlicensed spectrum, this may not be true. If the channel is not available, the event machine will go directly to the next slot boundary (with some details that are not explained here). The second is the MAC-to-PHY processing delay: a configurable parameter defaulted to 2 slots, indicates that the MAC is working ahead of the PHY to simulate the time needed for each component of the chain to perform its work. For example, in numerology 0, PHY level at Frame 10, Subframe 0, Slot 0, will call MAC to realize the allocations of Frame 10, Subframe 2, Slot 0.

Some of the details of what is explained above is present in the papers [WNS32018-NR]_, [CAMAD2018-NR]_, along with some performance evaluations.


FDM of numerologies
===================
An additional level of flexibility in the NR system can be achieved by implementing the multiplexing of numerologies in the frequency domain. As an example, ultra-reliable and low-latency communications (URLLC) traffic requires a short slot length to meet strict latency requirements, while enhanced mobile broadband (eMBB) use case in general aims at increasing throughput, which is achieved with a large slot length. Therefore, among the set of supported numerologies for a specific operational band and deployment configuration, URLLC can be served with the numerology that has the shortest slot length and eMBB with the numerology associated with the largest slot length. To address that, NR enables FDM of numerologies through different BWPs, to address the trade-off between latency and throughput for different types of traffic by physically dividing the bandwidth in two or more BWPs. In Figure :ref:`fig-bwp`, we illustrate an example of the FDM of numerologies. The channel is split into two BWPs that accommodate the two numerologies multiplexed in the frequency domain. The total bandwidth :math:`B` is then divided into two parts of bandwidth :math:`B_u` for URLLC and :math:`B_e` for eMBB, so that :math:`B_u+B_e \le B`.

.. _fig-bwp:

.. figure:: figures/bwp.*
   :align: center
   :scale: 80 %

   FDM of numerologies example

In the 'NR' module, the user can configure FDM bands statically before the simulation starts. This is a critical design assumption based on two main reasons. First, the 'NR' module relies on the channel and the propagation loss model that is not able to allow runtime modifications of the physical configuration parameters related to time/frequency configuration (such as the system bandwidth, the central carrier frequency, and the symbol length). Thus, until the current channel model is not modified to allow these runtime configuration changes, it will not be possible to perform semi-static reconfiguration of BWPs. The second reason is that in the simulator, the RRC messaging to configure the default BWP, as well as the BWP reconfiguration, are not supported yet. See implementation details and evaluations in [WNS32018-NR]_, which is inspired in [CA-WNS32017]_.


Duplexing schemes
=================
The 'NR' simulator supports both TDD and FDD duplexing modes in a flexible manner. Indeed a gNB can be configured with multiple carriers, some of them being paired (for FDD), and others being TDD. Each carrier can be further split into various BWPs, under the assumption that all the BWPs are orthogonal in frequency, to enable compatibility with the channel instances. The gNB can simultaneously transmit and receive from multiple BWPs. However, from the UE side, we assume the UE is active in a single BWP at a time.


TDD model
#########
NR allows different slot types: DL-only ("DL" slots), UL-only ("UL" slots), and Flexible ("F" slots). Flexible slots have a certain number of DL symbols, a guard band, and a certain number of UL symbols. For the DL-only and UL-only case, the slots have, as the name suggests, only DL or only UL symbols. A TDD pattern in NR, repeated with a pre-configured periodicity, is a set of the previously defined slot types.

In the 'NR' module, the TDD pattern is represented by a vector of slot types, where the length and the content of such vector are user-defined. In the case of Flexible slots, the first and the last OFDM symbols are reserved for DL CTRL and UL CTRL, respectively (e.g., DCI and UCI). The symbols in between can be dynamically allocated to DL and UL data, hence supporting dynamic TDD. In the case of DL-only slots, the first symbol is reserved for DL CTRL, and the rest of the symbols are available for DL data. In the case of UL-only slots, the last symbol is reserved for UL CTRL, and the rest of the symbols are available for UL data.

The model also supports the special slot type ("S" slots) to emulate LTE. In those slots, the first symbol is reserved for DL CTRL, the last slot is reserved for UL CTRL, and the rest of the symbols are available for DL data.

Note there are not limitations in the implementation of the TDD pattern size and structure. However, one must ensure there is no a large gap in between two DL slots, or two UL slots, so that different timers (e.g., RRC, HARQ, CQI) do not expire simply by the TDD pattern.


FDD model
#########
In the 'NR' module, FDD duplexing is modeled through the usage of two paired bandwidth parts, where one is dedicated to transmitting DL data and contrl, and the other for the transmission of the UL data and control. The user would configure each bandwidth part with a DL-only (or UL-only) pattern, and then configure a linking between the two bandwidth parts for the correct routing of the control messages. As an example, the HARQ feedback for a DL transmission will be uploaded through the UL-only bandwidth part, but it applies to the DL-only bandwidth part: the configuration is needed for correctly routing that message from one bandwidth part to the other.

This FDD model supports the pairing only between bandwidth parts configured with the same numerology.

How the time looks like in both schemes
#######################################
In both schemes, the time starts at the beginning of the slot. The GNB PHY retrieves the allocations made by MAC for the specific slot, and extract them one by one. Depending on the allocation type, the PHY schedules the variable TTI type. For instance, most probably in DL or F slot, the first symbol is allocated to the CTRL, so the GNB starts transmitting the CTRL symbol(s). The UE begins as well receiving these CTRLs, thanks to the fact that it (i) knows the type of the slot, and (ii) at the registration time it discovers how many symbols are reserved for the DL CTRL. In this (these) symbol(s), the UE receives the DCI. Based on the received DCIs, it can schedule multiple variable TTI to (i) receive data if it received DL DCI or (ii) send data if it received UL DCI. In UL or F slots, at the end of the slot, there will be a time in which the UE will be able to transmit its UL CTRL data. The GNB specifies this time at the registration time, and it is considered that it will be the last operation in the slot.

When the slot finishes, another one will be scheduled, which will be like what we described before.


CQI feedback
============
NR defines a Channel Quality Indicator (CQI), which is reported by the UE and can be used for MCS index selection at the gNB for DL data transmissions. NR defines three tables of 4-bit CQIs (see Tables 5.2.2.1-1 to 5.2.2.1-3 in [TS38214]_), where each table is associated with one MCS table. In the simulator, we support CQI Table1 and CQI Table2 (i.e., Table 5.2.2.1-1 and Table 5.2.2.1-2), which are defined based on the configured error model and corresponding MCS Table.

At the moment, we support the generation of a *wideband* CQI that is computed based on the data channel (PDSCH). Such value is a single integer that represents the entire channel state or better said, the (average) state of the resource blocks that have been used in the gNB transmission (neglecting RBs with 0 transmitted power).

The CQI index to be reported is obtained by first obtaining an SINR measurement and then passing this SINR measurement to the Adaptive Modulation and Coding module (see details in AMC section) that maps it to the CQI index. Such value is computed for each PDSCH reception and reported after it.

In case of UL transmissions, there is not explicit CQI feedback, since the gNB directly indicates to the UE the MCS to be used in UL data transmissions. In that case, the gNB measures the SINR received in the PUSCH, and computes based on it the equivalent CQI index, and from it the MCS index for UL is determined.


Power allocation
================
In the simulator, we assume a uniform power allocation over the whole set of RBs that conform the bandwidth of the BWP. That is, power per RB is fixed. However, if a RB is not allocated to any data transmission, the transmitted power there is 0, and no interference is generated in that RB.


Interference model
==================
The PHY model is based on the well-known Gaussian interference models, according to which the powers of interfering signals (in linear units) are summed up together to determine the overall interference power. The useful and interfering signals, as well as the noise power spectral density, are processed to calculate the SNR, the SINR, and the RSSI (in dBm).

Also, such powers are used to determine if the channel is busy or empty. For that, we are creating two events, one that adds, for any signal, the received power and another that subtracts the received power at the end time. These events determine if the channel is busy (by comparing it to a threshold) and for how long.


Spectrum model
==============
In the simulator, radio spectrum usage follows the usual way to represent radio transmission in the ns-3 simulator [baldo2009]_. The core is an object that represents the channel characteristic, including the propagation, following the 3GPP specifications [gpp-channel-dev]. In the simulation, there will be as many channel models as the user needs, remembering that two (or more) channel models cannot overlap over the spectrum frequencies. In the NR nodes, there will be as many physical layers as the number of channel models; each physical layer communicates to its channel model through a spectrum model instance that owns a model of the physical layer antenna. The combination of the sender's and receiver's antenna gain (given by the configured beam and the antenna element radiation pattern), the propagation loss, and the channel characteristics, provide the value of the received power spectral density for each transmitted signal. The interference among different nodes is calculated using the MultiModelSpectrumChannel described in [baldo2009]_. In this way, we can simulate dynamic spectrum access policies, as well as dynamic TDD schemes, considering downlink-to-uplink and uplink-to-downlink interferences.


Data PHY error model
====================
The PHY abstraction of NR based systems is a complex task due to the multiple
new features added to NR. In NR, in addition to number of RBs, the number of
OFDM symbols can also be variably allocated to a user, which in combination with
wide-bandwidth operation significantly increases the number of supported
transport block sizes (TBSs). The inclusion of LDPC coding for data channels (i.e., PDSCH and PUSCH)
with multiple lifting sizes and two types of base graphs increases the complexity of
the code block segmentation procedure at PHY. Moreover, NR supports
various configurations for MCS tables, and modulation orders up to 256-QAM.
All these features have been considered to model NR performance appropriately.

The 'NR' module includes a PHY abstraction model for error modeling that is compliant with the
latest NR specifications, including LDPC coding,
MCS up to 256-QAM, different MCS Tables (MCS Table1 and MCS Table2),
and NR transport block segmentation [TS38214]_ [TS38212]_. Also, the developed PHY
abstraction model supports HARQ
based on Incremental Redundancy (IR) and on Chase Combining (CC), as we will present in
the corresponding section.

Let us note that the attribute ``ErrorModelType`` configures the type of error modelling, which can be set to NR (ns3::NrEesmCcT1, ns3::NrEesmIrT1, ns3::NrEesmCcT2, ns3::NrEesmIrT2) or to LTE (ns3::NrLteMiErrorModel, default one) in case one wants to reproduce LTE PHY layer. In the NR case, the HARQ method and MCS table are configured according to the selected error model, e.g., ns3::NrEesmCcT1 uses HARQ-CC and MCS Table1.

The error model of the NR data plane in the 'NR' module is developed according to standard
link-to-system mapping (L2SM) techniques. The L2SM choice is aligned with the
standard system simulation methodology of frequency-selective
channels. Thanks to L2SM we are able to maintain a good
level of accuracy and at the same time limiting the computational complexity
increase. It is based on the mapping of single link layer performance obtained
by means of link level simulators to system (in our case network) simulators.
In particular a link-level simulator is used for generating the performance
of a single link from a PHY layer perspective, in terms of code block
error rate (BLER), under specific conditions. L2SM allows the usage
of these parameters in more complex scenarios, typical of system/network-level
simulators, where we have more links, interferences and frequency-selective fading.

To do this, a proprietary simulator of InterDigital Inc., compliant with NR specifications,
has been used for what concerns the extraction of link-level performance
by using the Exponential Effective SINR (EESM) as the L2SM mapping function.

The overall NR PHY abstraction model that is implemented in the 'NR' module is shown in
Figure :ref:`fig-l2sm`. The L2SM process receives inputs consisting of a vector
of SINRs per allocated RB, the MCS selection (including MCS index and the MCS
table to which it refers), the TBS delivered to PHY, and the HARQ history. Then,
it provides as output the BLER of the MAC transport block.
The model consists of the following blocks: SINR compression, LDPC base graph (BG) selection,
segmentation of a transport block into one or multiple code blocks
(known as code block segmentation), mapping of the effective SINR to BLER
for each PHY code block (denoted as code BLER),
and mapping of code BLERs to the transport BLER.

.. _fig-l2sm:

.. figure:: figures/l2sm-1.*
   :align: center
   :scale: 60 %

   NR PHY abstraction model

The HARQ history depends on the HARQ method. In HARQ-CC, the HARQ history contains
the SINR per allocated RB, whereas for HARQ-IR, the HARQ history contains the last
computed effective SINR and number of coded bits of each of the previous retransmissions.
Given the SINR vector and the HARQ history, the effective SINR is computed according to
EESM. The optimization of EESM is performed using the NR-compliant link-level simulator.
The LDPC BG selection follows NR specifications, which uses TBS and MCS selection, are detailed next.
Once the BG selection is known, the code block segmentation (if needed) is performed
to derive the number of code blocks and the number of bits in each code block,
which is also known as code block size (CBS), also as per NR specs. Given the effective SINR,
the ECR, the MCS selection, and the CBS, the corresponding code BLER can be
found using SINR-BLER lookup tables obtained from the NR-compliant link-level simulator.
Finally, based on the number of code blocks and the code BLER, the transport BLER
of the transport block is obtained. In what follows we detail the different blocks and
NR features supported by the model.

**MCS**: NR defines three tables of MCSs: MCS Table1 (up to 64-QAM),
MCS Table2 (up to 256-QAM), and MCS Table3 (up to 64-QAM with low spectral efficiency),
which are given by Tables 5.1.3.1-1 to 5.1.3.1-3 in [TS38214]_.
A base station can indicate the table selection to a UE either
semi-statically or dynamically, and the MCS index selection is communicated to
the UE for each transmission through the DCI.
Each MCS index defines an ECR, a modulation order,
and the resulting spectral efficiency (SE).

In the 'NR' module, MCS Table1 and MCS Table 2 can be selected.
The MCS Table1 includes from MCS0 (ECR=0.12, QPSK, SE=0.23 bits/s/Hz)
to MCS28 (ECR=0.94, 64-QAM, SE=5.55 bits/s/Hz), whereas the MCS Table2
has MCS indices from MCS0 (ECR=0.12, QPSK, SE=0.23 bits/s/Hz) to MCS27
(ECR=0.93, 256-QAM, SE=7.40 bits/s/Hz).
As shown in Figure :ref:`fig-l2sm`, the MCS Table (1 or 2) and the
MCS index (0 to 28 for MCS Table1, and 0 to 27 for MCS Table2) are
inputs for the NR PHY abstraction.

**LDPC BG selection**: BG selection in the 'NR' module is based on the following
conditions  (as per Sections 6.2.2 and 7.2.2 in TS 38.212) [TS38212]_. Assuming :math:`R` as the ECR of the selected MCS
and :math:`A` as the TBS (in bits), then,

* LDPC base graph 2 (BG2) is selected if :math:`A \le 292` with any value of :math:`R`, or if :math:`R \le 0.25` with any value of :math:`A`, or if :math:`A \le 3824` with :math:`R \le 0.67`,
* otherwise, the LDPC base graph 1 (BG1) is selected.

**Code block segmentation**: Code block segmentation for LDPC coding in
NR occurs when the number of total bits in a transport block including
cyclic redundancy check (CRC) (i.e., :math:`B = A + 24` bits) is larger than the maximum CBS, which is 8448
bits for LDPC BG1 and 3840 bits for LDPC BG2.
If code block segmentation occurs, each transport block is split into :math:`C` code blocks of
:math:`K` bits each, and for each code block, an additional CRC sequence of :math:`L=24`
bits is appended to recover the segmentation during the decoding process.
The segmentation process takes LDPC BG selection and LDPC lifting size
into account, the complete details of which can be found in [TS38212]_, and the
same procedure has been included in the 'NR' module (as per Section 5.2.2 in TS 38.212).

**SINR compression**: In case of EESM, the mapping
function is exponential and the effective SINR for single transmission depends
on a single parameter (:math:`\beta`).
More precisely, the effective SINR for single transmission is obtained as:

:math:`SINR_{\text{eff}} = {-}\beta \ln \Big( \frac{1}{|\upsilon|}\sum_{n \in \upsilon} \exp\big({-}\frac{\text{SINR}_n}{\beta}\big)\Big)`,

where :math:`\text{SINR}_n` is the SINR value in the n-th RB, :math:`\upsilon`
is the set of allocated RBs, and :math:`\beta` is the parameter that needs to be
optimized.

In EESM, given an experimental BLER measured
in a fading channel with a specific MCS, the :math:`\beta` value (and so the mapping function)
is calibrated
such that the effective SINR of that channel approximates to the SINR that
would produce the same BLER, with the same MCS, in AWGN channel conditions.
In order to obtain the optimal mapping functions, we used the NR-compliant
link-level simulator and use a calibration technique described in [calibration-l2sm]_.
We use tapped delay line (TDP) based fading channel models recommended by 3GPP in [TR38900]_.
A collection of LOS (TDL-D) and NLOS (TDL-A) channel models ranging in delay
spread from 30 ns to 316 ns are used. For NR, SCS of 30 KHz and 60 KHz are simulated.
The details of the link-level simulator as well as the optimized :math:`\beta` values
for each MCS index in MCS Table1 and MCS Table2 are detailed in [nr-l2sm]_,
as included in the 'NR' simulator.

**Effective SINR to code BLER mapping**: Once we have the effective SINR
for the given MCS, resource allocation, and channel model,
we need SINR-BLER lookup tables to find the corresponding code BLER.
In order to obtain SINR-BLER mappings, we perform extensive simulations using our
NR-compliant link-level simulator. Such curves are included in the 'NR' simulator in form
of structures.

For each MCS (both in MCS Table1 and Table2), various resource allocation
(with varying number of RBs from 1 to 132 and varying number of OFDM symbols from 1 to 10)
are simulated. Given the resource allocation,
the corresponding value of block size, LDPC BG selection,
and LDPC lifting size can be derived. In our link-level simulator,
the block size remains below the maximum CBS (i.e., 8448 bits  for  LDPC  BG1
or 3840 bits  for  LDPC  BG2), since code block segmentation is integrated into
the proposed NR PHY abstraction model to speed up the simulation rate.

Note that the SINR-BLER curves obtained from the link-level simulator
are quantized and consider a subset of CBSs. Accordingly, in the 'NR' module,
we implement a worst case approach to determine the code BLER value by
using lower bounds of the actual CBS and effective SINR.
In the PHY abstraction for HARQ-IR, for simplicity and according to the obtained curves,
we limit the effective ECR by the lowest ECR of the MCSs that have the same modulation
order as the selected MCS index.

**Transport BLER computation**: In case there is code block segmentation, there is
a need to convert the code BLER found from the link-level simulator's lookup table
to the transport BLER for the given TBS.
The code BLERs of the :math:`C` code blocks (as determined by the code block segmentation)
are combined to get the BLER of a transport block as:

:math:`TBLER = 1- \prod_{i=1}^{C} (1-CBLER_i) \approxeq 1- (1-CBLER)^C`.

The last approximate equality is implemented in the 'NR' simulator, which
holds because code block segmentation in NR generates code blocks of roughly equal sizes.


Beamforming model
=================
The 'NR' module supports different methods: long-term covariance matrix (OptimalCovMatrixBeamforming), beam-search (CellScanBeamforming), LOS path (DirectPathBeamforming), and LOS path at gNB and quasi-omni at UE (QuasiOmniDirectPathBeamforming). OptimalCovMatrixBeamforming assumes knowledge of the channel matrix to produce the optimal transmit and receive beam. In CellScanBeamforming, a set of predefined beams is tested, and the beam-pair providing a highest average SNR is selected. For the beam-search method, our simulator supports abstraction of the beam ID through two angles (azimuth and elevation). A new interface allows you to have the beam ID available at MAC layer for scheduling purposes. DirectPathBeamforming assumes knowledge of the pointing angle in between devices, and configures transmit/receive beams pointing into the LOS path direction. QuasiOmniDirectPathBeamforming uses the LOS path for configuring gNB beams, while configures quasi-omnidirectional beamforming vectors at UEs for transmission and reception.

All methods are, as of today, ideal in the sense that no physical resources are employed to do the beam selection procedure, and as such no errors in the selection are taken into account.


HARQ
****
The NR scheduler works on a slot basis and has a dynamic nature [TS38300]_.
For example, it may assign different sets of OFDM symbols in time and RBs
in frequency for transmissions and the corresponding redundancy versions.
However, it always assigns an integer multiple of the RB consisting of 12
resource elements in frequency domain and 1 OFDM symbol in time domain.
In our module, for simplicity, we assume that retransmissions
(including the first transmission and the corresponding redundancy versions)
of the same HARQ process use the same MCS and the same number of RBs,
although the specific RBs' time/frequency positions within a slot may vary
in between the retransmissions. Also, the SINRs experienced
on each RB may vary through retransmissions. As such, HARQ affects both the PHY
and MAC layers.

The 'NR' module supports two HARQ methods: Chase Combining (HARQ-CC)
and Incremental Redundancy (HARQ-IR).

At the PHY layer, the error model has been extended to support HARQ with
retransmission combining.
Basically, it is used to evaluate the correctness of the blocks received and
includes the messaging algorithm in charge of communicating to the HARQ entity
in the scheduler the result of the combined decodifications. The EESM for
combined retransmissions
varies with the underline HARQ method, as detailed next.

**HARQ-CC:** In HARQ-CC, every retransmission contains the same coded bits
(information and coding bits). Therefore, the effective code rate (ECR)
after the q-th retransmission remains the same as after the first transmission.
In this case, the SINR values of the corresponding resources are summed across
the retransmissions, and the combined SINR values are used for EESM. After q
retransmissions, in the 'NR' simulator, the effective SINR using EESM
is computed as:

:math:`SINR_{\text{eff}} = {-}\beta \ln \Big( \frac{1}{|\omega|}\sum_{m \in \omega} \exp \big({-}\frac{1}{\beta}\sum_{j=1}^q \text{SINR}_{m,j}\big)\Big)`,

where :math:`\text{SINR}_{m,j}` is the SINR experienced by the m-th RB in the j-th
retransmission, and :math:`\omega` is the set of RBs to be combined.

**HARQ-IR:** In HARQ-IR, every retransmission contains different coded bits than
the previous one. The different retransmissions typically use a different set of
coding bits. Therefore, both the effective SINR and the ECR need to be
recomputed after each retransmission.
The ECR after q retransmissions is obtained in the 'NR' simulator as:

:math:`\text{ECR}_{\text{eff}} = \frac{X}{\sum_{j=1}^q C_j}`,

where X is the number of information bits and :math:`C_j` is the number of
coded bits in the j-th retransmission.
The effective SINR using EESM after q retransmissions is given by:

:math:`\text{SINR}_{\text{eff}} = {-}\beta \ln \Big( \frac{1}{|\omega|}\sum_{m \in \omega}\exp\big({-}\frac{ \text{SINR}_{\text{eff}}^{q{-}1}{+}\text{SINR}_{m,q}}{\beta}\big)\Big)`,

where :math:`\text{SINR}_{\text{eff}}^{q{-}1}` is the effective SINR after the previous,
i.e., (q-1)-th retransmission, :math:`\text{SINR}_{m,q}` is the SINR experienced by the m-th
RB in the q-th retransmission, and :math:`\omega` is the set of RBs.


At the MAC layer, the HARQ entity residing in the scheduler is in charge of
controlling the HARQ processes for generating new packets and managing the
retransmissions both for the DL and the UL. The scheduler collects the HARQ
feedback from gNB and UE PHY layers (respectively for UL and DL connection)
by means of the FF API primitives ``SchedUlTriggerReq`` and ``SchedUlTriggerReq``.
According to the HARQ feedback and the RLC buffers status, the scheduler generates
a set of DCIs including both retransmissions of HARQ blocks received erroneous
and new transmissions, in general, giving priority to the former.
On this matter, the scheduler has to take into consideration one constraint
when allocating the resource for HARQ retransmissions, it must use the same
modulation order of the first transmission attempt. This restriction comes from
the specification of the rate matcher in the 3GPP standard [TS38212]_, where
the algorithm fixes the modulation order for generating the different blocks
of the redundancy versions.

The 'NR' module supports multiple (20) stop and wait processes to allow continuous data flow. The model is asynchronous for both DL and UL transmissions. The transmissions, feedbacks, and retransmissions basically depend on the the processing timings, the TDD pattern, and the scheduler. We support up to 4 redundancy versions per HARQ process; after which, if combined decoding is not successful, the transport block is dropped.


MAC layer
*********
This section describes the different models supported and developed at MAC layer.


Resource allocation model: OFDMA and TDMA
=========================================
The 'NR' module supports variable TTI DL-TDMA and DL-OFDMA with a single-beam capability. In the UL direction, we support TDMA with variable TTI only. The single-beam capability for DL-OFDMA implies that only a single receive or transmit beam can be used at any given time instant. The variable TTI means that the number of allocated symbols to one user is variable, based on the scheduler allocation, and not fixed as was happening in LTE. Of course, LTE-like behaviors can be emulated through a scheduler that always assigns all the available symbols.

In OFDMA, under the single-beam capability constraint, UEs that are served by different beams cannot be scheduled at the same time. But we do not have any limitations for what regards UEs that are served by the same beam, meaning that the simulator can schedule these UEs at the same time in the frequency domain. The implementation, as it is, is compatible with radio-frequency architectures based on single-beam capability, which is one of the main requirements for operation in bands with a high center carrier frequency (mmWave bands). Secondly, it allows meeting the occupied channel bandwidth constraint in the unlicensed spectrum. Such restriction, for example, is required at the 5 GHz and 60 GHz bands. The scheduler meets the requirements by grouping UEs per beam and, within a TTI, only UEs that are served by the same gNB beam would be allowed to be scheduled for DL transmission in different RBGs.

For decoding any transmission, the UE relies on a bitmask (that is an output of the scheduler) sent through the DCI. The bitmask is of length equal to the number of RBGs, to indicate (with 1's) the RBGs assigned to the UE. This bitmask is translated into a vector of assigned RB indices at PHY. In NR, an RBG may encompass a group of 2, 4, 8, or 16 RBs [TS38214]_ Table 5.1.2.2.1-1, depending on the SCS and the operational band. a TDMA transmission will have this bitmask all set to 1, while OFDMA transmissions will have enabled only the RBG where the UE has to listen.

An implementation detail that differentiates the 'NR' module from the 'mmWave' module, among the others, is that the scheduler has to know the beam assigned by the physical layer to each UE. Two parameters, azimuth and elevation, characterize the beam in case of CellScanBeamforming. This is only valid for the beam search beamforming method (i.e., for each UE, the transmission/reception beams are selected from a set of beams or codebook).


Scheduler
=========
In the 'NR' module, we have introduced schedulers for OFDMA and TDMA-based access
with variable TTI under single-beam capability. The main output of
a scheduler functionality is a list of DCIs for a specific slot,
each of which specifies four parameters: the transmission starting
symbol, the duration (in number of symbols) and an RBG bitmask,
in which a value of 1 in the position x represents a transmission
in the RBG number x.
The current implementation of schedulers API follows the FemtoForum specification
for LTE MAC Scheduler Interface [ff-api]_ , but
can be easily extended to be compliant with different industrial interfaces.

The core class of the NR module schedulers design is ``NrMacSchedulerNs3``.
This class defines the core scheduling process and
splits the scheduling logic into the logical blocks. Additionally, it implements
the MAC schedulers API, and thus it decouples a
scheduling logic from any specific MAC API specification. These two features
facilitate and accelerate the introduction of the new
schedulers specializations, i.e., the new schedulers only need to implement a
minimum set of specific scheduling functionalities
without having to follow any specific industrial API.

The scheduling process assigns the resources for active DL and UL
flows and notifies the MAC of the scheduling decision
for the corresponding slot. Currently, since in the uplink the TDMA is used, the
scheduling for UL flows
is designed to support only TDMA scheduling. On the
other hand, the
scheduling for DL flows is designed to allow both, TDMA and OFDMA,
modes for the downlink. The scheduling functions are
delegated to subclasses to perform the allocation of symbols among beams
(if any), allocation of RBGs in time/frequency-domain
among active UEs by using specific scheduling algorithm (e.g.,
round robin, proportional fair, etc.), and finally, the construction
of corresponding DCIs/UCIs. For example, TDMA scheduling can be easily
implemented by skipping the first step of allocating symbols
among beams and by fixing the minimum number of assignable RBGs to the total
number of RBGs. To obtain true TDMA-based access with
variable TTI, it is then necessary to group allocations for the same UE in
one single DCI/UCI which is the last step.

Another important
class to be mentioned is ``NrMacSchedulerNs3Base`` which is a child class
of ``NrMacSchedulerNs3``, and represents a base
class of all schedulers in the NR module (OFDMA and TDMA). This class handles
the HARQ retransmissions for the DL and the UL.
Currently, the NR module offers the scheduling of the HARQ retransmissions
in a round robin manner.

An overview of the different phases that the OFDMA schedulers follow are:

1) BSR and CQI messages processing. The MCS is computed by the AMC model
for each user based on the CQIs for the DL or SINR measurements
for the UL data channel. The MCS and BSR of each user are stored in a
structure that will be later read to determine UE capabilities and needs.
The procedure for estimating the MCS and determining the minimum number of
RBs is common to all the OFDMA-based schedulers that we may derive.

2) Upon being triggered by the MAC layer, the scheduler prepares a slot
indication. As a first step, the total number of active flows is calculated
for both UL and DL. Then, the UL is processed, and then the DL. This
requirement comes from the fact that UL and DL have, in most cases,
different delays. This delay is defined as the number of the slots that have
to pass between the moment in which the decision is taken, and the moment that
such decision is traveling in the air. The default delay parameters are 2 slots
for DL and 4 slots for UL: therefore, UL data can be penalized by the higher delay,
and hence has to be prioritized in some way when preparing the slot. For this reason,
the scheduler is also taking UL and DL decision for the same slot in different moments.

3) The UL decisions are not considered for the slot indicated by the MAC layer,
but for a slot in the future. These involve firstly any HARQ retransmission that
should be performed, for instance when the previous transmission has been NACKed.
The requirement for retransmitting any piece of data is to have enough space (indicated
by the number of RBG). This is because, while the retransmission does not need to
start at the same symbol and RB index as the previous transmission of the same TB,
it does need the same number of RBGs and MCS, since an adaptive HARQ scheme (where
the re-transmission can be scheduled with a different MCS) is not implemented. If
all the symbols are used by the UL retransmissions, the scheduling procedure ends here.
Otherwise, UL data is scheduled, by assigning the remaining resources (or less) to the
UEs that have data to transmit. The total number of symbols reserved for UL data is
then stored internally along with the slot number to which these allocations are
referred, and the procedure for UL ends here.

4) The procedure for DL allocations is started, relative to the slot indicated by
the MAC layer. The number of symbols previously given for UL data in the current
slot has to be considered during the DL phase. Before evaluating what data can
be scheduled, that number is extracted from the internal storage, and the DL phase
can continue only if there are available symbols not used by the UL phase. If it
is the case, then, the symbols can be distributed by giving priority to the HARQ
retransmissions, and then to the new data, according to different metrics.

The base class for OFDMA schedulers is ``NrMacSchedulerOfdma``.
In the downlink, such class and its subclasses perform
OFDMA scheduling, while in the uplink they leverage some of the subclasses of
``NrMacSchedulerTdma`` class that implements TDMA scheduling.

The OFDMA scheduling in the downlink is composed of the two scheduling levels:
1) the scheduling of the symbols per beam (time-domain level), where scheduler
selects a number of consecutive OFDM symbols in a slot to assign to a specific
beam, and 2) the scheduling of RBGs per UE in a beam, where the scheduler
determines the allocation of RBGs for the OFDM symbols of the corresponding
beam (frequency-domain level).
The scheduling of the symbols per beam can be performed in a load-based or
round robin fasion. The calculation of load is based on the BSRs and the
assignment of symbols per beam is proportional to the load. In the following
level, the specific scheduling algorithm (round robin, proportional fair,
max rate) decides how RBGs are allocated among different UEs asociated to the same beam.
Multiple fairness checks can be ensured in between each level of scheduling -
the time domain and the frequency domain. For instance, a UE that already has
its needs covered by a portion of the assigned resources can free these
resources for others to use.

The NR module currently offers three specializations of the OFMA schedulers.
These specializations perform the downlink scheduling in a round robin (RR), proportional fair
(PF) and max rate (MR) manner, respectively, as explained in the following:

* RR: the available RBGs are divided evenly among UEs associated to that beam
* PF: the available RBGs are distributed among the UEs according to a PF metric that considers the actual rate (based on the CQI) elevated to :math:`\alpha` and the average rate that has been provided in the previous slots to the different UEs. Changing the α parameter changes the PF metric. For :math:`\alpha=0`, the scheduler selects the UE with the lowest average rate. For :math:`\alpha=1`, the scheduler selects the UE with the largest ratio between actual rate and average rate.
* MR: the total available RBGs are distributed among the UEs according to a maximum rate (MR) metric that considers the actual rate (based on the CQI) of the different UEs.

Each of these OFDMA schedulers is performing a load-based scheduling of
symbols per beam in time-domain for the downlink. In the uplink,
the scheduling is done by the TDMA schedulers.

The base class for TDMA schedulers is ``NrMacSchedulerTdma``.
This scheduler performs TDMA scheduling for both, the UL and the DL traffic.
The TDMA schedulers perform the scheduling only in the time-domain, i.e.,
by distributing OFDM symbols among the active UEs. 'NR' module offers three
specializations of TDMA schedulers: RR, PF, and MR, where
the scheduling criteria is the same as in the corresponding OFDMA
schedulers, while the scheduling is performed in time-domain instead of
the frequency-domain, and thus the resources being allocated are symbols instead of RBGs.


Scheduler operation
===================
In an NR system, the UL decisions for a slot are taken in a different moment than the DL decision for the same slot. In particular, since the UE must have the time to prepare the data to send, the gNB takes the UL scheduler decision in advance and then sends the UL grant taking into account these timings. Consider that the DL-DCIs are usually prepared two slots in advance with respect to when the MAC PDU is actually over the air. For the UL case, to permit two slots to the UE for preparing the data, the UL grant must be prepared four slots before the actual time in which the UE transmission is over the air. In two slots, the UL grant will be sent to the UE, and after two more slots, the gNB is expected to receive the UL data.

At PHY layer, the gNB stores all the relevant information to properly schedule reception/transmission of data in a vector of slot allocations. The vector is guaranteed to be sorted by the starting symbol, to maintain the timing order between allocations. Each allocation contains the DCI created by the MAC, as well as other useful information.


Timing relations
================
The 'NR' module supports flexible scheduling and DL HARQ Feedback timings in the
communication between the gNB and the UE as specified in [TS38213]_, [TS38214]_.
In particular, the following scheduling timings are defined:

* K0 → Delay in slots between DL DCI and corresponding DL Data reception
* K1 → Delay in slots between DL Data (PDSCH) and corresponding ACK/NACK transmission on UL
* K2 → Delay in slots between UL DCI reception in DL and UL Data (PUSCH) transmission

The values of the scheduling timings are calculated at the gNB side and result
from the processing timings that are defined in the 'NR' module as:

* N0 → minimum processing delay (in slots) needed to decode DL DCI and decode DL data (UE side)
* N1 → minimum processing delay (in slots) from the end of DL Data reception to the earliest possible start of the corresponding ACK/NACK transmission (UE side)
* N2 → minimum processing delay (in slots) needed to decode UL DCI and prepare UL data (UE side)

The values of the processing delays depend on the UE capability (1 or 2) and the configured numerology.
Typical values for N1 are 1 and 2 slots, while N2 can range from 1 to 3 slots based on the numerology and the UE capability. The processing times are defined in Table 5.3-1/2 for N1 and Table 6.4-1/2 for N2 of [TS38214]_. Although in the standard they are defined in multiples of the OFDM symbol, in the simulator we define them in multiples of slots, because then they are used to compute dynamic K values that are measured in slots. Also note that N0 is not defined in the specs, but so is K0, and so we have included both in the 'NR' module.
The values of the processing delays in the 'NR' simulator can be configured by the user through the attributes ``N0Delay``, ``N1Delay``, and ``N2Delay``, and default to 0 slots, 2 slots, and 2 slots, respectively.
Note there are not limitations in the implementation and every N value can be equal or larger than 0 (N0, N1, N2 >= 0).

For the scheduling timings let us note that each K cannot take a value smaller than
the corresponding N value (e.g., K2 cannot be less than N2).
The proceedure followed for the calculation of the scheduling and DL HARQ Feedback
timings at the gNB side is briefly described below.

For K0, the gNB calculates (based on the TDD pattern) which is the next DL (or F)
slot that follows after (minimum) N0 slots. In the current implementation we use
N0=0, as such in this case DL Data are scheduled in the same slot with the DL DCI.

For K1/K2, the gNB calculates (based on the TDD pattern) which is the next UL (or F)
slot that follows after (minimum) N1/N2 slots and calculates K1/K2 based on the
resulted slot and the current slot.

Then, the gNB communicates the scheduling timings to the UE through the DCI. In
particular, K0 and K1 are passed to the UE through the DL DCI in the time domain
resource assignment field and PDSCH-to-HARQ_feedback timing indicator, respectively,
while K2 is passed through the UL DCI in the time domain resource assignment field.
Upon reception of the DL/UL DCI, the UE extracts the values of K0/K1/K2:

* For the case of K0, UE extracts from the DL DCI its value and calculates the corresponding slot for the reception of the DL Data.
* For the case of K2, UE extracts from the UL DCI its value and calculates the corresponding slot for the transmission of its UL Data.

      For example, if UL DCI is received in slot n and K2 = 2, UE will transmit UL Data in slot (n + K2)

* For the case of K1, UE extracts from the DL DCI its value and stores it in a map based on the HARQ Process Id. This way, when the UE is going to schedule the DL HARQ feedback, it can automatically find out in which slot it will have to schedule it.


BWP manager
===========
Our implementation has a layer that acts as a 'router' of messages. Initially, it was depicted as a middle layer between the RLC and the MAC, but with time it got more functionalities. The purpose of this layer, called the bandwidth part manager, is twofold. On the first hand, as we have already seen, it is used to route the control messages to realize the FDD bandwidth part pairing. On the other hand, it is used to split or route traffic over different spectrum parts.

For the FDD pairing functionality, the user has to enter the pairing configuration that applies to his/her scenario. The NetDevice will then ask the manager for the input/output bandwidth part to which the message should be routed. It is important to note that this feature virtually connects different physical layers.

For the flows routing among different spectrum, the layer intercepts the BSR from the RLC queues, and route them to the correct stack (MAC and PHY) that is attached to a particular spectrum region. The algorithmic part of the split is separated from the Bandwidth Part Manager. In other words, the algorithm is modularized to let the user write, change, and test different ways of performing the split. The only requirement is that such routing is done based on the QCI of the flow.


Adaptive modulation and coding model
====================================
MCS selection in NR is an implementation specific procedure.
The 'NR' module supports 1) fixing the MCS to a predefined value, both for
downlink and uplink
transmissions, separately, and 2) two different AMC models for link adaptation:

* Error model-based: the MCS index is selected to meet a target transport BLER (e.g., of at most 0.1)
* Shannon-based: chooses the highest MCS that gives a spectral efficiency lower than the one provided by the Shannon rate

In the Error model-based AMC, the PHY abstraction model described in PHY layer
section is used for link adaptation, i.e.,
to determine an MCS that satisfies the target transport BLER based
on the actual channel conditions. In particular, for a given set of SINR values,
a target transport BLER, an MCS table, and considering a transport block
composed of the group of RBs in the band (termed the CSI reference resource [TS38214]_),
the highest MCS index that meets the target transport BLER constraint is selected
at the UE. Such value is then reported through the associated CQI index to the gNB.

In the Shannon-based AMC, to compute the Shannon rate we use a coefficient
of :math:`{-}\ln(5{\times} Ber)/1.5` to account for the difference in
between the theoretical bound and real performance.

The AMC model can be configured by the user through the attribute ``AmcModel``. In case
the Error model-based AMC is selected, the attribute ``ErrorModelType`` defines
the type of the Error Model that is used when AmcModel is set to ErrorModel, which takes
the same error model type as the one configured for error modeling. In case the
Shannon-based AMC is selected, the value :math:`Ber` sets the requested bit error rate
in assigning the MCS.

In the 'NR' module, link adaptation is done at the UE side, which selects the MCS index (quantized
by 5 bits), and such index is then communicated to the gNB through a CQI index (quantized by 4 bits).

Note also that, in case of adaptive MCS, in the simulator, the gNBs DL data transmissions start with MCS0. Such MCS is used at the start and until there is a UE CQI feedback.


Transport block model
=====================
The model of the MAC Transport Blocks (TBs) provided by the simulator is simplified with respect to the 3GPP specifications. In particular, a simulator-specific class (PacketBurst) is used to aggregate MAC SDUs to achieve the simulator’s equivalent of a TB, without the corresponding implementation complexity. The multiplexing of different logical channels to and from the RLC layer is performed using a dedicated packet tag (LteRadioBearerTag), which produces a functionality which is partially equivalent to that of the MAC headers specified by 3GPP. The incorporation of real MAC headers has recently started, so it is expected that in the next releases such tag will be removed. At the moment, we introduced the concept of MAC header to include the Buffer Status Report as a MAC Control Element, as it is defined by the standard (with some differences, to adapt it to the ancient LTE scheduler interface).

**Transport block size determination**: Transport block size determination in NR is described in [TS38214]_, and it is used to determine the TB size of downlink and uplink shared channels, for a given MCS table, MCS index and resource allocation (in terms of OFDM symbols and RBs). The procedure included in the 'NR' module for TB size determination follows TS 38.214 Section 5.1.3.2 (DL) and 6.1.4.2 (UL) but without including quantizations and and limits. That is, including Steps 1 and 2, but skipping Steps 3 and 4, of the NR standard procedure. This is done in this way to allow the simulator to operate in larger bandwidths that the ones permitted by the NR specification. In particular, the TB size is computed in the simulator as follows:

:math:`N_{info}= R \times Q \times n_s \times n_{rb} \times (12- n_{refSc})`,

where :math:`R` is the ECR of the selected MCS, :math:`Q` is the modulation order of the selected MCS, :math:`n_s` is the number of allocated OFDM symbols, :math:`n_{rb}` is the number of allocated RBs, and :math:`n_{refSc}` is the number of reference subcarriers carrying DMRS per RB.

After this computation, we substract the CRC attachment to the TB (24 bits), and if code block segmentation occurs, also the code block CRC attachments are substracted, to get the final TB size.


RLC layer
*********
The simulator currently reuses the RLC layer available in LENA ns-3 LTE. For details see:
https://www.nsnam.org/docs/release/3.29/models/html/lte-design.html#rlc


PDCP layer
**********
The simulator currently reuses the PDCP layer available in LENA ns-3 LTE. For details see:
https://www.nsnam.org/docs/release/3.29/models/html/lte-design.html#pdcp


SDAP layer
**********
SDAP layer is not present yet in the 'NR' module.


RRC layer
*********
The simulator currently reuses the RRC layer available in LENA ns-3 LTE. For details see:
https://www.nsnam.org/docs/release/3.29/models/html/lte-design.html#rrc


NAS layer
*********
The simulator currently reuses the NAS layer available in LENA ns-3 LTE. For details see:
https://www.nsnam.org/docs/release/3.29/models/html/lte-design.html#nas


EPC model
*********
The simulator currently reuses the core network (EPC) of LENA ns-3 LTE. For details see:
https://www.nsnam.org/docs/release/3.29/models/html/lte-design.html#epc-model


S1, S5, S11 interfaces
**********************
The simulator currently reuses the S1, S5, and S11 interfaces of LENA ns-3 LTE. For details see:
https://www.nsnam.org/docs/release/3.29/models/html/lte-design.html#s1-s5-and-s11


X2 interface
************
The simulator currently reuses the X2 interfaces of LENA ns-3 LTE. For details see:
https://www.nsnam.org/docs/release/3.29/models/html/lte-design.html#x2


NR REM Helper
*************

The purpose of the ``NrRadioEnvironmentMapHelper`` is to generate rem maps, where
for each point on the map (rem point) a rem value is calculated (SNR/SINR/IPSD).
The IPSD (Interference Power Spectral Density) corresponds to the aggregated
received power of all signals at each rem point (treated as interference).

For the nr radio environment helper we have introduced the following terminologies\:
 * RTD(s) -> rem transmitting device(s)
 * RRD -> rem receiving device

As general case, the rem point is configured according to the RRD passed to the
``NrRadioEnvironmentMapHelper`` (e.g. antenna configuration).

Two general types of maps can be generated according to whether the BeamShape
or CoverageArea is selected.
The first case considers the configuration of the beamforming vectors (for each
RTD) as defined by the user in the scenario script for which the REM maps
(SNR/SINR/IPSD) are generated. Examples are given in Figure :ref:`fig-BSiso3gpp`
where the first two figures depict the SNR (left) and SINR (right) for the case
of two gNBs with antenna array configuration 8x8 and Isotropic elements, while
the two figures on the bottom correspond to 3GPP element configuration.

.. _fig-BSexamples:

.. figure:: figures/BSiso3gpp.*
   :align: center
   :scale: 50 %

   BeamShape map examples (left: SNR, right: SINR)

In the second case, the beams are reconfigured during the map generation for
each rem point in order to visualize the coverage area in terms of SNR, SINR
and IPSD. Examples of the SNR (left) and SINR (right) CoverageArea maps for two
gNBs with Isotropic/3GPP (top/bottom) antenna elements are presented in
Figure :ref:`fig-CAiso3gpp`.

.. _fig-CAexamples:

.. figure:: figures/CAiso3gpp.*
   :align: center
   :scale: 50 %

   CoverageArea map examples (left: SNR, right: SINR)

The ``NrRadioEnvironmentMapHelper`` allows also the visualization of the coverage
holes when buildings are included in the deployment. An example is given in
Figure :ref:`fig-CAexamplesBuildings`, where Isotropic antenna elements were
configured to both gNBs of the example.

.. _fig-CAexamplesBuildings:

.. figure:: figures/CAexamplesBuildings.*
   :align: center
   :scale: 50 %

   CoverageArea map examples with buildings (left: SNR, right: SINR)

An example for a hexagonal deployment is given in Figure :ref:`fig-UmaRma`. In this
example the REM depicts a scenario for the frequency band of 2GHz, BW of 10 MHz,
while the Inter-Site Distance (ISD) has been set to 1732m for the Urban case (top)
and 7000m for the Rural case (bottom). The transmit power has been set to 43 dBm.

.. _fig-S3:

.. figure:: figures/UmaRma.*
   :align: center
   :scale: 50 %

   Hexagonal Topology (BeamShape) map examples (left: SNR, right: SINR)

Finally, Figure :ref:`fig-HetNet` presents an example of a Heterogeneous Network
(HetNet) of 7 Macro sites and 3 randomly deployed Small Cells.

.. _fig-HetNet:

.. figure:: figures/HetNet.*
   :align: center
   :scale: 75 %

   Heterogeneous Network map example (left: SNR, right: SINR)

The ``NrRadioEnvironmentMapHelper`` gives the possibility to generate maps either
for the DL or the UL direction. This can be done by passing to the rem helper
the desired transmitting device(s) (RTD(s)) and receiving device (RRD), which
for the DL case correspond to gNB(s) and UE, respectively, while for the UL
case to UE(s) and gNB, respectively. An example of an UL case is given in
Figure :ref:`fig-UlRemHex`, for the hexagonal topology presented in Figure :ref:`fig-S3`
above (Urban case), for 324 UEs with UE transmit power 23 dBm, antenna height 1.5m
and 1x1 antenna array.

.. _fig-UlRemHex:

.. figure:: figures/UlRemHex.*
   :align: center
   :scale: 50 %

   UL REM map example (IPSD)

In addition, an UL map can be generated to visualize the coverage area of a tx
device (UE), while there is the possibility to add interference from DL gNB
device(s) to study a worst case mixed FDD-TDD scenario.

Let us notice that for the SNR/SINR/IPSD calculations at each REM Point the
channel is re-created to avoid spatial and temporal dependencies among
independent REM calculations. Moreover, the calculations are the average of
N iterations (specified by the user) in order to consider the randomness of
the channel.


NR-U extension
**************
TBC


NR V2X extension
****************
TBC


Scope and Limitations
*********************
This module implements a partial set of features currently defined in the standard.
Key aspects introduced in Release-15 that are still missing are:
spatial multiplexing, configured grant scheduling and puncturing,
realistic beam management, error model for control channels.





Usage
-----

This section is principally concerned with the usage of the model, using
the public API. We discuss on examples available to the user.


Examples
********

Several example programs are provided to highlight the operation.


cttc-3gpp-channel-simple-ran.cc
===============================
The program ``nr/examples/cttc-3gpp-channel-simple-ran.cc``
allows users to select the numerology and test the performance considering
only the RAN. The scenario topology is simple, and it
consists of a single gNB and single UE. The scenario is illustrated in
Figure ::`fig-scenario-simple`.

.. _fig-scenario-simple:

.. figure:: figures/scenario-simple.*
   :align: center
   :scale: 50 %

   NR scenario for simple performance evaluation (RAN part only)

The output of the example is printed on the screen and it shows the PDCP and RLC delays.
The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-3gpp-channel-simple-ran_8cc.html


cttc-3gpp-channel-nums.cc
=========================
The program ``examples/cttc-3gpp-channel-nums.cc``
allows users to select the numerology and test the end-to-end performance.
Figure :ref:`fig-end-to-end` shows the simulation setup.
The user can run this example with UDP full buffer traffic and can specify the
UDP packet interval.

.. _fig-end-to-end:

.. figure:: figures/end-to-end.*
   :align: center

   NR end-to-end system performance evaluation

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-3gpp-channel-nums_8cc.html


cttc-3gpp-channel-simple-fdm.cc
===============================

The program ``examples/cttc-3gpp-channel-simple-fdm.cc`` can be used to
simulate FDM  of numerologies in scenario with a single UE and gNB.
In this program the packet is directly injected to the gNB, so this program
can be used only for simulation of the RAN part.
This program allows the user to configure 2 BWPs.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-3gpp-channel-simple-fdm_8cc.html

cttc-3gpp-channel-nums-fdm.cc
=============================
The program ``examples/cttc-3gpp-channel-nums-fdm.cc`` allows the user to configure
2 UEs and 1 or 2 BWPs and test the end-to-end performance.
This example is designed to expect the full configuration of each BWP.
The configuration of BWP is composed of the following parameters:
central carrier frequency, bandwidth and numerology. There are 2 UEs, and each UE has one flow.
One flow is of URLLC traffic type, while the another is eMBB.
URLLC is configured to be transmitted over the first BWP, and the eMBB over the second BWP.
Figure :ref:`fig-end-to-end` shows the simulation setup.
Note that this simulation topology is as the one used in ``scratch/cttc-3gpp-channel-nums.cc``
The user can run this example with UDP full buffer traffic or can specify the
UDP packet interval and UDP packet size per type of traffic.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-3gpp-channel-nums-fdm_8cc.html


cttc-3gpp-indoor-calibration.cc
===============================
The program ``examples/cttc-3gpp-indoor-calibration`` is the simulation
script created for the NR-MIMO Phase 1 system-level calibration.
The scenario implemented in this simulation script is according to
the topology described in 3GPP TR 38.900 V15.0.0 (2018-06) Figure 7.2-1:
"Layout of indoor office scenarios".
The simulation assumptions and the configuration parameters follow
the evaluation assumptions agreed at 3GPP TSG RAN WG1 meeting #88,
and which are summarised in R1-1703534 Table 1.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-3gpp-indoor-calibration_8cc.html


cttc-error-model.cc
===================
The program ``examples/cttc-error-model`` allows the user to test the end-to-end
performance with the new NR PHY abstraction model for error modeling by using a fixed MCS.
It allows the user to set the MCS, the MCS table, the error model type, the gNB-UE distance, and the HARQ method.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-error-model_8cc.html

cttc-error-model-comparison.cc
==============================
The program ``examples/cttc-error-model-comparison`` allows the user to compare the Transport
Block Size that is obtained for each MCS index under different error models (NR and LTE)
and different MCS Tables. It allows the user to configure the MCS Table and the
error model type.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-error-model-comparison_8cc.html


cttc-error-model-amc.cc
=======================
The program ``examples/cttc-error-model-amc`` allows the user to test the end-to-end
performance with the new NR PHY abstraction model for error modeling by using
adaptive modulation and coding (AMC).
It allows the user to set the AMC approach (error model-based or Shannon-based),
the MCS table, the error model type, the gNB-UE distance, and the HARQ method.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-error-model-amc_8cc.html


cttc-3gpp-channel-example.cc
============================
The program ``examples/cttc-3gpp-channel-example`` allows the user to setup a
simulation using the implementation of the 3GPP channel model decribed in TR 38.900.
The network topology consists, by default, of 2 UEs and 2 gNbs. The user can
select any of the typical scenarios in TR 38.900 such as Urban Macro (UMa),
Urban Micro Street-Canyon (UMi-Street-Canyon), Rural Macro (RMa) or Indoor
Hotspot (InH) in two variants: 'InH-OfficeMixed' and 'InH-OfficeOpen'. The
example also supports either mobile or static UEs.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-3gpp-channel-example_8cc.html

cttc-lte-ca-demo.cc
===================
The program ``examples/cttc-lte-ca-demo`` allows the user to setup a simulation
to test the configuration of inter-band Carrier Aggregation in an LTE deployment.
One Component Carrier (CC) is created in LTE Band 40 and two CCs are created in
LTE Band 38. The second CC in Band 38 can be configured to operate in TDD or FDD
mode; the other two CCs are fixed to TDD.  The user can provide the TDD pattern
to use in every TDD CC as input.

In this example, the deployment consists of one gNB and one UE. The UE can be
configure to transmit different traffic flows simultaneously. Each flow is mapped
to a unique CC, so the total UE traffic can be aggregated.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-lte-ca-demo_8cc.html

cttc-nr-cc-bwp-demo.cc
======================
The program ``examples/cttc-nr-cc-bwp-demo`` allows the user to setup a simulation
to test the configuration of intra-band Carrier Aggregation (CA) in an NR deployment.
The example shows how to configure the operation spectrum by definining all the
operation bands, CCs and BWPs that will
be used in the simulation. The user can select whether the creation of the spectrum
structures is automated with the CcBwpCreator helper; or if the user wants to
manually provide a more complex spectrum configuration.

In this example, the NR deployment consists of one gNB and one UE. The operation
mode is set to TDD. The user can provide a TDD pattern as input to the simulation;
otherwise the simulation will assume by default that dowlink and uplink
transmissions can occur in the same slot.

The UE can be configured to transmit three different traffic flows simultaneously.
Each flow is mapped to a unique CC, so the total UE traffic can be aggregated.
The generated traffic can be only DL, only UL, or both at the
same time. UE data transmissions will occur in the right DL or UL slot according
to the configured TDD pattern.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-nr-cc-bwp-demo_8cc.html

cttc-nr-demo.cc
===============
The program ``examples/cttc-nr-demo`` is recommended as a tutorial to the use of
the ns-3 NR module. In this example, the user can understand the basics to
successfully configure a full NR simulation with end-to-end data transmission.

Firtly, the example creates the network deployment using the GridScenario helper.
By default, the deployment consists of a single gNB and two UEs, but the user
can provide a different number of gNBs and UEs per gNB.

The operation mode is set to TDD. The user can provide a TDD pattern as input to
the simulation; otherwise the simulation will assume by default that dowlink and
uplink transmissions can occur in the same slot.

The example performs inter-band Carrier Aggregation of two CC, and each CC has one BWP occupying the whole CC bandwidth.

It is possible to set different configurations for each CC such as the numerology,
the transmission power or the TDD pattern. In addition, each gNB can also have a
different configuration of its CCs.

The UE can be configure to transmit two traffic flows simultaneously. Each flow
is mapped to a single CC, so the total UE traffic can be aggregated.

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/cttc-nr-demo_8cc.html

s3-scenario.cc
===============
The program ``examples/s3-scenario`` allows the user to run a multi-cell network deployment with site sectorization.

The deployment follows the typical hexagonal grid topology and it is composed of 21 sectorized sites. Each site has 3 sectors, with 3 antennas pointing in different directions. Sectors are equally sized, meaning that each sector covers 120º in azimuth. The deployment assumes a frequency reuse of 3, which is typical in cellular networks. This means that each sector of a site transmits in a separate frequency bands called sub-bands, so sectors of the same site do not interfere. Sub-bands are centered in different frequencies but they all have the same bandwidth. Sub-band utilization is repeated for all sites. The Inter-Site Distance (ISD) is configurable. Although, we use two possible values, one for each of the target scenarios. The two scenarios in consideration are: Urban Macro (UMa), where ISD = 500 metres; and Urban Micro (UMi), where ISD = 200 metres The choice of UMa or UMi determines the value of scenario-specific parameters, such as the height of the gNB, the transmit power, and the propagation model.

The list of simulation parameters that can be provided as input parameters in the simulation run command are defined in the example script. They include, among others, the scenario (UMa or UMi), the number of rings (0, 1, 2, 3), the number of UEs per sector, the packet size, the numerology, the TDD pattern, and the direction (DL or UL).

The complete details of the simulation script are provided in
https://cttc-lena.gitlab.io/nr/s3-scenario_8cc.html


Validation
----------

Tests
*****
To validate the implemented features, we have designed different tests.


NR test for new NR frame structure and numerologies configuration
=================================================================
Test case ``nr-system-test-configurations`` validates that the NR frame structure is correctly
configured by using new configuration parameters.
This is the system test that is validating the configuration of
different numerologies in combination with different schedulers.
The test provides the traces according to which can be checked whether
the gNB and UE clocks perform synchronously according the selected numerology,
and that serialization and deserialization of the frame, subframe, slot and TTI number
performs correctly for the new NR frame structure.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/nr-system-test-configurations_8cc.html


Test of packet delay in NR protocol stack
=========================================
Test case ``nr-test-numerology-delay`` validates that the delays of a single
UDP packet are correct.
UDP packet is monitored at different points of NR protocol stack,
at gNB and UE. The test checks whether the delay corresponds to
configuration of the system for different numerologies.

.. _fig-protocol-stack:

.. figure:: figures/protocol-stack.*
   :align: center
   :scale: 60%

   Performance evaluation of packet delay in NR protocol stack

The test monitors delays such as, gNB processing time, air time, UE time, etc.
The test fails if it detects unexpected delay in the NR protocol stack.
The test passes if all of the previous steps are according to the
timings related to a specific numerology. The test is run for different
numerologies.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/nr-test-numerology-delay_8cc.html


Test for CC/BWP
===============
Test case called ``nr-lte-cc-bwp-configuration`` validates that the creation of operation bands, CCs and BWPs is correct within the limitations of the NR implementation. The main limitation of BWPs is that they do not overlap, because in such case, the interference calculation would be erroneous. This test also proves that the creation of BWP information with the CcBwpHelper is correct.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/nr-lte-cc-bwp-configuration_8cc.html


Test of numerology FDM
======================
To test the FDM of numerologies, we have implemented the ``nr-test-fdm-of-numerologies``, in which the gNB is configured to operate with 2 BWPs. The test checks if the achieved throughput of a flow over a specific BWP is proportional to the bandwidth of the BWP through which it is multiplexed. The scenario consists of two UEs that are attached to a gNB but served through different BWPs, with UDP full buffer downlink traffic. Since the traffic is full buffer traffic, it is expected that when more bandwidth is provided, more throughput will be achieved and vice versa.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/nr-test-fdm-of-numerologies_8cc.html


Test for NR schedulers
======================
To test the NR schedulers, we have implemented various system tests called
``nr-system-test-schedulers-tdma/ofdma-mr/pf/rr`` whose purpose is to test that the
NR schedulers provide a required amount of resources to all UEs, for both cases,
the downlink and the uplink. The topology consists of a single gNB and
variable number of UEs, which are distributed among variable number of beams.
Test cases are designed in such a way that the offered rate for the flow
of each UE is dimensioned in such a way that each of the schedulers under the
selected topology shall provide at least the required service to each of the UEs.
Different system tests cases are available for the various modes of scheduling (OFDMA and TDMA) and different scheduling algorithms (RR, PR, MR) supported in the simulator. Each of the test cases checks different system configuration by choosing
different number of UEs, number of beams, numerology, traffic direction (DL, UL,
DL and UL).

The complete details of the validation scripts are provided in
https://cttc-lena.gitlab.io/nr/nr-system-test-schedulers-ofdma-mr_8cc.html,
https://cttc-lena.gitlab.io/nr/nr-system-test-schedulers-ofdma-pf_8cc.html,
https://cttc-lena.gitlab.io/nr/nr-system-test-schedulers-ofdma-rr_8cc.html,
https://cttc-lena.gitlab.io/nr/nr-system-test-schedulers-tdma-mr_8cc.html, https://cttc-lena.gitlab.io/nr/nr-system-test-schedulers-tdma-pf_8cc.html, https://cttc-lena.gitlab.io/nr/nr-system-test-schedulers-tdma-rr_8cc.html

The base class for all the scheduler tests is
https://cttc-lena.gitlab.io/nr/system-scheduler-test_8h.html


Test for NR error model
=======================
Test case called ``nr-test-l2sm-eesm`` validates specific functions of the NR
PHY abstraction model.
The test checks two issues: 1) LDPC base graph (BG) selection works properly, and 2)
BLER values are properly obtained from the BLER-SINR look up tables for different
block sizes, MCS Tables, BG types, and SINR values.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/nr-test-l2sm-eesm_8cc.html


Test for 3GPP antenna model
===========================
Test case called ``test-antenna-3gpp-model-conf`` validates multiple configurations
of the antenna array model by checking if the throughput/SINR/MCS obtained is as
expected. The test scenario consists of one gNB and a single UE attached to the
gNB. Different positions of the UE are evaluated.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/test-antenna-3gpp-model-conf_8cc.html


Test for TDD patterns
=====================
Test case called ``nr-lte-pattern-generation`` validates the maps generated from the function
``NrGnbPhy::GenerateStructuresFromPattern`` that indicate the slots that the DL/UL
DCI and DL HARQ Feedback have to be sent/generated, as well as the scheduling timings
(K0, K1, k2) that indicate the slot offset to be applied at the UE side for the reception
of DL Data, scheduling of DL HARQ Feedback and scheduling of UL Data, respectively.
The test calls ``NrGnbPhy::GenerateStructuresFromPattern`` for a number of possible
TDD patterns and compares the output with a predefined set of the expected results.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/nr-lte-pattern-generation_8cc.html

Test case called ``nr-phy-patterns`` creates a fake MAC that checks if, that
when PHY calls the DL/UL slot allocations, it does it for the right slot in pattern.
In other words, if the PHY calls the UL slot allocation for a slot that should be DL,
the test will fail.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/nr-phy-patterns_8cc.html


Test for spectrum phy
=====================
Test case called ``test-nr-spectrum-phy`` sets two times noise figure and validetes that such a setting is applied correctly to connected classes of SpectrumPhy, i.e., SpectrumModel, SpectrumValue, SpectrumChannel, etc.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/test-nr-spectrum-phy_8h.html


Test for frame/subframe/slot number
===================================
Test case called ``test-sfnsf`` is a unit-test for the frame/subframe/slot numbering, along with the numerology. The test checks that the normalized slot number equals a monotonically-increased integer, for every numerology.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/test-sfnsf_8cc.html


Test for NR timings
===================
Test case called ``test-timings`` checks the NR timings for different numerologies. The test is run for every numerology, and validates that the slot number of certain events is the same as the one pre-recorded in manually computed tables. We currently check only RAR and DL DCI messages, improvements are more than welcome.

The complete details of the validation script are provided in
https://cttc-lena.gitlab.io/nr/test-timings_8cc.html


Open issues and future work
---------------------------

.. [TR38912] 3GPP TR 38.912 "Study on New Radio (NR) access technology", (Release 14) TR 38.912v14.0.0 (2017-03), 3rd Generation Partnership Project, 2017.

.. [mmwave-module] NYU WIRELESS, University of Padova, "ns-3 module for simulating mmwave-based cellular systems," Available at https://github.com/nyuwireless/ns3-mmwave.

.. [TR38900] 3GPP TR 38.900 "Study on channel model for frequency above 6GHz", (Release 14) TR 38.912v14.0.0 (2016-12), 3rd Generation Partnership Project, 2016.

.. [end-to-end-mezz] Marco Mezzavilla, Menglei Zhang, Michele Polese, Russell Ford, Sourjya Dutta, Sundeep Rangan, Michele Zorzi, "End-to-End Simulation of 5G mmWave Networks,", in IEEE Communication Surveys and Tutorials, vol. 13, No 20,  pp. 2237-2263, April 2018.

.. [WNS32018-NR]  B. Bojovic, S. Lagen, L. Giupponi, Implementation and Evaluation of Frequency Division Multiplexing of Numerologies for 5G New Radio in ns-3 , in Workshop on ns-3, June 2018, Mangalore, India.

.. [CAMAD2018-NR] N. Patriciello, S. Lagen, L. Giupponi, B. Bojovic, 5G New Radio Numerologies and their Impact on the End-To-End Latency , in Proceedings of IEEE International Workshop on Computer-Aided Modeling Analysis and Design of Communication Links and Networks (IEEE CAMAD), 17-19 September 2018, Barcelona (Spain).

.. [3GPPTSGSSA] 3GPP TS 23.501 V15.0.0, System Architecture for the 5G System; Stage 2 (Release 15), Dec. 2017

.. [CA-WNS32017] B. Bojovic, D. Abrignani Melchiorre, M. Miozzo, L. Giupponi, N. Baldo, Towards LTE-Advanced and LTE-A Pro Network Simulations: Implementing Carrier Aggregation in LTE Module of ns-3, in Proceedings of the Workshop on ns-3, Porto, Portugal, June 2017.

.. [ff-api] FemtoForum , "LTE MAC Scheduler Interface v1.11", Document number: FF_Tech_001_v1.11 , Date issued: 12-10-2010.

.. [TS38300] 3GPP TS 38.300, TSG RAN; NR; Overall description; Stage 2 (Release 16), v16.0.0, Dec. 2019

.. [TS38214] 3GPP  TS  38.214, TSG  RAN;  NR;  Physical  layer  procedures  for  data (Release 16), v16.0.0, Dec. 2019.

.. [TS38213] 3GPP  TS  38.213, TSG  RAN;  NR;  Physical  layer  procedures  for  control (Release 16), v16.0.0, Dec. 2019.

.. [TS38212] 3GPP  TS  38.212, TSG  RAN;  NR;  Multiplexing  and  channel  coding (Release 16), v16.0.0, Dec. 2019.

.. [calibration-l2sm] A.-M. Cipriano,  R.  Visoz,  and  T.  Salzer,  "Calibration  issues  of  PHY layer  abstractions  for  wireless  broadband  systems", IEEE  Vehicular Technology Conference, Sept. 2008.

.. [nr-l2sm] S. Lagen, K. Wanuga, H. Elkotby, S. Goyal, N. Patriciello, L. Giupponi, "New Radio Physical Layer Abstraction for System-Level Simulations of 5G Networks", in Proceedings of IEEE International Conference on Communications (IEEE ICC), 7-11 June 2020, Dublin (Ireland).

.. [baldo2009] N. Baldo and M. Miozzo, "Spectrum-aware Channel and PHY layer modeling for ns3", Proceedings of ICST NSTools 2009, Pisa, Italy.
