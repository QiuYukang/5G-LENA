Example Module Documentation
----------------------------

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)



Introduction
------------
The 3rd Generation Partnership Project (3GPP) has devoted significant efforts to standardize the
fifth generation (5G) New Radio (NR) access technology [TS38300]_,
which is designed to be extremely flexible from its physical layer definition and up to the architecture,
in order to be able to work in a wide range of frequency bands and address many different use cases
and deployment options.

As the NR specification is developed and evolves,
a network simulator that is capable of simulating emerging NR
features is of great interest for both, scientific and industrial communities.
In recent years a lot effort has been done by New York University (NYU) Wireless and University of Padova to develop a simulator
that will allow simulations of communications in millimeter-wave (mmWave) bands,
which represent a central technology of future 5G cellular wireless systems,
such as NR, public safety and vehicular communications.
Hence, a new mmWave simulation tool has been developed as a new module of |ns3|.
A complete description of the mmWave module is provided in [end-to-end-mezz]_.
The mmWave module source code is still not part of the standard ns-3 distribution and
is available at a different repository [mmwave-module]_.
The 'mmWave' module implements a complete 3GPP protocol, where the physical (PHY) layer and
medium access control (MAC) layer are custom implementations
developed to support a new mmWave-based channel, propagation, beamforming and antenna models;
and the MAC layer to support Time Division Duplexing (TDD),
Time Division Multiple Access (TDMA) MAC scheduling, and
enhanced Hybrid Automatic Repeat and reQuest (HARQ) for low latency applications.
The higher layers are mostly based on |ns3| 'LTE' module functionalities, thus still
following 3GPP LTE specifications,
but extending it to involve some of advanced features that are expected to emerge in 5G networks,
such as dual connectivity and low latency radio link control (RLC) layer.


In this document we describe the implementation that we have initiated to generate
a 3GPP-compliant NR module able to
provide |ns3| simulation capabilities in the bands above and below 6 GHz,
aligned with 3GPP NR Release-15, following the description in [TS38300]_.
The work has been initially
funded by InterDigital Communications Inc, and continues with funding from the
Lawrence Livermore National Lab (LLNL) and
a grant from the National Institute of Standards and Technologies (NIST).


The module, as already mentioned, is built upon the 'mmwave' module developed by the NYU Wireless
and University of Padova, which is available in [mmwave-module]_. The 'mmwave' module
is developed as a new module within |ns3| and
it leverages on the well known and extensively used LTE/EPC network simulator
provided by the 'LTE' module in |ns3|.

The NR module is built upon the 'mmwave' simulator, but is focused to
target the 3GPP Release-15 NR specification. As such. it
incorporates fundamental PHY-MAC NR features like a flexible frame structure by means of
multiple numerologies support, bandwidth parts (BWPs) and Component Carriers (CCs),
Frequency Division Multiplexing (FDM) of
numerologies, Orthogonal Frequency-Division Multiple Access (OFDMA),
flexible time- and frequency- resource allocation and scheduling,
Low-Density Parity Check (LDPC) coding for data channels,
modulation and coding schemes (MCSs) with up to 256-QAM, and dynamic TDD, among others.
The NR module still relies on higher layers and core network (RLC, PDCP, RRC, NAS, EPC)
based on |ns3| 'LTE' module, thus providing an NR non-standalone (NSA) implementation.

The source code for the NR module lives currently in the directory ``src/nr``.

This document describes the models available in the 'NR' module, including also the models
from 'mmwave' and 'lte' modules that are utilized in 'NR' module.

Over time, extensions found in this module may migrate to the existing |ns3| main development tree.

The rest of this document is organized into five major chapters:

2. **Design:**  Describes the models developed for |ns3| extension to support NR
   features and procedures.
3. **Usage:**  Documents how users may run and extend the NR test scenarios.
4. **Validation:**  Documents how the models and scenarios have been verified
   and validated by test programs.
5. **Open Issues and Future Work:**  Describes topics for which future work
   on model or scenario enhancements is recommended, or for which questions
   on interpretations of standards documents may be listed.


Design
------
In this section, we present the design of the different features and procedures that we have
developed following 3GPP Release-15 NR activity.
For those features/mechanisms/layers that still have not been upgraded to NR,
the current design following LTE specifications is also indicated.


Architecture
************
The 'NR' module has been designed to perform end-to-end simulations of
3GPP-oriented cellular networks. The end-to-end overview of a typical simulation
with the 'NR' module is drawn in Figure :ref:`fig-e2e`. In dark gray,
we represent the existing, and unmodified, ns-3 and LENA components.
In light gray, we represent the NR components.
On one side, we have a remote host (depicted as a single node in the figure,
for simplicity, but there can be multiple nodes)
that connects to an SGW/PGW (Service Gateway and Packet Gateway), through a link. Such a connection can be of any
technology that is currently available in ns-3. It is currently implemented
through a single link, but it can be replaced by an entire subnetwork with many
nodes and routing rules. Inside the SGW/PGW, the ``EpcSgwPgwApp`` encapsulates
the packet using the GTP protocol. Through an IP connection, which represents
the backhaul of the NR network (again, represented with a single link in the figure,
but the topology can vary), the GTP packet is received by the gNB.
There, after decapsulating the payload, the packet is transmitted over the RAN
through the entry point represented by the class ``NRGnbNetDevice``. The packet,
if received correctly at the UE, is passed to higher layers by the class ``NRUeNetDevice``.
The path crossed by packets in the UL case is the same as the one described
above but on the contrary direction.

.. _fig-e2e:

.. figure:: figures/drawing2.*
   :align: center
   :scale: 80 %

   End-to-end class overview

Concerning the RAN, we detail what is happening between ``NRGnbNetDevice`` and
``NRUeNetDevice`` in Figure :ref:`fig-ran`. The ``NRGnbMac`` and ``NRUeMac`` MAC
classes implement the LTE module SAP provider and user interfaces,
enabling the communication with the LTE RLC layer. The module supports
RLC TM, SM, UM and AM modes. The MAC layer contains the scheduler (``NRMacScheduler``
and derived classes). Every scheduler also implements a SAP for LTE RRC layer
configuration (``LteEnbRrc``). The ``NRPhy`` classes are used to perform
the directional communication for both downlink (DL) and uplink (UL), to transmit/receive the data
and control channels. Each ``NRPhy`` class writes into an instance of ``MmWaveSpectrumPhy``
class, which is shared between the UL and DL parts.

.. _fig-ran:

.. figure:: figures/drawing1.*
   :align: center
   :scale: 80 %

   RAN class overview

Interesting blocks in Figure :ref:`fig-ran` are the ``NRGnbBwpM`` and ``NRUeBwpM`` layers.
3GPP does not explicitly define them, and as such, they are virtual layers,
but they help construct a fundamental feature of our simulator:
the multiplexing of different parts of the bandwidth (like BWPs or CCs).
NR has included the definition of BWP for energy-saving purposes,
as well as to multiplex a variety of services with different QoS requirements. CC concept
was already introduced in LTE, and persists in NR, as a way to aggregate different
carriers and so improve the system capacity.
In the 'NR' simulator, it is possible to divide the entire bandwidth into different
BWPs and CCs. Each BWP/CC can have its own PHY and MAC configuration
(e.g., a specific numerology, scheduler rationale, and so on).
We added the possibility for any node to transmit and receive flows
in different BWPs, by either assigning each bearer to a specific BWP
or distributing the data flow among different CCs, according to the rules of the manager.
The introduction of a proxy layer to multiplex and demultiplex the data
was necessary to glue everything together, and this is the purpose
of these two new classes (``NRGnbBwpM`` and ``NRUeBwpM``).


PHY layer
*********
This section describes the different models supported and developed at PHY layer.

Frame structure model
=====================
In NR, the 'numerology' concept is introduced to flexibly define the frame structure,
in such a way that it can work in both sub-6 GHz and mmWave bands.
The flexible frame structure is defined by multiple numerologies formed by scaling
the basic subcarrier spacing (SCS) of 15 KHz. The supported numerologies (0, 1, 2, 3, 4)  correspond
to SCSs of 15 KHz, 30 KHz, 60 KHz, 120 KHz, and 240 KHz. 480 KHz is under study.
Not all SCS options are supported in all carrier frequencies and channels. For example,
for sub 6 GHz,
only 15 KHz, 30 KHz, and 60 KHz are defined. Above 6 GHz the supported ones are 60 KHz,
120 KHz and 240 KHz. Also, for numerology 2 (i.e., SCS = 60 KHz), two cyclic prefix (CP) overheads
are considered: normal and extended. For the rest of numerologies,  normal CP is defined.

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


In the time domain, each frame of length of 10 ms is split in time into 10 subframes, each
of duration of 1 ms. Every subframe is split in time into a variable number of slots, and each
slot is composed of a fixed number of OFDM symbols.
In particular, the length of the slot and the number of slots per subframe depend
on the numerology, and the length
of the OFDM symbol varies according to the numerology and CP.
The number of OFDM symbols per slot is fixed to 14 symbols for normal CP, and to
12 OFDM symbols for extended CP.

In frequency domain, the number of subcarriers per physical resource block (PRB)
is fixed to 12, and the maximum number of PRBs according to Release-15 is 275.
The numerology defines also the size of a PRB, and the total number of PRBs of the NR system
within a fixed bandwidth. PRBs are grouped into PRB groups at MAC scheduling time.

Figure :ref:`fig-frame` shows the NR frame structure in time- and
frequency- domains for numerology 3 with normal CP and a total
channel bandwidth of 400 MHz.

.. _fig-frame:

.. figure:: figures/numerologies-1.*
   :align: center
   :scale: 35 %

   NR frame structure example

The implementation in the 'NR' module currently supports the NR frame structures and numerologies
shown in Table :ref:`tab-numerologies`. This corresponds to all the numerologies defined in NR Release-15
with normal CP, plus numerology 5 (not yet supported in NR, but likely to be included
in future releases).
In the simulator, the numerology is specified by an attribute.
Once the numerology is configured, the lengths of the symbol,
the slot, the SCS, the number of PRBs within the bandwidth, and the number of slots per subframe,
are dynamically determined in a runtime, based on Table :ref:`tab-numerologies`.

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

In the 'NR' module, to support a realistic NR simulation, we properly model (as per the standard)
the numerology-dependent slot and OFDM symbol granularity. This affects different parts
of the simulator: the PHY transmission/reception
functionality, the MAC
scheduling and the resource allocation information, the processing delays,
and the interaction of the PHY layer with the MAC layer.
First, the transmission and the reception are per slot level, and are
handled by the corresponding functions. These functions are executed with a
fixed periodicity, i.e., every 14 OFDM symbols,
but the real duration of the slot depends on the configured numerology.
Second, the scheduling operation is done on a slot basis, and the scheduler
assigns transmission time intervals (TTIs) no longer than that of one slot.
Third, the MAC-to-PHY processing delay depends on the numerology, and defaults to 2 slots.
See implementation details and evaluations in [WNS32018-NR]_, [CAMAD2018-NR]_.

FDM of numerologies
===================
An additional level of flexibility in the NR system can be achieved by
implementing the multiplexing of numerologies in the frequency domain.
As an example, ultra-reliable and low-latency communications (URLLC) traffic
requires a short slot length to meet strict latency requirements, while
enhanced mobile nroadband (eMBB) use case in general aims at increasing
throughput, which is achieved with a large slot length.
Therefore, among the set of supported numerologies for a specific operational
band and deployment configuration, URLLC can be served with the numerology
that has the shortest slot length, and eMBB with the numerology associated
to the largest slot length. To address that, NR enables FDM of numerologies through different
BWPs, to address the trade-off between latency
and throughput for different types of traffic by physically dividing the
bandwidth in two or more BWPs. In Figure :ref:`fig-bwp`, we illustrate
an example of FDM of numerologies. The channel is split into two BWPs
that accommodate the two numerologies multiplexed in
frequency domain. The total bandwidth :math:`B` is then divided into two parts
of bandwidth :math:`B_u` for URLLC and :math:`B_e` for eMBB, so that :math:`B_u+B_e \le B`.

.. _fig-bwp:

.. figure:: figures/bwp.*
   :align: center
   :scale: 80 %

   FDM of numerologies example

In the 'NR' module, the user can configure FDM bands statically before the
simulation starts. This is a critical design assumption based on two main reasons.
First, the 'NR' module relies on the channel and the propagation loss model
that is not able to allow runtime modifications
of the physical configuration parameters related to time/frequency configuration
(such as the system bandwidth, the central carrier frequency, and the symbol length).
Thus, until the current channel model is not modified to allow
these runtime configuration changes, it will not be possible to perform
semi-static reconfiguration of BWPs. The second reason is that in the simulator
the RRC messaging to configure the default bandwidth part, as well as the
bandwidth part reconfiguration, are not implemented yet.
See implementation details and evaluations in [WNS32018-NR]_, which is inspired in [CA-WNS32017]_.


Duplexing schemes
=================
The 'NR' simulator supports both TDD and FDD duplexing modes, in a flexible manner.
Indeed a gNB can be configured with multiple carriers, some of them being paired (for FDD), and
others being TDD. Each carrier can be further split into multiple BWPs, under the
assumption that all the BWPs are orthogonal in frequency, to enable compatibility
with the channel instances. The gNB can simultaneously transmit and/or receive from
multiple BWPs. However, from the UE side, we assume the UE is active in a single
BWP at a time.


TDD model
#########
In case of TDD, NR allows different slot types: DL-only ("DL" slots), UL-only ("UL"
slots), and Flexible ("F" slots).
The TDD pattern in NR, which is repeated with a pre-configured periodicity, is composed of a
set of consecutive DL-only slots, a set of consecutive DL symbols, a guard band, a set of
consecutive UL symbols,
and a set of consecutive UL-only slots. In case the set of DL symbols, guard band, and UL symbols
fit in a slot, this slot is a Flexible one.

In the 'NR' module, we let the user configure the TDD pattern by specifying
the type of slot for each slot within the pattern, with a length that can be set
by the end user. In case of Flexible slots,
the first and the last OFDM symbols of the slot are reserved for
DL CTRL and UL CTRL, repectively (e.g., DCI and UCI). The symbols in between can be
dynamically allocated to DL and/or UL data. Thus, dynamic TDD is supported.
In the case of DL-only slots, the first
symbol is reserved for DL CTRL and the rest of symbols are available for DL data.
In the case of UL-only slots, the last
symbol is reserved for UL CTRL and the rest of symbols are available for UL data.

Also, the model supports configuring special slots ("S" slots) in order to emulate
LTE. In those slots, the first symbol is reserved for DL CTRL, the
last slot is reserved for UL CTRL, and the rest of symbols are available for DL data.


FDD model
#########
TBC

MAC to channel delay
====================
TBC


CQI feedback
============
TBC


Interference model
==================
The PHY model is based on the well-known Gaussian interference models, according
to which the powers of interfering signals (in linear units) are summed up together
to determine the overall interference power.
The useful and interfering signals, as well as the noise power spectral density,
are processed to calculate the SNR, the SINR, and the RSSI (in dBm).

Also, such powers are used to determine if the channel is busy or empty. For that,
we are creating two events, one that adds, for any signal, the received power and
another that substracts the received power at the end time. These events are then
used to determine if the channel is busy (by comparing to a threshold)
and for how long.


Spectrum model
==============
In the simulator, radio spectrum usage is modeled as follows. A Spectrum Model is defined in the
simulation for each set of central carrier frequency, transmission bandwidth (in number
of PRBs), subcarrier spacing, and number of subcarriers per PRB, using the
Spectrum framework [baldo2009]_. A spectrum model is configured for every bandwidth part (BWP)
of each gNB instantiated in the simulation; hence, each gNB-BWP can use a different spectrum model.
Every UE will automatically use the spectrum model of the gNB it is attached to.
Using the MultiModelSpectrumChannel described in [baldo2009]_, the interference among gNBs and UEs
that use different spectrum models is properly accounted for. This allows to
simulate dynamic spectrum access policies, as well as dynamic TDD schemes, considering
downlink-to-uplink and uplink-to-downlink interferences.


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
the corresponding section. The MCS table and
the HARQ method are two attributes that can be configured by the user.

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
conditions [TS38212]_. Assuming :math:`R` as the ECR of the selected MCS
and :math:`A` as the TBS (in bits), then,

* LDPC base graph 2 (BG2) is selected if :math:`A \le 292` with any value of :math:`R`, or if :math:`R\le 0.25` with any value of :math:`A`, or if :math:`A \le 3824` with :math:`R \le 0.67`,
* otherwise, the LDPC base graph 1 (BG1) is selected.


**Code block segmentation**: Code block segmentation for LDPC coding in
NR occurs when the number of total bits in a transport block including
cyclic redundancy check (CRC) is larger than the maximum CBS, which is 8448
bits for LDPC BG1 and 3840 bits for LDPC BG2.
If code block segmentation occurs, each transport block is split into :math:`C` code blocks of
:math:`K` bits each, and for each code block, an additional CRC sequence of :math:`L=24`
bits is appended to recover the segmentation during the decoding process.
The segmentation process takes LDPC BG selection and LDPC lifting size
into account, the complete details of which can be found in [TS38212]_, and the
same procedure has been included in the 'NR' module.


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
The 'NR' module supports two methods: long-term covariance matrix and beam-search.
The former assumes knowledge of the channel matrix to produce the optimal transmit
and receive beam.
In the later, a set of predefined beams is tested, and the beam-pair providing a
highest average SNR is selected. For the beam-search method, our simulator supports
abstraction of the beam ID through two angles (azimuth and elevation).
A new interface allows you to have the beam ID available at MAC layer for scheduling purposes.

Both methods are, as of today, ideal in the sense that no physical resources are employed
to do the beam selection procedure, and as such no errors in the selection are taken into
account.




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
on each RB may vary through retransmissions. As such, HARQ affects both the PHY and MAC layers.

The 'NR' module supports two HARQ methods: Chase Combining (HARQ-CC)
and Incremental Redundancy (HARQ-IR), which can be selected by the user through an attribute.

At the PHY layer, the error model has been extended to support HARQ with retransmission combining.
Basically, it is used to evaluate the correctness of the blocks received and
includes the messaging algorithm in charge of communicating to the HARQ entity
in the scheduler the result of the combined decodifications. The EESM for combined retransmissions
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




MAC layer
*********
This section describes the different models supported and developed at MAC layer.


Resource allocation model: OFDMA and TDMA
=========================================
The 'NR' module supports TDMA and OFDMA with single-beam capability and variable TTI
in the DL direction. In the UL direction, only TDMA with variable
TTI is supported (and so, single-beam capability by definition).
The single-beam capability implies that a single receive or transmit beam can be used
at any given time instant. The variable TTI means that the number of allocated
symbols to one user (either to receive or transmit) is variable, based on the scheduler
allocation.

The implementation of OFDMA under the single-beam capability constraint
means that frequency-domain multiplexing of
different UEs is allowed among UEs associated to the same beam. So, in one OFDM
symbol, or group of OFDM symbols, UEs that
are attached to the same gNB beam can be scheduled in different RBGs, but not are UEs attached
to different beams. This is motivated by
two main reasons. First, it is compatible with radio-frequency architectures based
on single-beam capability, which is one of
the main requirements for operation in bands with a high centre carrier frequency
(mmWave bands). Second, it allows meeting the
occupied channel bandwidth constraint in the unlicensed spectrum, e.g., that is
required at the 5 GHz and 60 GHz bands, for any
scheduling decision under the aforementioned constraint, since the scheduler will
group UEs per beam and, within a TTI, only
UEs that are attached to the same gNB beam would be allowed to be scheduled
for DL transmission in different RBGs.

The implementation of OFDMA with variable TTI is motivated by the NR specifications,
which encompass slot- and mini-slot-based
transmissions, and thus a variable TTI length composed by a flexible number of
symbols may be encountered.

To account for OFDMA, the code relies on a bitmask per UE that is an output of
the scheduler and then introduced in the DCI to
enable correct decoding at the receiver. The bitmask is used at MAC level, and
it is a vector of 0s and 1s, of length equal to
the number of RBGs, to indicate the RBGs assigned to a particular UE. This bitmask
is translated into a vector of assigned RB
indeces at PHY, of variable length, at most the number of available RBs, for
compatibility issues with PHY layer functions. In
NR, a RBG may encompass a group of 2, 4, 8, or 16 RBs [TS38214]_ Table 5.1.2.2.1-1,
depending on the SCS and the operational
band. This is a configuration parameter. In case the number of RBs in a RBG equals
to the number of RBs in the whole channel
bandwidth, then one can properly configure a TDMA-based channel access with variable TTI.

The implementation of an OFDMA-based access in the 'NR' module affects the scheduler,
HARQ operation, AMC model, temporal evolution,
interference computation at PHY, and packet burst generation. Schedulers for OFDMA are
detailed in next section. For OFDMA with variable TTI,
all the temporal references are such that enable a correct PHY behaviour
both at gNBs and UEs. That is, the reference
is not based on the transmitted TBs per UE (as in the
'mmWave' module, for which each symbol was assigned at most to a single UE),
but on the TTIs assigned on different beams. Note that
now, in the 'NR' module, within one symbol multiple UEs may be scheduled
in different RBGs, so that the TBs do not indicate the
temporal reference. This affects the timing update at the devices.
Finally the packet burst is generated in a way
such that it may include different UEs in a TTI.

Due to the OFDMA with single-beam capability, the 'NR' module has a
PHY-MAC interface to enable communication to the MAC entity
of the beam ID, which is computed at PHY, and thus allow the OFDMA with variable TTI
scheduler at MAC to consider this
information. This is done through the corresponding interfaces, to obtain
the beam ID of a UE, and change it.
The beam ID is characterized by two parameters, azimuth and elevation, and
it is only valid for the beam seach beamforming method (i.e.,
for each UE, the transmission/reception beams are selected from a set of beams or codebook).



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

The core class of the NR module schedulers design is ``MmWaveMacSchedulerNs3``.
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
class to be mentioned is ``MmWaveMacSchedulerNs3Base`` which is a child class
of ``MmWaveMacSchedulerNs3``, and represents a base
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

The base class for OFDMA schedulers is ``MmWaveMacSchedulerOfdma``.
In the downlink, such class and its subclasses perform
OFDMA scheduling, while in the uplink they leverage some of the subclasses of
``MmWaveMacSchedulerTdma`` class that implements TDMA scheduling.

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

The base class for TDMA schedulers is ``MmWaveMacSchedulerTdma``.
This scheduler performs TDMA scheduling for both, the UL and the DL traffic.
The TDMA schedulers perform the scheduling only in the time-domain, i.e.,
by distributing OFDM symbols among the active UEs. 'NR' module offers three
specializations of TDMA schedulers: RR, PF, and MR, where
the scheduling criteria is the same as in the corresponding OFDMA
schedulers, while the scheduling is performed in time-domain instead of
the frequency-domain, and thus the resources being allocated are symbols instead of RBGs.



BWP manager
===========
TBC


Adaptive modulation and coding model
====================================
MCS selection in NR is an implementation specific procedure.
However, NR defines the Channel Quality Indicator (CQI), which is reported by
the UE and can be used for MCS index selection at the gNB.
NR defines three tables of 4-bit CQIs (see Tables 5.2.2.1-1 to 5.2.2.1-3 in [TS38214]_),
each table being associated with one MCS table.


The 'NR' module supports 1) fixing the MCS to a predefined value, both for downlink and uplink
transmissions, separately, and 2) two different AMC models for link adaptation:

* Error model-based: in which the MCS index is selected to meet a target transport BLER (e.g., of at most 0.1)
* Shannon-based: which chooses the highest MCS that gives a spectral efficiency lower than the one provided by the Shannon rate

In the Error model-based AMC, the PHY abstraction model described in PHY layer section is used
for link adaptation, i.e.,
to determine an MCS that satisfies the target transport BLER based
on the actual channel conditions. In particular, for a given set of SINR values,
a target transport BLER, an MCS table, and considering a transport block
composed of the group of RBs in the band (termed the CSI reference resource [TS38214]_),
the highest MCS index that meets the target transport BLER constraint is selected
at the UE. Such value is then reported through the associated CQI index to the gNB.

In the Shannon-based AMC, to compute the Shannon rate we use a coefficient
of :math:`{-}\ln(5{\times}0.00001)/0.5` to account for the difference in
between the theoretical bound and real performance.

The AMC model can be configured by the user through the associated attribute.

In the 'NR' module, link adaptation is done at the UE side, which selects the MCS index (quantized
by 5 bits),
and such index is then communicated to the gNB through a CQI index (quantized by 4 bits).



Transport block model
=====================
The model of the MAC Transport Blocks (TBs) provided by the simulator
is simplified with respect to the 3GPP specifications. In particular,
a simulator-specific class (PacketBurst) is used to aggregate MAC SDUs in
order to achieve the simulator’s equivalent of a TB, without the corresponding
implementation complexity. The multiplexing of different logical channels
to and from the RLC layer is performed using a dedicated packet tag (LteRadioBearerTag),
which performs a functionality which is partially equivalent to that of the MAC headers
specified by 3GPP.


Delay for UL data
=================
In an NR system, the UL decisions for a slot are taken in a different moment
than the DL decision for the same slot. In particular, since the UE must have
the time to prepare the data to send, the gNB takes the UL scheduler decision
in advance and then sends the UL grant taking into account these timings.
For example, consider that the DCIs for DL are usually prepared two slots in
advance with respect to when the MAC PDU is actually over the air. For example,
for UL, the UL grant must be prepared four slots before the actual time in
which the UE transmission is over the air transmission: after two slots,
the UL grant will be sent to the UE, and after two more slots, the gNB is
expected to receive the UL data. Please note that latter examples consider
default values for MAC to PHY processing delays at gNB and UE, which are in 'NR'
module set to 2 slots. The processing delays are parameters of the simulator
that may be configured through corresponding attributes.

At PHY layer, the gNB stores all the relevant information to properly schedule
reception/transmission of data in a vector of slot allocations. The vector
is guaranteed to be sorted by the starting symbol, to maintain the timing
order between allocations. Each allocation contains the DCI created by the
MAC, as well as other useful information.

To accommodate the NR UL scheduling delay, the new MAC scheduler design is
actively considering these delays during each phase.



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
The program ``mmwave/examples/cttc-3gpp-channel-simple-ran.cc``
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
The program ``examples/cttc-error-model`` allows the user to test the end-to-end performance
with the new NR PHY abstraction model for error modeling by using a fixed MCS.
It allows the user to set the MCS, the gNB-UE distance, and either HARQ-CC or HARQ-IR.


cttc-error-model-comparison.cc
==============================
The program ``examples/cttc-error-model-comparison`` allows the user to compare the Transport
Block Size that is obtained for each MCS index under different error models (NR and LTE)
and different MCS Tables.


cttc-error-model-amc.cc
=======================
The program ``examples/cttc-error-model-amc`` allows the user to test the end-to-end performance
with the new NR PHY abstraction model for error modeling by using adaptive modulation and
coding (AMC).
It allows the user to set the AMC approach (error model-based or Shannon-based),
the gNB-UE distance, and either HARQ-CC or HARQ-IR.




Validation
----------

Tests
*****
To validate the implemented features, we have designed different tests.


NR test for new NR frame structure and numerologies configuration
=================================================================
Test case ``mmwave-system-test-configurations`` validates that the NR frame structure is correctly
configured by using new configuration parameters.
This is the system test that is validating the configuration of
different numerologies in combination with different schedulers.
The test provides the traces according to which can be checked whether
the gNB and UE clocks perform synchronously according the selected numerology,
and that serialization and deserialization of the frame, subframe, slot and TTI number
performs correctly for the new NR frame structure.


Test of packet delay in NR protocol stack
=========================================
Test case ``mmwave-test-numerology-delay`` validates that the delays of a single
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


Test of numerology FDM
======================
To test the FDM of numerologies, we have implemented
the ``MmWaveTestFdmOfNumerologiesTestSuite``, in which the gNB is configured to operate with
2 BWPs. The test checks if the achieved throughput of a flow over a specific BWP is proportional to the
bandwidth of the BWP through which it is multiplexed.


Test for NR schedulers
======================
To test the NR schedulers, we have implemented a system test called ``MmWaveSystemTestSchedulers`` whose purpose is to test that the
NR schedulers provide a required amount of resources to all UEs, for both cases, the downlink and the uplink. The topology consists of a single gNB and
variable number of UEs, which are distributed among variable number of beams. Test cases are designed in such a way that the offered rate for the flow
of each UE is dimensioned in such a way that each of the schedulers under the selected topology shall provide at least the required service to each of the UEs.
The system test suite for NR schedulers creates a various number of test cases that check different system configuration by choosing
different number of UEs, number of beams, numerology, traffic direction (DL, UL, DL and UL), modes of scheduling (OFDMA and TDMA) and
different scheduling algorithms (RR, PR, MR).


Test for OFDMA
==============
Test case called ``MmWaveSystemTestOfdma`` validates that the NR scheduling
in OFDMA mode provides expected interference situations in the scenario.
The test consists of a simple topology where 2 gNBs transmit to 2 UEs, and
these two simultaneous transmissions are on the same transmission directions,
the opposite transmission directions or the perpendicular transmission directions.
Test configures a new simple test scheduler to use only a limited portion of RBGs
for scheduling. Test case where assigned RBGs for gNBs overlap is
expected to affect the MCS and performance, while in the case when RBGs assigned
to gNBs are not overlapping shall be the same as in the case when only a
single gNB is transmitting.


Test for error model
====================
Test case called ``NrL2smEesmTestCase`` validates specific functions of the NR PHY abstraction model.
The test checks two issues: 1) LDPC base graph (BG) selection works properly, and 2)
BLER values are properly obtained from the BLER-SINR look up tables for different block sizes, MCS
Tables, BG types, and SINR values.


Test for channel model
======================
Test case called ``NrTest3gppChannelTestCase`` validates the channel model. TBC


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

.. [TS38212] 3GPP  TS  38.212, TSG  RAN;  NR;  Multiplexing  and  channel  coding (Release 16), v16.0.0, Dec. 2019.

.. [calibration-l2sm] A.-M. Cipriano,  R.  Visoz,  and  T.  Salzer,  "Calibration  issues  of  PHY layer  abstractions  for  wireless  broadband  systems", IEEE  Vehicular Technology Conference, Sept. 2008.

.. [nr-l2sm] S. Lagen, K. Wanuga, H. Elkotby, S. Goyal, N. Patriciello, L. Giupponi, "New Radio Physical Layer Abstraction for System-Level Simulations of 5G Networks", in Proceedings of IEEE International Conference on Communications (IEEE ICC), 7-11 June 2020, Dublin (Ireland).

.. [baldo2009] N. Baldo and M. Miozzo, "Spectrum-aware Channel and PHY layer modeling for ns3", Proceedings of ICST NSTools 2009, Pisa, Italy.
