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
3rd Generation Partnership Project (3GPP) is devoting significant efforts to define the 
fifth generation (5G) New Radio (NR) access technology [TR38912]_, 
which is expected to be extremely flexible from its physical layer definition and up to the architecture, 
in order to be able to work in a wide range of bands and address many different use cases. 

Since the standardization of NR access technology is currently under development, 
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
The 'mmWave' module implements a complete 3GPP protocol, where the physical (PHY) layer and MAC layers are custom implementations 
developed to support a new mmWave-based channel, propagation, beamforming and antenna models; 
and the medium access control (MAC) layer to support time division duplex (TDD), 
time division multiple access (TDMA) MAC scheduling, 
enhanced hybrid automatic repeat request (HARQ) for low latency applications.
The higher layers are mostly based on |ns3| 'LTE' module functionalities, thus still following 3GPP LTE specifications, 
but extending it to involve some of advanced features that are expected to emerge in 5G networks, 
such as dual connectivity and low latency radio link control (RLC) layer.


In this document we describe the implementation that we have initiated to generate a 3GPP compliant New Radio (NR) module able to provide |ns3| simulation capabilities in the bands above and below 6 GHz, following the description in [TR38912]_. The work has been funded by InterDigital Communications Inc.


The module, as already mentioned, is built upon the 'mmwave' module developed by the NYU Wireless 
and University of Padova, which is available in [mmwave-module]_. The 'mmwave' module is developed as a new module within |ns3| and it leverages on the well known and extensively used LTE/EPC network simulator provided by the 'LTE' module in |ns3|. 

The NR module is built upon the 'mmwave' simulator, but is more focused to target new 3GPP NR specs and so it currently targets the following additional features: Formal definition of numerologies, Frequency Division Multiplexing (FDM) of 
numerologies, Orthogonal Frequency-Division Multiple Access (OFDMA), and scheduling.
Additionally, some feature of 'mmwave' module shall be adapted 
to follow 3GPP standard, as for example the proposed frame structure.

The source code for the NR module lives currently in the directory ``src/mmwave``. 

This document primarily concerns extensions and modifications developed during the development of 'NR' module. However, since there is currently no available documentation for the 'mmwave' module, we will briefly cover the features 
from 'mmwave' module that are utilized in 'NR' module. 

Over time, extensions found in this module may migrate to the existing |ns3| main development tree.

The rest of this document is organized into five major chapters:

2. **Design:**  Describes how |ns3| has been extended to support NR
   studies, and describes NR test scenarios design.
3. **Usage:**  Documents how users may run and extend these simulation 
   scenarios themselves.
4. **Validation:**  Documents how the models and scenarios have been verified
   and validated by test programs.
5. **Results:** Documents preliminary results from some scenarios and how
   to reproduce them.
6. **Open Issues and Future Work:**  Describes topics for which future work
   on model or scenario enhancements is recommended, or for which questions
   on interpretations of standards documents may be listed.



Design
------

In this section we present the design of the new features that we have developed following Release 15 3GPP NR activity. In particular, in the following we will focus and discuss the following features: NR frame structure and numerologies, Configurable Subcarrier Spacing (SCS) support, FDM of numerologies, mini-slot and mixed UL-DL slot format support, OFDMA, scheduling. 
 


NR frame structure and numerologies
***********************************

In New Radio, the concept of 'numerology' is introduced to flexibly define the frame structure, in such a way that it can work in both sub-6 GHz and mmWave bands. This is achieved by creating multiple numerologies formed by scaling a basic subcarrier scaling (SCS), by an integer N, where 15 KHz is the baseline SCS and N is a power of 2. The numerology is selected independently of the frequency band, with possible SCS of 15 KHz to 240 KHz. 480 KHz is under study. Not all SCS options are proposed for all frequencies. For sub 6 GHz, only 15 KHz, 30 KHz, and 60 KHz are considered. Above 6 GHz the candidates are 60 KHz, 120 KHz and 240 KHz.
A numerology is defined by the SCS and the cyclic prefix overhead. Some parts of the numerology are flexible, like the SCS, while other are fixed. 
In the time domain, the subframe duration is fixed to 1ms and the frame length to 10ms. On the other hand, the length of the symbol and the slot depend on the numerology/SCS.

Each frame of length of 10 ms is split in time into 10 subframes, each 
of duration of 1 ms. Each subframe is split in time into a variable number of slots that depends on the configured numerology. 
The number of OFDM symbols per slot can be either 7 or 14 OFDM as is specified in [TR38912]_. 
The number of subcarriers per physical resource block (PRB) is fixed to 12, and the maximum number of PRBs according to Release 15 is 275. The SCS defines as well the size of PRB, and the total number of PRBs of the NR system. PRBs are grouped into PRB groups at MAC scheduling time.

NR also defines mini-slots composed of 2 OFDM symbols up to the slot length -1 in any band, and of 1 symbol, at least above 6 GHz. 
The purpose of mini-slots is to reduce the latency by providing more flexibility for the transmission of the small amounts of data.
Additionally, the concept of self-contained slot is recently proposed as an important feature to be supported to provide 
a significant latency reduction by reducing a delay in reception of e.g. HARQ feedback or UL grant. 
For example, ACK/NACK is scheduled in the same slot as DL data, or UL grant is followed by UL transmission in the same slot. 
Both of these features, mini-slots and self-contained slots, can be further combined with the FDM feature to accommodate 
different types of traffic and DL-UL traffic asymmetries in an adequate way.

Figure :ref:`fig-frame-structure-time` shows the NR frame structure in time domain for the numerology 4.

.. _fig-frame-structure-time:

.. figure:: figures/frame-structure-time-WNS3.*
   :align: center
   :scale: 60 %

   NR frame structure in time domain for the numerology 4


Figure :ref:`fig-frame-structure-freq` shows the NR frame structure in the frequency domain.  


.. _fig-frame-structure-freq:

.. figure:: figures/frame-structure-freq-WNS3.*
   :align: center
   :scale: 60 %

   NR frame structure in frequency domain

Our implementation currently supports numerologies 
shown in Table :ref:`tab-numerologies`.

.. _tab-numerologies:

.. table:: Implemented NR numerologies

   ===========   ==================   ==================     ================   =========================   ================
   Numerology    Slots per subframe   Symbol length (μs)     Slot length (ms)   Subcarrier spacing in kHz   Symbols per slot
   ===========   ==================   ==================     ================   =========================   ================
    0 (LTE)      1                         71.42                1                  15                          14
    1            2                         35.71                0.5                30                          14
    2            4                         17.85                0.25               60                          14
    3            8                         8.92                 0.125              120                         14
    4            16                        4.46                 0.0625             240                         14
   ===========   ==================   ==================     ================   =========================   ================

In order to simplify the modeling, the cyclic prefix is included in the symbol length.
For example, for numerology n=0, the OFDM symbol length is 1ms/14=71.42μs, 
while the real symbol length is 1/SCS=66.67μs and the cyclic prefix is 1/14 – 1/SCS = 4.8μs.
A slot can contain all downlink, all uplink, or at least one downlink part and uplink part. Data transmission can span multiple slots.


Enhancements to support NR frame structure and numerologies
===========================================================

At the time of writing, the latest release of the 'mmwave' module does not support flexible numerologies. 
In particular, the 'mmwave' module implements the frame structure according to its own design, which is not compliant with the previously mentioned requirements for subframe and frame lengths. The main difference compared to the NR frame structure is that there is no notion of slot as defined per NR specifications. Consequently, in 'mmWave' module, to modify the numerology one would need to e.g. change the subframe length,
which in turn should be fixed to 1~ms according to the NR frame structure, or by changing other parameters. However, 
without having the slot granularity it is hard to match the NR configurations of the numerologies by modifying 
the available parameters.

The authors of the 'mmWave' module propose a specific frame structure configuration which 
is similar to the NR numerology 4, but has some important differences. 
For example, the subframe, which determines the granularity of the MAC scheduling in the 'mmWave' module, 
is composed of 24 OFDM symbols, 
while in NR, the granularity of the MAC scheduling is per slot, 
which is composed of 14 OFDM symbols. Additionally, in the frequency domain the total bandwidth is 
composed of 72 subbands, out of which each subband has bandwidth of 13.89~MHz and is composed of 48 subcarriers, 
while the NR structure assumes a fixed number of SCSs per PRB, and this number is 12.  
Since the number of subcarriers per PRB is fixed, the SCS defines the size of PRB, 
and also the total number of PRBs of the NR system.
In case of numerolgoy 4, the width of the PRB should be 2.88 MHz, and the total number of PRBs depends on the 
system bandwidth. 

To support the NR frame structure in the time domain and the scheduling operation per slot granularity, 
we have introduced an additional granularity level in time domain, the slot. 
Thus, according to the new frame structure design, frame has length of 10 ms and is split into 
10 subframes, each of duration of 1 ms. Each subframe is further split in time into a variable number 
of slots that depends on the configured numerology. 
The number of OFDM symbols per slot is 14 OFDM symbols.

According to our model, the transmission and the reception are per slot level, and are handled by the functions ``StartSlot()`` and ``EndSlot``, respectively. These functions are executed with a fixed periodicity, i.e., every 14 OFDM symbols, but the real duration of the slot depends on the configured 
numerology.
The numerology is specified by the numerology attribute of ``MmWavePhyMacCommon`` class. 
Once the numerology is configured, the lengths of the symbol, 
the slot, the SCS, and the number of PRBs are dynamically determined in a runtime.

Besides supporting SCS up to 240 KHz, we also support numerology 5, with SCS=480 kHz, which might be used in future NR releases for operation in high carrier frequencies.
Numerology 5 is defined by SCS of 480 KHz, OFDM symbol length of 2.08 us, CP of 0.15 us, slot length of 31.25 us, and there are 
32 slots in a single subframe.

Currently, the first and the last OFDM symbols of the frame structure are reserved for DL and UL CTRL, repectively (e.g., DCI and UCI).
According to the current design, the MAC-to-PHY delay depends on the numerology. MAC-to-PHY delay is 
configured as the number of slots and defaults to 2. 
On the other hand, the transport block decoding time at UE is fixed and equals to 100μs.








FDM of numerologies
********************
An additional level of flexibility in NR system can be achieved by employing the multiplexing of numerologies in the frequency domain.
In 5G it is expected that a base station (a.k.a. gNB in NR) should provide access to different types of services, as enhanced Mobile
BroadBand (eMBB), massive Machine Type Communications (mMTC), and Ultra-Reliable and Low Latency Communications (URLLC). 
A latency-throughput trade-off appears when attempting the selection of the proper numerology for gNB's
operation: larger SCS is better to reduce latency and for complexity (i.e., for URLLC traffic), 
while lower SCS is better for high throughput performance (i.e., for eMBB traffic).
For that reason, 3GPP NR specifies that multiple numerologies should be able to be multiplexed in time and/or frequency 
for a better user (a.k.a. UE in NR) performance under different types of data. 
This way, a gNB can accommodate different services with different latency requirements, as eMBB and URLLC, within the same
channel bandwidth. 
When numerologies are multiplexed in a frequency domain, a.k.a. frequency division multiplexing (FDM) of numerologies, 
each numerology occupies a part of the whole channel bandwidth, 
which is referred to in NR as a bandwidth part (BWP). 
In addition, occasions where multiplexing of numerologies in time domain, 
a.k.a. time division multiplexing (TDM) of numerologies is needed are rare if the gNB configures properly the FDM of numerologies for eMBB and URLLC traffics. 
3GPP agreed that the BWP allocation has to be statically or semi-statically configured.

In general, URLLC traffic requires a short slot length to meet strict latency requirements, 
while eMBB use case in general aims at increasing throughput, which is achieved with a large slot length. 
Therefore, among the set of supported numerologies for a specific operational band and deployment configuration, 
URLLC shall be served with the numerology that has the shortest slot length,
and eMBB with the numerology associated to the largest slot length.
That is, the numerology for URLLC will be larger than the numerology for eMBB.
Hence, the main objective of FDM of numerologies is to address the trade-off between latency and throughput for different types of traffic.

In Figure :ref:`fig-bwps` we illustrate an example of FDM of two numerologies.
In the example, the channel is split into two BWPs that accommodate the two aforementioned numerologies multiplexed in frequency domain. The total bandwidth $B$ is divided into BWPs of bandwidth $B_u$ for URLLC and $B_e$ for eMBB, 
so that $B_u + B_e \leq B$. 
The number of PRBs for URLLC is $N_u$ and $N_e$ for eMBB. Note, that the PRB width varies with the numerology.


.. _fig-bwps:

.. figure:: figures/bwps.*
   :align: center
   :scale: 60 %

   An example of BWPs




Enhancements to support FDM of numerologies
============================================
The main challenge in designing and implementing the FDM feature in mmWave module is that this feature 
is not yet fully defined by 3GPP. For example, 3GPP does not define the control plane functionality of the FDM. 
The PHY needs a capability of configuration of different BWPs, and this configuration 
may be static or semi-static. On the other hand, from the simulation point of view, 
a typical use case scenario for FDM of numerologies would be to allow configuration of a small number of different BWPs, 
and as a preliminary implementation of this feature it would be sufficient that this configuration is static. 
At this point, this is an important design assumption for two reasons. 
Firstly, the current implementation of the channel and the propagation loss model of the mmWave module is not 
designed in a way to allow runtime modifications of the physical configuration parameters related to frequency, 
such as: the system bandwidth, the central carrier frequency, and neither 
time related physical layer parameters, such as symbol and slot length.
Thus, until the current 3GPP mmWave channel model is not being modified to allow these runtime configuration changes, it will not be possible to perform semi-static reconfiguration of BWPs.

Additionally, since the control plane functionality of the FDM is not defined yet by 3GPP 
and it is not clear how the runtime reconfiguration would be performed, 
i.e., the PHY reconfiguration of BWPs, the periodicity, and the control messages. Some options that are being considered by 3GPP are 
via the broadcast channel, RRC signaling or periodical group common PDCCH.

Regarding the data plane, note that there is a similarity of the concept of the component carriers (CCs) in LTE and the BWPs in NR. 
While the purpose is different, the concept of having aggregated physical layer resources remains the same. 
The main difference is that in LTE, the different carriers have the same symbol, slot, subframe, and frame boundary, 
while in NR only the subframe and frame boundaries of different BWPs are aligned.
Another difference is that the bandwidth of a Component Carrier (CC) in LTE is predefined and cannot be dynamically changed. 
On the other hand, in NR it is expected to allow semi-dynamic reconfiguration of the bandwidth of different BWPs.
Additionally, in NR the SCS may be BWP-dependent, while in LTE it is the same for all sub-carriers.
However, in the case of static BWPs configuration, possibility of having different configurations of time and frequency 
among BWPs does not make the BWP concept in its essence very different from the CCs in LTE.

Taking into account the previously explained assumptions along with the currently available requirements for the control and data plane 
implementation of FDM of numerologies, the main design idea is to leverage the Carrier Aggregation (CA) feature of the 'LTE' module to implement the 
basic FDM feature in the 'NR' module. Note that the CA feature is mainly living in the Radio Respurce Control (RRC) and MAC layers. 
Since the current implementation of the 'mmWav'e leverages on the LTE module implementation of RRC, 
incorporating this part of CA functionality is automatically done by upgrading the 'LTE' module to the version that includes the CA feature.
In this way, the 'mmWave' module inherits the features, such as the extension of RRC 
messages in order to handle the secondary CC (SCC) information. 
By leveraging on the LTE CA RRC functionality, each NR UE establishes the cell connection following the LTE R8 procedure. 
This includes a cell search and selection, followed by the system information acquisition phase and the random access procedure. 

The RRC is in charge of the connection procedure, and it configures the primary CC (PCC).
Once the UE is in connected state, the gNB RRC is responsible to perform reconfiguration, 
addition and removal of SCCs. 
Since the ``UE Capability Inquiry`` and ``UE Capability Information`` are not implemented,
it is assumed that each UE can support any BWP configuration provided by the 
gNB to which it is attached. Since, 3GPP defines the Primary CC (PCC) as UE related and not as eNB related, 
according to LTE CA the PCC is the CC that is perceived as the most robust connection for each UE.

While, the RRC CA functionality is inherited from the 'LTE' module, the rest of the protocol stack of both, gNB and UE, 
needs to be updated according to the CA architecture. This means that the models of both ``MmWaveEnbDevice`` and ``MmWaveUeDevice`` 
shall be extended to support the installation of the MAC and PHY per carriers.
Additionally, the MAC layer of 'mmWave' module needs to be updated to allow scheduling on different BWPs. 

It should be defined how the MAC scheduling will be performed in multi-BWP system, 
and how will be this information transmitted to the UE.
Different options are proposed recently to be included in 3GPP specification: 

* dedicated control channel
* anchor control channel, and 
* shared control channel. 

.. _gnb-changes:

.. figure:: figures/gnb-changes.*
   :align: center
   :scale: 60 %

   Class diagram of changes at ``MmWaveEnbNetDevice``

   Sending the scheduling information through a dedicated control channel of each 
BWP assumes that the UE is capable of decoding several control channels simultaneously.
Depending on the type of traffic, the UE can be scheduled on different BWPs. This option is 
analog to independent carrier scheduling in LTE Release 10, according to which each CC has its 
own physical DL control channel (PDCCH) and the scheduler allocated traffic per CC-basis. 
Anchored control channel is transmitted on the BWP that is always ON or been used most frequently.
Transmitting the scheduling information through anchored control channel means that the control information 
for all BWPs is transmitted on the same BWP. This is similar to cross-carrier scheduling in LTE Release 10, according to which 
the scheduling information is sent through the PCC. 
The third option, shared control channel, means that the control channel spans over BWPs in which the UE is being scheduled. 
Thus the bandwidth of the shared control channel dynamically changes depending on the scheduling information for UE.

In the current implementation of FDM of numerologies in |ns3|, we support the transmission of the 
scheduling information through a dedicated control channel in each BWP, 
and the MAC scheduling and HARQ processes are performed per BWP. 
Finally, according to our model, the multiplexing of the data flows based on the type of traffic is performed 
by a new entity called BWP manager, whose role is similar to that of CC manager in the LTE module.
The BWP manager can use 5G quality-of-service (QoS) identifiers (5QI), as defined [3GPPTSGSSA]_, 
to determine on which numerology (i.e., BWP) to allocate the packets of a radio bearer and to establish priorities among radio bearers. 

The first step to implement this design was to upgrade to latest 'LTE', in order to include the CA implementation therein available.
However, upgrading of the 'LTE' module to the latest ns-3-dev version is not enough for the correct functioning of CA with 'mmWave' module, 
since the latest version of the 'LTE' module expects the service access point (SAP) interfaces of the lower layers and the stack 
initialization according the CA design. 
The class that is responsible for these initializations in the 'mmWave' module is ``MmWaveHelper`` class. 
Thus, the second major changes in the 'mmWave' module to enable CA architecture are in the functions of 
``MmWaveHelper`` class for installation of gNB and UE, and these are respectively, ``InstallSingleEnbDevice`` and 
``InstallSingleUeDevice``. Additionally, the function that has suffered important changes in ``MmWaveHelper``
is ``DoInitialize``, which is responsible for the initialization of the channel and the pathloss model. 

``InstallSingleEnbDevice`` function is modified to allow configuration of the gNB according to CA architecture. 
In Figure \ref{fig:gnb-changes} we illustrate the changes in the model of gNB device, which are highlighted in dark green.
The gNB device which is in 'mmWave' module represented by ``MmWaveEnbNetDevice`` is extended to 
include the map of CCs. Similarly to the LTE module, a CC consists of the MAC and PHY layers of each CC. 
Since the MAC and PHY configuration parameters in ``MmWavePhyMacCommon`` instance 
are extended to support a flexible numerology configuration, 
each CC can now be configured as a separate BWP with different numerology and other BWP specific parameters: 
carrier frequency, bandwidth, L1L2 processing and decoding delay, HARQ timing, etc. 
Apart from latter parameters, another important BWP-specific parameter is the transmission power per BWP 
which can be configured through the attribute of ``MmWaveEnbPhy``.

``MmWaveEnbNetDevice`` is extended to include the CC manager entity whose interface is implemented 
in abstract class called ``LteEnbComponentCarrierManager``. The already existing set of LTE CC manager implementations is 
extended with a new CC manager entity called ``BwpManager``, 
which is different from the LTE CC manager since it can make decisions based on the numerology configuration of carriers.
The role of the ``BwpManager`` class is to allow differentiation of the traffic flows based on their QoS requirements, 
and to forward the flow to the BWP which would provide the minimum latency for latency-sensitive types of flows, and 
which would maximize the throughput for the traffic flows that do not have a tight latency constraint. 
Current implementation of ``BwpManager`` supports all LTE EPS bearer QoS types, and 
the assignment of the corresponding BWP is based on the static configuration provided by the user. 
I.e., user shall specify which QoS type shall be mapped to which numerology and BWP.
Similarly to the changes in gNB device that we illustrated in Figure :ref:`gnb-changes`, 
we have also extended the NR UE model to support the CCs and UE CC manager. 
However, in the current implementation, 
we are interested only in FDM of numerologies coordinated by the gNB and its BWP manager.
Additionally, the function ``InstallSingleEnbDevice`` of ``MmWaveHelper`` is responsible for building up the whole protocol 
stack of the gNB and installing the SAP interfaces for control and data planes. 
In Figure Figure :ref:`ca-enb-data-plane-v6` we illustrate the gNB data plane according to this implementation. 
The main changes are installation of CC manager between the RLC and the MAC layers, 
and installation of CCs. To allow this architecture, the CC manager implements the SAP interfaces 
that were previously handled by RLC and MAC. In this way, gNB MAC and PHY continue to function as in the 
previous architecture, while CC manager is in charge of handling the information needed for CC management and 
multiplexing the flows over different carriers (or BWPs). Also, the control plane architecture of the gNB device 
is updated according the LTE module CA architecture. The full description of the LTE module CA architecture is provided in 
[CA_WNS32017]_.
Similarly, the function ``InstallSingleUeDevice`` is modified to allow installation of NR UE device according to
the LTE module CA architecture. ``InstallSingleUeDevice`` builds the CCs, installs the CC manager, and 
builds the protocol stack that is analog to the LTE UE CA.
``DoInitialize`` function of ``MmWaveHelper`` is modified to allow configuration of different BWPs. 
However, this initialization is different from ``DoInitialize`` of ``LteHelper`` due to the current 
limitation of the 'mmWave' module channel models (MmWaveBeamforming, MmWaveChannelMatrix, 
MmWaveChannelRaytracing, MmWave3gppChannel), 
all of which depend on the various PHY and MAC configuration parameters 
specified through a single instance of ``MmWavePhyMacCommon``. 
Hence, due to the current design of the 'mmWave' module and channel models it is not possible like in LTE module 
to install devices operating on different central carrier frequencies, neither to install different CCs.
Thus, to enable installation of different CCs, we have modified ``DoInitialize`` to allow installation 
of ``SpectrumChannel`` instance per CC. In this way, a single simulation will have as many 
``MmWave3gppChannel`` channel model instances as BWPs are configured. 
However, an important limitation of this design is that all gNBs and UEs in the simulation have the same BWP configuration.
Each ``MmWave3gppChannel`` instance will represent a channel model for a different BWP whose 
parameters are defined in a corresponding instance of ``MmWavePhyMacCommon`` class. 

.. _ca-enb-data-plane-v6:

.. figure:: figures/ca-enb-data-plane-v6.*
   :align: center
   :scale: 60 %

   gNB data plane architecture with CA extension

mini-slot and mixed UL-DL slot format support
************************************************************
According to the NR definition of TTI, 
one TTI duration corresponds to a number of consecutive symbols in the time domain in one transmission direction, 
and different TTI durations can be defined when using different number of symbols 
(e.g., corresponding to a mini-slot, one slot or several slots in one transmission direction). 
Thus, a TTI is in general of variable length, regardless of the numerology.
To support operation per slot, instead of ``SfAllocInfo`` structure that is used in the 'mmWave' module to hold the information corresponding to a 
subframe, we introduce ``SfAllocInfo`` that holds the information per slot, 
and it extends the ``SfAllocInfo`` structure according the NR frame structure. 
One ``SfAllocInfo`` is built through a list of ``VarTtiAllocInfo`` elements.
``VarTtiAllocInfo`` contains the scheduling information for a single TTI whose duration is no longer than that of one slot. 
At the UE PHY, ``VarTtiAllocInfo`` objects are populated after reception of Downlink Control Information
(DCI) messages. For each ``VarTtiAllocInfo`` the scheduler specifies which are containing 
contiguous OFDM symbols, along with the information whether the allocation is DL or UL, 
and whether is control or data. 
Currently, according to the 'mmWave' module design, and in accordance with the self-contained slot structure in NR,
the first and the last OFDM symbol of the frame structure are reserved for, respectively, DL control and UL control.


OFDMA
********************
To give flexibility in implementation choice, NR is deliberately introducing functionality to support analog beamforming in addition to digital precoding/beamforming. At high frequencies, analog beamforming, where the beam is shaped after digital-to-analog conversion, is necessary from an implementation perspective. Analog beamforming (or single-beam capability) results in the constraint that a receive or transmit beam can only be formed in a single direction at any given time instant and requires beam-sweeping where the same signal is repeated in multiple OFDM symbols but in different transmit beams. With beam-sweeping possibility, it is ensured that any signal can be transmitted with a high gain and narrow beamformed transmission to reach the entire intended coverage area.

The 'NR' module supports TDMA and OFDMA with single-beam capability and variable TTI.

The implementation of OFDMA under the single-beam capability constraint for mmWave means that frequency-domain multiplexing of different UEs is allowed among UEs associated to the same beam, so that in one OFDM symbol, or group of OFDM symbols, UEs that have the same beam ID can be scheduled in different RBGs, but not are UEs attached to different beam IDs. This is motivated by two main reasons. First, it is compatible with radio-frequency architectures based on single-beam capability, which is one of the main requirements for operation in bands with a high centre carrier frequency (mmWave bands). Second, it allows meeting the occupied channel bandwidth constraint in the unlicensed spectrum, e.g., that is required at the 5 GHz and 60 GHz bands, for any scheduling decision under the aforementioned constraint, since the scheduler will group UEs per beam ID and, within a TTI, only UEs that have the same beam ID would be allowed to be scheduled for DL transmission in different RBs. 

The implementation of OFDMA with variable TTI is motivated by the NR specifications, which encompass slot- and mini-slot-based transmissions, and thus a variable TTI length composed by a flexible number of symbols may be encountered.

To account for OFDMA, the code relies on a bitmask per UE that is an output of the scheduler and then introduced in the DCI to enable correct decoding at the receiver. The bitmask is used at MAC level, and it is a vector of 0s and 1s, of length equal to the number of RBGs, to indicate the RBGs assigned to a particular UE. This bitmask is translated into a vector of assigned RB indeces at PHY, of variable length, at most the number of available RBs, for compatibility issues with PHY layer functions. In NR, a RBG may encompass a group of 2, 4, 8, or 16 RBs [TS38214]_ Table 5.1.2.2.1-1, depending on the SCS and the operational band. This is a configuration parameter. In case the number of RBs in a RBG equals to the number of RBs in the whole channel bandwidth, then one can properly configure a TDMA-based channel access with variable TTI.

In addition, OFDMA-based access in 'NR' module has implied changes in the scheduler, HARQ operation, AMC model, temporal evolution, interference computation at PHY, and packet burst generation, as compared to the 'mmWave' module. Scheduler and HARQ for OFDMA are detailed in next section. The AMC model has been updated to map CQI into MCS, compute required number of RBGs from MCS and TBS, and to compute TBS from MCS, assigned number of RBGs and symbols. For OFDMA with variable TTI, all the temporal references are updated to enable a correct PHY behaviour both at gNBs and UEs. That is, the reference is not based on the transmitted TBs per UE (as in the 'mmWave' module, for which each symbol was assigned at most to a single UE), but on the TTIs assigned on different beams. Note that now, in the 'NR' module, within one symbol multiple UEs may be scheduled in different RBGs, so that the TBs do not indicate the temporal reference. This has incurred changes in the timing update at the devices. Finally the packet burst is generated in a way such that it may include different UEs in a TTI.

Due to the OFDMA with single-beam capability, the 'NR' module has a new PHY-MAC interface to enable communication to the MAC entity of the beam ID, which is computed at PHY, and thus allow the OFDMA with variable TTI scheduler at MAC to consider this new information. The new interface has two functions ``GetBeamId`` and ``ChangeBeamId``, to obtain the beam ID of a UE, and change it. The beam ID is characterized by two parameters, azimuth and elevation, and it is only valid for the beam seach method (i.e., for each UE, the transmission/reception beams are selected from a set of beams or codebook). 


Scheduler
********************
We have introduced schedulers for OFDMA and TDMA-based access with variable TTI under single-beam capability. The main output of a scheduler functionality is a list of DCIs for a specific slot, each of which specifies four parameters: the transmission starting symbol, the duration (in number of symbols) and an RBG bitmask, in which a value of 1 in the position x represents a transmission in the RBG number x. 
The current implementation of schedulers API follows the FemtoForum specification for LTE MAC Scheduler Interface [ff-api]_ , but can be easily extended to be compliant with different industrial interfaces.

The core class of the NR module schedulers design is ``MmWaveMacSchedulerNs3``. This class defines the core scheduling process and splits the scheduling logic into the logical blocks. Additionally, it implements the MAC schedulers API, and thus it decouples a scheduling logic from any specific MAC API specification. These two features facilitate and accelerate the introduction of the new schedulers specializations, i.e., the new schedulers only need to implement a minimum set of specific scheduling functionalities without having to follow any specific industrial API. The core scheduling process is defined in ``ScheduleDl`` and ``ScheduleUl`` functions. The scheduling process assigns the resources for active DL and UL flows and notifies the MAC of the scheduling decision for the corresponding slot. The scheduling logic for DL and UL are defined, respectively, in ``DoScheduleDl`` and ``DoScheduleUl`` functions. Currently, since in the uplink the TDMA is used, the ``DoScheduleUl`` is designed to support only TDMA scheduling. On the other hand, ``DoScheduleDl`` is designed to allow both, TDMA and OFDMA, modes for the downlink. Through these functions it is delegated to subclasses to perform the allocation of symbols among beams (if any), allocation of RBGs in time/frequency-domain among active UEs by using specific scheduling algorithm (e.g., round robin, proportional fair, etc.), and finally, the construction of corresponding DCIs/UCIs. For example, TDMA scheduling can be easily implemented by skipping the first step of allocating symbols among beams and by fixing the minimum number of assignable RBGs to the total number of RBGs. To obtain true TDMA-based access with variable TTI, it is then necessary to group allocations for the same UE in one single DCI/UCI which is the last step. Another important class to be mentioned is ``MmWaveMacSchedulerNs3Base`` which is a child class of ``MmWaveMacSchedulerNs3``, and represents a base class of all schedulers in the NR module (OFDMA and TDMA). This class handles the HARQ retransmissions for the DL and the UL. Currently, the NR module offers the scheduling of the HARQ retransmissions in a round robin manner by leveraging the ``MmWaveMacSchedulerHarqRr`` implementation. Scheduler inheritance model and collaboration diagram are shown in Figures ::`fig-nr-scheduler-collab` and ::`nr-schedulers`.

An overview of the different phases that the OFDMA schedulers follow are:

1) BSR and CQI messages processing. The MCS is computed by the AMC model for each user based on the CQIs for the DL or SINR measurements for the UL data channel. The MCS and BSR of each user are stored in a structure that will be later read to determine UE capabilities and needs. The procedure for estimating the MCS and determining the minimum number of RBs is common to all the OFDMA-based schedulers that we may derive. 
2) Upon being triggered by the MAC layer, the scheduler prepares a slot indication. As a first step, the total number of active flows is calculated for both UL and DL. Then, the UL is processed, and then the DL. This requirement comes from the fact that UL and DL have, in most cases, different delays. This delay is defined as the number of the slots that have to pass between the moment in which the decision is taken, and the moment that such decision is traveling in the air. The default delay parameters are 2 slots for DL and 4 slots for UL: therefore, UL data can be penalized by the higher delay, and hence has to be prioritized in some way when preparing the slot. For this reason, the scheduler is also taking UL and DL decision for the same slot in different moments.
3) The UL decisions are not considered for the slot indicated by the MAC layer, but for a slot in the future. These involve firstly any HARQ retransmission that should be performed, for instance when the previous transmission has been NACKed. The requirement for retransmitting any piece of data is to have enough space (indicated by the number of RBG). This is because, while the retransmission does not need to start at the same symbol and RB index as the previous transmission of the same TB, it does need the same number of RBGs and MCS, since an adaptive HARQ scheme (where the re-transmission can be scheduled with a different MCS) is not implemented. If all the symbols are used by the UL retransmissions, the scheduling procedure ends here. Otherwise, UL data is scheduled, by assigning the remaining resources (or less) to the UEs that have data to transmit. The total number of symbols reserved for UL data is then stored internally along with the slot number to which these allocations are referred, and the procedure for UL ends here.
4) The procedure for DL allocations is started, relative to the slot indicated by the MAC layer. The number of symbols previously given for UL data in the current slot has to be considered during the DL phase. Before evaluating what data can be scheduled, that number is extracted from the internal storage, and the DL phase can continue only if there are available symbols not used by the UL phase. If it is the case, then, the symbols can be distributed by giving priority to the HARQ retransmissions, and then to the new data, according to different metrics.

The base class for OFDMA schedulers is ``MmWaveMacSchedulerOfdma``. In the downlink, ``MmWaveMacSchedulerOfdma`` class and its subclasses perform OFDMA scheduling, while in the uplink they leverage some of the subclasses of ``MmWaveMacSchedulerTdma`` class that implements TDMA scheduling. 

The OFDMA scheduling in the downlink is composed of the two scheduling levels: 1) the scheduling of the symbols per beam (time-domain level), where scheduler selects a number of consecutive OFDM symbols in a slot to assign to a specific beam, and 2) the scheduling of RBGs per UE in a beam, where the scheduler determines the allocation of RBGs for the OFDM symbols of the corresponding beam (frequency-domain level).
The scheduling of the symbols per beam can be performed in a load-based or round robin fasion. The calculation of load is based on the BSRs and the assignment of symbols per beam is proportional to the load. In the following level, the specific scheduling algorithm (round robin, proportional fair, max rate) decides how RBGs are allocated among different UEs asociated to the same beam.
Multiple fairness checks can be ensured in between each level of scheduling - the time domain and the frequency domain. For instance, a UE that already has its needs covered by a portion of the assigned resources can free these resources for others to use. 

The NR module currently offers three specializations of the OFMA schedulers. These specializations are implemented in the following classes: ``MmWaveMacSchedulerOfdmaRR``, ``MmWaveMacSchedulerOfdmaPF``, and ``MmWaveMacSchedulerOfdmaMR`` , and are, respectively, performing the downlink scheduling in a round robin (RR), proportional fair (PF) and max rate (MR) manner, as explained in the following: 

* RR: the available RBGs are divided evenly among UEs associated to that beam
* PF: the available RBGs are distributed among the UEs according to a PF metric that considers the actual rate (based on the CQI) elevated to α and the average rate that has been provided in the previous slots to the different UEs. Changing the α parameter changes the PF metric. For α=0, the scheduler selects the UE with the lowest average rate. For α=1, the scheduler selects the UE with the largest ratio between actual rate and average rate.
* MR: the total available RBGs are distributed among the UEs according to a maximum rate (MR) metric that considers the actual rate (based on the CQI) of the different UEs.

Each of these OFDMA schedulers is performing a load-based scheduling of symbols per beam in time-domain for the downlink. In the uplink, the scheduling of ``MmWaveMacSchedulerOfdmaRR``, ``MmWaveMacSchedulerOfdmaPF``, and ``MmWaveMacSchedulerOfdmaMR``, is leveraging the implementation of, respectively, ``MmWaveMacSchedulerTdmaRR``, ``MmWaveMacSchedulerTdmaPF``, and ``MmWaveMacSchedulerTdmaMR`` TDMA schedulers.

The base class for TDMA schedulers is ``MmWaveMacSchedulerTdma``. This scheduler performs TDMA scheduling for both, the UL and the DL traffic. The TDMA schedulers perform the scheduling only in the time-domain, i.e., by distributing OFDM symbols among the active UEs. NR module offers three specializations of TDMA schedulers: ``MmWaveMacSchedulerTdmaRR``, ``MmWaveMacSchedulerTdmaPF``, ``MmWaveMacSchedulerTdmaMR``, where the scheduling criteria is the same as in the corresponding OFDMA schedulers, while the scheduling is performed in time-domain instead of the frequency-domain, and thus the resources being allocated are symbols instead of RBGs.



.. _fig-nr-schedulers:

.. figure:: figures/nr-schedulers.*
   :align: center
   :scale: 75 %

   NR scheduler inheritance class diagram



.. _fig-nr-scheduler-collab:

.. figure:: figures/nr-scheduler-collab.*
   :align: center
   :scale: 75 %

   NR scheduler class collaboration diagram


Uplink delay support for UL data
********************************
In an NR system, the UL decisions for a slot are taken in a different moment than the DL decision for the same slot. In particular, since the UE must have the time to prepare the data to send, the gNB takes the UL scheduler decision in advance and then sends the UL grant taking into account these timings. For example, consider that the DCIs for DL are usually prepared two slots in advance with respect to when the MAC PDU is actually over the air. For example, for UL, the UL grant must be prepared four slots before the actual time in which the UE transmission is over the air transmission: after two slots, the UL grant will be sent to the UE, and after two more slots, the gNB is expected to receive the UL data. Please note that latter examples consider default values for MAC to PHY processing delays at gNB and UE, which are in NR module set to 2 slots. The processing delays are parameters of the simulator that may be configured through corresponding attributes of ``MmWavePhyMacCommon`` class.

At PHY layer, the gNB stores all the relevant information to properly schedule reception/transmission of data in a vector of slot allocations. The vector is guaranteed to be sorted by the starting symbol, to maintain the timing order between allocations. Each allocation contains the DCI created by the MAC, as well as other useful information. 

To accommodate the NR UL scheduling delay, the new MAC scheduler design is actively considering these delays during each phase.


Scope and Limitations
********************
This module implements a partial set of features currently defined in the standard. Key aspects to be addressed in future works are described in the following.
First of all, the PHY layer abstraction inherits the Link to System mapping procedure of the 'LTE' module, which still lacks of 256QAM introduced in Release 12. The NR module should include a new Link to System mapping specific for the NR technology, and supporting the newly defined NR channel coding (LDPC, polar coding, block size and codeword size flexibility). Other important missing features are the spatial multiplexing and the autonomous UL, introduced in Release 15. The module also, at the time of writing needs upgrade to the latest |ns3| release, |ns3-28|. 




Usage
------------

This section is principally concerned with the usage of the model, using
the public API. We discuss on examples available to the user, the helpers, the attributes and the simulation campaign we run.


Examples
***********************************

Several example programs are provided to highlight the operation.

cttc-3gpp-channel-simple-ran.cc
============================================
The program ``mmwave/examples/cttc-3gpp-channel-simple-ran.cc``
allows users to select the numerology and test the performance considering only the radio access network (RAN). 
The numerology can be toggled by the argument, 
e.g. ``'--numerology=1'``. The scenario topology is simple, and it 
consists of a single gNB and single UE. The scenario is illustrated in 
Figure ::`fig-scenario-simple`.

.. _fig-scenario-simple:

.. figure:: figures/scenario-simple.*
   :align: center
   :scale: 75 %

   NR scenario for simple performance evaluation (RAN part only)

The output of the example is printed on the screen and it shows the PDCP and RLC delays.

By default, the program uses the 3GPP channel model, without shadowing and with 
line of sight ('l') option. 
The program runs for 0.4 seconds and only one packet is transmitted. 
The packet size can be toggled by the argument, 
e.g. ``'--packetSize=1'``.

This produces the following output:

::
  
  ./waf --run cttc-3gpp-channel-simple-ran


 Data received by UE RLC at:+402242855.0ns
 rnti:1
 lcid:3
 RLC bytes :1004
 RLC delay :2242855
 Packet PDCP delay:2242855

By default, the example is run with numerology 0.
   
To run with different numerology:

::

  ./waf --run "cttc-3gpp-channel-simple-ran --numerology=4"
  

the output should be: 

 Data received by UE RLC at:+400238391.0ns
 rnti:1
 lcid:3
 RLC bytes :1004
 RLC delay :238391
 Packet PDCP delay:238391

We notice that the RLC and PDCP delay are equal. This means that 
the packet has not been fragmented at RLC layer and a single transmission opportunity 
was enough to transmit it. 

cttc-3gpp-channel-nums.cc
============================================
The program ``examples/cttc-3gpp-channel-nums.cc``
allows users to select the numerology and test the end-to-end performance.
Figure :ref:`fig-end-to-end` shows the simulation setup. 
The user can run this example with UDP full buffer traffic and can specify the 
UDP packet interval.  

.. _fig-end-to-end:

.. figure:: figures/end-to-end.*
   :align: center

   NR end-to-end system performance evaluation

For example, this can be configured by setting the 
argument, e.g. ``'--udpFullBuffer=0'``, and by setting the interval 
``'--udpInterval=0.001'``. The numerology can be toggled by the argument, 
e.g. ``'--numerology=1'``. Additionally, in this example two arguments 
are added ``'bandwidth'`` and ``'frequency'``. The modulation scheme of 
this example is in test mode, it is fixed to 28.

By default, the program uses the 3GPP channel model, without shadowing and with 
line of sight ('l') option. 
The program runs for 0.4 seconds and one single packet is to be transmitted. 
The packet size can be toggled by the argument, 
e.g. ``'--packetSize=1'``.

This simulation prints the output to the terminal and also to the file which 
is named by default ``cttc-3gpp-channel-nums-fdm-output`` and which is by 
default placed in the root directory of the project. 
To run the simulation with the default 
configuration one shall run the following in the command line

::  
  ./waf --run cttc-3gpp-channel-nums


The example of the output is the following:
:: 

  Flow 1 (1.0.0.2:49153 -> 7.0.0.2:1234) proto UDP
  Tx Packets: 56254
  Tx Bytes:   57829112
  TxOffered:  771.055 Mbps
  Rx Bytes:   48814580
  Throughput: 651.917 Mbps
  Mean delay:  49.2909 ms
  Mean upt:  0.265386 Mbps 
  Mean jitter:  0.0229247 ms
  Rx Packets: 47485
 Total UDP throughput (bps):6.33133e+08

The output of this simulation represents the metrics that are collected 
by the flow-monitor ns-3 tool. 

By default, the example runs with numerology 0.
To run with different numerology:

::

  ./waf --run "cttc-3gpp-channel-nums --numerology=4"


the output should be: 


  Flow 1 (1.0.0.2:49153 -> 7.0.0.2:1234) proto UDP
  Tx Packets: 56254
  Tx Bytes:   57829112
  TxOffered:  771.055 Mbps
  Rx Bytes:   48792992
  Throughput: 650.605 Mbps
  Mean delay:  47.5682 ms
  Mean upt:  0.375825 Mbps 
  Mean jitter:  0.0189741 ms
  Rx Packets: 47464
  Total UDP throughput (bps):6.32853e+08

We notice that, as expected, the throughput is similar for different numerologies, 
while the delay is lower for the higher value of numerology. Also UPT throughput is higher.


cttc-3gpp-channel-simple-fdm.cc
============================================

The program ``examples/cttc-3gpp-channel-simple-fdm.cc`` can be used to simulate FDM in scenario with a single UE and gNB.
In this program the packet is directly injected to the gNB, so this program can be used only for simulation of the RAN part.
This program allows the user to configure 2 bandwidth parts (BWPs). The user can also configure the size of the packet by 
using the global variable ``packetSize``, and can also specify an indicator that tells whether the traffic flow is URLLC or 
eMBB. The latter parameter is called ``isUll`` and when configured to 1, it means that the traffic is URLLC, while when set to 0, it 
means that the trafic is eMBB. In the following we show how to run the program:

./waf --run 'cttc-3gpp-channel-simple-fdm --isUll=0'

Consider the case in  which a first BWP (Bandwidth part) uses numerology 4, central carrier frequency 28.1 GHz and the bandwidth of 100 MHz, 
and a second BWP uses numerology 2, central frequency 28 GHz and the bandwidth of 100 MHz.
The BWP manager should be configured in the following way:

Config::SetDefault ("ns3::BwpManager::GBR_ULTRA_LOW_LAT", UintegerValue (0));

Config::SetDefault ("ns3::BwpManager::GBR_CONV_VOICE", UintegerValue (1));

This means that the URLLC traffic would be served through BWP 1, and the another type, GBR_CONV_VOICE, would be transmitted through BWP 2. Thus, when the parameter :math:`isUll=0`
the output of the simulation is the following:

 Data received by UE RLC at:+400653570.0ns

 rnti:1

 lcid:3

 bytes :1004

 delay :653570

 Packet PDCP delay:653570


On the other hand, when :math:`isUll=1`, the output of the simulation is the following:

 Data received by UE RLC at:+400251784.0ns

 rnti:1

 lcid:3

 bytes :1004

 delay :251784

 Packet PDCP delay:251784
 
 As a result, in the previous example, we just vary the type of QCI (Quality Channel Indicator) of the flow, and 
 based on that, the BWP manager decides over which BWP will transmit the packet. 
 We notice that when the traffic is URLLC, it will be transmitted over the first BWP which is configured with 
 the higher numerology, :math:`\mu=4`, which guarantees much lower delay for a small amount of data then the 
 :math:`\mu=2`. In this case, the packet delay is 2.6 time lower than when the higher numerology is used. 
This example considers a fixed MCS (Modulation and Coding Scheme) of 28.

cttc-3gpp-channel-nums-fdm.cc
============================================

The program ``examples/cttc-3gpp-channel-nums-fdm.cc`` allows the user to configure 2 UEs and 1 or 2 bandwidth parts (BWPs) and test the end-to-end performance.
This example is designed to expect the full configuration of each BWP. The configuration of BWP is composed of the following parameters:
central carrier frequency, bandwidth and numerology. There are 2 UEs, and each UE has one flow. 
One flow is of URLLC traffic type, while the another is eMBB.
URLLC is configured to be transmitted over the first BWP, and the eMBB over the second BWP. 
Hence, in this example it is expected to configure the first BWP to use a higher numerology than the second BWP.
Figure :ref:`fig-end-to-end` shows the simulation setup. Note that this simulation topology is as the one used in ``scratch/cttc-3gpp-channel-nums.cc``
The user can run this example with UDP full buffer traffic or can specify the UDP packet interval and UDP packet size per type of traffic.
``udpIntervalUll`` and `packetSizeUll` parameters are used to configure the UDP traffic of URLLC flow, 
while ``udpIntervalBe`` and `packetSizeBe` parameters are used to configure the UDP traffic of eMBB flow.
If UDP full buffer traffic is configured, the packet interval for each flow is calculated based on approximated value of saturation rate for the bandwidth to 
which the flow is mapped, and taking into account the packet size of the flow. 
The total transmission power :math:`P_{(i)}` depends on the bandwidth of each BWP, and will be proportionally assigned to each BWP in the following way:

 .. math::
   
      P_{(i)}= 10\log_{10} ((bandwidthBwp(i)/totalBandwidth)*10^{(totalTxPower/10)}) 


If the user configures only 1 BWP, then the configuration for the first BWP will be used. 


cttc-3gpp-indoor-calibration.cc
=====================================

The program ``examples/cttc-3gpp-indoor-calibration`` is the simulation 
script created for the NR-MIMO Phase 1 system-level calibration. 
The scenario implemented in this simulation script is according to 
the topology described in 3GPP TR 38.900 V15.0.0 (2018-06) Figure 7.2-1: 
"Layout of indoor office scenarios".
The simulation assumptions and the configuration parameters follow 
the evaluation assumptions agreed at 3GPP TSG RAN WG1 meeting #88, 
and which are summarised in R1-1703534 Table 1. 
In the following Figure is illustrated the scenario with the gNB positions 
which are represented with "x". The UE nodes are randomly uniformly dropped 
in the area. There are 10 UEs per gNB.

The results of the simulation are files containing data that is being 
collected over the course of the simulation execution:

 - SINR values for all the 120 UEs
 - SNR values for all the 120 UEs
 - RSSI values for all the 120 UEs
 
Additionally, there are files that contain:

  - UE positions
  - gNB positions
  - distances of UEs from the gNBs to which they are attached

The file names are created by default in the root project directory if not 
configured differently by setting resultsDirPath parameter of the Run() 
function.

The file names by default start with the prefixes such as "sinrs", "snrs",
"rssi", "gnb-positions,", "ue-positions" which are followed by the 
string that briefly describes the configuration parameters that are being 
set in the specific simulation execution.


Simulation campaigns
***********************************

In this section, we describe briefly the simulation campaigns that we have carried out to test the new implemented features. 
We have created different simulation campaign scripts, where each script has different objectives. 
All simulation campaing scripts can be found in the folder src/mmwave/campaigns. In this folder, we group scripts by evaluated functionality, under different conditions.
For example, ``3gpp-nr-numerologies`` sub-folder contains simulation campaigns related to the evaluation of frame structure/numerologies, and 
``3gpp-nr-fdm`` sub-folder contains simulation campaigns related to the evaluation of FDM of numerologies. Additionally, 
the simulation campaign scripts for the 3gpp calibration phase 1 are placed in ``3gpp-calibration`` sub-folder.
If the simulation campaign is relatively small (it is not run for many different parameters) the output of campaign is printed on the screen, 
while in case of large campaigns, the results are written to results file in ``campaigns/*/results`` folder. 
The name of the file is normally composed to reveal with which values of the parameters is executed the simulation. 
There is a single file per simulation. File contains the statistics of all flows belonging to that simulation.

NR frame structure and numerologies
============================================

In this subsection, we provide and overview of simulation campaigns contained in ``campaigns/3gpp-nr-numerologies``.

Script called ``run-simple-ran.sh`` is used to run ``cttc-3gpp-channel-simple-ran.cc`` simulation example. The user can configure the set 
of numerologies for which she would like to carry out the simulation campaign. Additionally, the parameters that can be configured are 
the packet size and whether to use the fixed MCS.

Script ``run-e2e-single-ue-topology.sh`` is similar to the previous campaign since it also considers only 1 UE, but the simulation is now an end-to-end simulation defined in ``cttc-3gpp-channel-nums.cc``. This campaign allows variation of numerology, and 
additionally many parameters are available to be configured, such as bandwidth, frequency, whether to use a fixed MCS, 
the value of MCS in case of fixed MCS, which cell scan method to use, etc.  

Scripts ``run-e2e-multi-ue.sh``, ``run-e2e-multi-ue-diff-beam-angles.sh``, ``run-e2e-multi-ue-diff-cell-scan-method-adaptive-mcs-d2.sh`` 
and ``run-e2e-multi-ue-diff-mcs-d2.sh`` are all running the simulation script ``cttc-3gpp-channel-nums.cc`` in multi-UE mode. 
As a result, all of these scripts vary the number of gNBs and UEs per gNB. On the other hand, ``run-e2e-multi-ue.sh`` is a more generic simulation campaign. The 
rest of scripts are created to evaluate the impact of a specific parameter. For example, the simulation campaign called 
``run-e2e-multi-ue-diff-cell-scan-method-adaptive-mcs-d2.sh`` is used to compare cell search methods: long term covariance and beam 
search beamforming method. ``run-e2e-multi-ue-diff-beam-angles.sh`` is used to evaluate the performance of different angle steps of beam 
search beamforming method. And finally , the script called ``run-e2e-multi-ue-diff-mcs-d2.sh`` is running the program for different 
values of MCS, where the MCS is configured as fixed.

We consider now first scenario, for a single BWP, where we evaluate different numerologies while varying the traffic load.
The numerologies that we evaluate in this simulation are from 1 to 5. The traffic is DL UDP.
Packets of 100 bytes are being generated with a rate (in packets/s) which takes the following values: 1250, 62500, 125000, 250000, 375000.
The simulation scenario is composed of a single gNB and a single UE that are at 10 m distance. 
The height of the gNB is 10 m and of UE is 1.5 m.
The gNB transmission power is 1 dBm, the central carrier frequency is 28.1 GHz, and the bandwidth is 100 MHz. 
The channel model is the 3GPP channel model, and the scenario is UMi-StreetCanyon. 
Beam search is performed by using the long-term covariance matrix method. The delay between EPC and gNB is configured to 1 ms.

In Figures :ref:`delay-1bwp` and :ref:`throughput-1bwp`, we show the mean latency and the mean throughput of the 
flow versus the offered load, when the system is being configured to use different numerologies. 
From Figure :ref:`delay-1bwp`, we observe that when the system is not saturated there is a significant impact of 
the numerology on the delay. However, when it is saturated there is a great impact of higher layers buffer delays and 
the actual transmission time does not play an important role. 
However, with respect to throughput, we observe a different effect in Figure :ref:`throughput-1bwp`. 
When there is no saturation, each numerology is able to satisfy QoS requirements of the flow equally, i.e. the served rate is equal to the offered rate. 
But, once in the saturation region, the lower numerologies (1, 2 and 3) 
start to provide a slightly better throughput than the higher numerologies (4 and 5). 
The reason for this is that depending on the bandwidth and SCS of each numerology a different portion of the 
bandwidth will be utilized. E.g., for numerology 5, the SCS is 480 kHz, PRB (Physical Respurce Block) width is 5.76 MHz, thus the 
actual bandwidth used is 97.92 MHz, while for numerology 1, the SCS is 30 kHz, PRB width is 0.36 MHz, thus the 
actual bandwidth is 99.72 MHz. Hence, there is around 2% more bandwidth in the second case, which corresponds 
to the difference in the achieved throughput that we see in the Figure :ref:`throughput-1bwp`.
Note that the difference in the maximum throughput achieved by different numerologies depends on the channel bandwidth, 
and it is more remarkable for low bandwidths.

.. _delay-1bwp:

.. figure:: figures/delay_numerologies2.*
   :align: center
   :scale: 60 %

   Mean delay per UDP flow 
 


.. _throughput-1bwp:

.. figure:: figures/throughput_numerologies.*
   :align: center
   :scale: 60 %

   Mean throughput per UDP flow

FDM of numerologies
============================================

In this subsection, we describe the simulation campaigns to evaluate the FDM of numerologies. These simulation campaigns are evaluated on 
``cttc-3gpp-channel-nums-fdm.cc``. As mentioned earlier, this example can be configured to operate with 1 or 2 BWPs. 
The expected use of this example is with 2 BWPs, but 1 BWP can also be used for comparison purposes. This is the case with the 
``run-e2e-wns3-1bwp-symload.sh`` simulation campaign, in which there are 2 flows of different QCI. However, since there is only a single 
BWP, they will be served through the same BWP. The input parameter of this script is the configuration of BWP and also the 
configuration of traffic for both flows.

Script ``run-e2e-wns3-2bwps-symload.sh`` is used  to simulate FDM of numerologies, 
when the bandwidth is uniformly divided between the BWPs.


Script ``run-e2e-bwps-opt-asymload.sh`` can be used to configure different partitions 
of bandwidth among BWPs, and differently from two  simulations, 
the load of different BWPs is asymmetric. Finally, ``run-e2e-bwps-uni-asymload.sh`` 
has the uniform partition of the bandwidth, but has still and asymmetric load. 

We present the results of a simulation configuration in which we evaluate 
the performance of FDM of numerologies when the total system bandwidth is 
divided in 2 BWPs of equal size. There are two BWPs, of 100 MHz bandwidth each. 
The total transmission power is 4 dBm, which is uniformly distributed among the two BWPs, 
so that each BWP disposes of  1 dBm. 
The central carrier frequencies are 28.1 GHz and 28 GHz.
First BWP is configured with numerology 4 and the second BWP withnumerology 2. 
The topology consists of 1 gNB and 2 UEs. The first UE requests URLLC DL flow, 
and the second UE demands of an eMBB DL flow. 
URLLC flow packets are of 100 bytes and they are transmitted with 
rates from {1250, 62500, 125000, 250000, 375000}. eMBB flow packets are 1252 bytes. Packet sizes and rate are configured in this way 
to achieve the same offered throughput for both flows for any value of the rate. 
The packet sizes are adjusted to account for the UDP header of 28 bytes. 
UEs are placed at 10 m distance from gNB. 
When there are 2 BWPs, the BWP manager is configured to forward URLLC traffic over the BWP with a
higher numerology, and eMBB over the BWP with a lower numerology. We compare the performance of 2 BWPs configuration 
with a single BWP, of the same total bandwidth (200 MHz) and total power (4 dBm), and numerolgoy 1.

In Figures :ref:`delay-2bwps` and :ref:`throughput-2bwps` we show the performance in terms 
of the mean delay and the throughput versus the offered load per flow. As expected, we observe a positive impact of FDM of numerologies 
on the mean delay of URLLC flow. This flow benefits from being scheduled via BWP of a higher numerology. On the 
other hand, eMBB flow has almost the same performance in the terms of delay regardless if the FDM of 
numerologies is being used. With respect to throughput, we observe that URLLC flow obtains a 
better performance without FDM and when is transmitted over lower numerology in the saturation regime, due to the reasoning given in the previous subsection. 
However, there is almost no impact of FDM of numerologies on the performance of the eMBB flow.

.. _fig-delay_2b:

.. figure:: figures/delay_2b.*
   :align: center
   :scale: 60 %

   Mean delay per UDP flow 
 


.. _fig-throughput_2:

.. figure:: figures/throughput_2.*
   :align: center
   :scale: 60 %

   Mean throughput per UDP flow


3gpp Indoor Calibration Phase 1
================================

In this subsection, we describe the simulation campaign that is desidned for 
the 3gpp calibration Phase 1. The simulation script that implements the indoor 
scenario acoording to 3gpp phase 1 calibration is placed in  
``cttc-3gpp-indoor-calibration.cc``. The main script simulation campaign is 
``run-calibration.sh`` which laverages the parameter configuration 
defined in the config file which is placed in the root of the same directory.

We have run the simulation campaigns for different configurations:

 - Different 3gpp indoor pathloss models: 
    a) InH office-mixed, 
    b) InH office-open, 
    c) InH shopping-mall

 - Shadowing: 
    a) Enabled 
    b) Disabled

 - Different gNB antenna orientation:
    a) Z = 0 (XY plane) 
    b) X = 0 (ZY plane)

 - Beamforming method:
    a) Optimal 
    b) Beam search method for different beamsearching method angle, i.e. 5, 10, 30 degrees

 - gNB antenna radiation pattern:
    a) 3GPP single-sector according to 3gpp 38.802. Table 8.2.1-7, 
    b) 3GPP wall-mount according to 3gpp 38.802. Table 8.2.1-7, 
    c) Isotropic

 - UE antenna radiation pattern:
    a) 3GPP directional antenna according to 38.802. Table A.2.1-8:
    b) Isotropic

The rest of the parameters is in the following trying to be the closest possible 
to 3gpp calibration phase 1 simulation assumptions:

- Carrier frequency: 30 GHz
- Mode: DL only
- Bandwidth: 40 MHz
- SCS: 60 kHz (μ=2)
- Channel model: Indoor TR 38.900
- BS Tx Power: 23 dBm
- BS antenna configuration: M=4, N=8 (32 antenna elements), 1 sector, vertical polarization 
- UE antenna configuration: M=2, N=4 (8 antenna elements), 1 panel, vertical polarization 
- BS antenna height: 3 mt
- UE antenna height: 1,5 mt
- BS noise figure: 7 dB
- UE noise figure: 10 dB
- UE speed: 3 km/h 
- Scheduler: TDMA PF
- Traffic model: full buffer

The deployment scenario is composed of 12 sites at 20 meters distance, 
and 120 UEs (100% indoor) randomly dropped in a 50 x 120 meters area. 

As reference curves, we use the results provided by the companies in R1-1709828. 
We consider the CDF of the wideband SINR with beamforming, and the CDF of the 
wideband SNR with step b (i.e., with analog TX/RX beamforming, 
using a single digital TX/RX port). For each case, we depict as reference the 
average of the companies contributing to 3GPP as well as the company that gets 
the minimum and the maximum of the average wideband SNR/SINR, so that a region 
for calibration is defined.

In Figures :ref:`snr` and :ref:`sinr`, we display the CDF of wideband SNR and 
SINR of one of the confiugurations that match 3GPP calibration region. The simulation 
configuration is the folliowing:
 - pathloss model is the indoor shopping-mall, 
 - shadowing is enabled, 
 - gNB antenna orientation is XY (Z=0),
 - beam search method is optimal,
 - gNB 3GPP wall-mount, 
 - UE 3GPP (see Figure 12 and Figure 13)


.. _fig-snr:

.. figure:: figures/snrs-r1-sh1-aoZ0-amGnb3GPP-amUe3GPP-scInH-ShoppingMall-sp3-bs0-ang10-gmWALL.*
   :align: center
   :scale: 60 %

   SNR
 
.. _fig-sinr:

.. figure:: figures/sinrs-r1-sh1-aoZ0-amGnb3GPP-amUe3GPP-scInH-ShoppingMall-sp3-bs0-ang10-gmWALL.*
   :align: center
   :scale: 60 %

   SINR


Helpers
***********************************

What helper API will users typically use?  Describe it here.

Attributes
***********************************

Common NR attributes
==========================

Class mmwave-phy-mac-common holds common NR attributes. According 
to our current design all NR devices in the simulation have the same numerology 
configuration which is the one that is specified in mmwave-phy-mac-common class.
In table :ref:`tab-mmwave-phy-mac-common` we show the mmwave-mac-phy-common class attributes.


.. _tab-mmwave-phy-mac-common:

.. table:: NR common system attributes defined through mmwave-phy-mac-common class

        ====================   ==================================================================================       ==================      
	Name 		       Description 										Default value		
        ====================   ==================================================================================       ==================     
	Numerology             The 3GPP numerology to be used                                                                      4               
	Bandwidth              The system bandwidth in Hz                                                                          400e6             
	CtrlSymbols            The number of OFDM symbols for DL control per subframe                                               1	           
	NumReferenceSymbols    The number of reference symbols per slot                                                             6              
	CenterFreq             The center frequency in Hz                                                                          28e9            	
	UlSchedDelay           The number of TTIs between UL scheduling decision and subframe to which it applies                   1              
	NumRbPerRbg            The number of resource blocks per resource block group                                               1              
	WbCqiPeriod            The period between wideband DL-CQI reports in microseconds                                           500            	
	NumHarqProcess         The number of concurrent stop-and-wait Hybrid ARQ processes per user                                 20             
	HarqDlTimeout          Downlink harq timeout timer                                                                          20
	TbDecodeLatency        Transport block decode latency in microseconds                                                       100            
        ====================   ==================================================================================       ==================     


User may specify the numerology of the system by configuring "Numerology" attribute of 
the mm-wave-phy-common class. The numerology parameter should be set along with
the bandwidth in order to allow correct configuration of all numerologies parameters of mm-wave-phy-mac-common.
The numerologies parameters that are being configured at a run-time based on the configuration of "Numerology" and 
"Bandwidth" attribute, and which cannot be anymore directly accessed through the attributes of mm-wave-phy-common class are:
symbol period, slot period, symbols per slot, subframes per frame, slots per subframe, subcarriers per PRB, 
the number of PRBs, and the subcarrier spacing.

According to the current design that temporarily follows some of NYU 'mmwave' module design choices, 
the bandwidth and numerology attributes are currently placed in mmwave-phy-mac-common class. 
However, in order to allow that the numerology or FDM of numerologies is NR device specific, 
and the PHY an MAC parameters shall belong to mmwave-enb-net-device and mmwave-ue-net device.

3GPP channel model attributes
=======================================

In the terms of 3GPP channel model we leverage on NYU implementation which is 
according the 3GPP channel model reported by 3GPP in [TR38900]_.
NYU 3GPP channel model implements the channel model for frequencies of 
the 6-100 GHz band and associated MIMO beamforming architecture. The description 
of the implementation may be found in [ns-3-3gpp-cm]_.

The 3GPP channel model attributes are specified in classes MmWave3gppChannel, 
MmWave3gppPropagationLossModel and MmWave3gppBuildingsPropagationLossModel.
These attributes are listed in Tables :ref:`tab-3gpp-channel-attributes`, 
:ref:`tab-3gpp-propagation-loss-attributes` and :ref:`tab-3gpp-buildings-propagation-loss`.


.. _tab-3gpp-channel-attributes:

.. table:: 3GPP channel model attributes that can be configured in MmWave3gppChannel

        ====================   ======================================================================================================================================       ================== 
	Name 		       Description 										                                                    Default value	
        ====================   ======================================================================================================================================       ================== 	
	UpdatePeriod     		Enable spatially-consistent UT mobility modeling procedure A, the update period unit is in ms, set to 0 ms to disable update		0
	CellScan         		Whether to use search method to determine beamforming vector, the default is long-term covariance matrix method				false
	Blockage         		Enable blockage model A (sec 7.6.4.1)													false
	NumNonselfBlocking       	The number of non-self-blocking regions													4
	BlockerSpeed			The speed of moving blockers, the unit is m/s												1
	PortraitMode                    True for portrait mode, false for landscape mode											true
        ====================   ======================================================================================================================================       ================== 


.. _tab-3gpp-propagation-loss-attributes:

.. table:: 3GPP propagation loss attributes that can be configured in MmWave3gppPropagationLossModel

        ====================   ======================================================================================================================================       ================== 
	Name 		       Description 										                                                    Default value	
        ====================   ======================================================================================================================================       ================== 
	Frequency 		        The carrier frequency (in Hz) at which propagation occurs  									        28 GHz
	MinLoss                         The minimum value (dB) of the total loss, used at short ranges										0
	ChannelCondition                'l' for LOS, 'n' for NLOS, 'a' for all													a
	Scenario			The available channel scenarios are 'RMa', 'UMa', 'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen', 'InH-ShoppingMall'		RMa
	OptionalNlos			Whether to use the optional NLoS propagation loss model											false
	Shadowing			Enable shadowing effect															true
	InCar				If inside a vehicle, car penetration loss should be added to propagation loss								false
        ====================   ======================================================================================================================================       ================== 
        

.. _tab-3gpp-buildings-propagation-loss:

.. table:: 3GPP buildings propagation loss attributes that can be configured in MmWave3gppBuildingsPropagationLossModel

        ====================   ======================================================================================================================================       ================== 
	Name 		       Description 										                                                    Default value	
        ====================   ======================================================================================================================================       ================== 
	Frequency 		        The carrier frequency (in Hz) at which propagation occurs  									        28 GHz
        UpdateCondition                 Whether to Update LOS/NLOS condition while UE moves											true
        ====================   ======================================================================================================================================       ================== 


What classes hold attributes, and what are the key ones worth mentioning?

Output
***********************************

What kind of data does the model generate?  What are the key trace
sources?   What kind of logging output can be enabled?




Validation
------------

Tests
***********************************
To validate the implemented features, we have designed different tests.

NR test for new NR frame structure and numerologies configuration
==============================================================================
Test case ``mmwave-system-test-configurations`` validates that the NR frame structure is correctly 
configured by using new configuration parameters. 
This is the system test that is validating the configuration of 
different numerologies in combination with different schedulers. 
The test provides the traces according to which can be checked whether 
the gNB and UE clocks perform synchronously according the selected numerology, 
and that serialization and deserialization of the frame, subframe, slot and TTI number 
performs correctly for the new NR frame structure.

Test of packet delay in NR protocol stack 
==============================================================================

Test case ``mmwave-test-numerology-delay`` validates that the delays of a single UDP packet are correct. 
UDP packet is monitored at different points of NR protocol stack, 
at gNB and UE. The test checks whether the delay corresponds to
configuration of the system for different numerologies. 

.. _fig-protocol-stack:

.. figure:: figures/protocol-stack.*
   :align: center
   :scale: 60%

   Performance evaluation of packet delay in NR protocol stack 

The test monitors delays such as, eNB processing time, air time, UE time, etc.
The test fails if it detects unexpected delay in the NR protocol stack.

The topology consists of 1 gNB and 1 UE, and considers only the Radio Access Network (RAN). 
The distance between the gNB and UE is 10 meters, 
and the heights are: h_BS=10 m and h_UT=1.5 m. A single packet of 1000 bytes is directly 
injected to gNB device and configured to have the destination address of the UE. 
The test supports also the possibility to inject various amounts of packets, or to change size of packets.
The propagation model is the 3GPP channel model, without buildings. 
The central frequency is 28 GHz and the bandwidth is 400 MHz.
Other parameters of 3GPP model are the following:


MinLoss = 0.0 

ChannelCondition = 'a'

Scenario = Rma

OptionalNlos = false

InCar = false

The test checks that the "injected packet" is passed through the NR stack in the following way:

1. Packet arrives to gNB at time t

2. gNB PDCP forwards immediately packet to RLC at time t

3. gNB RLC immediately informs MAC that there is new packet in queue via BSR at time t

4. gNB MAC schedules the packet and informs RLC of TXOP at time t

5. gNB RLC sends packets, transmits it to MAC t

6. gNB MAC pass the packet to PHY, and packet is untouched at PHY until t + L1L2 delay (this is how is simulated this delay)

7. gNB PHY starts to transmit the PDU at time t + L1L2 delay + CTRL duration (symbol)

8. UE PHY finished reception at time t + L1L2 delay + CTRL duration (symbol duration) + DATA duration (number of symbols * symbol duration) = t_reception_finished

9. UE decodes the packet and pass it to MAC at time t_reception_finished + TB decode delay (100 micro seconds currently set by default)

10. UE MAC pass immediately the PDU to RLC at time t_reception_finished + TB decode delay (100 micro seconds currently set by default)

11. RLC waits to receive all PDUs of corresponding packet and then it pass it to PDCP t_reception_finished (last PDU) + TB decode delay (100 micro seconds currently set by default)


The test passes if all of the previous steps are according to the timings related to a specific numerology. The test is run for different numerologies and the output of the test is as follows:

 Numerology:0	 Packet of :1000 bytes	#Slots:1	#Symbols:3	Packet PDCP delay:2385712	RLC delay of first PDU:+2385712.0ns	MCS of the first PDU:1 

 Numerology:1	 Packet of :1000 bytes	#Slots:1	#Symbols:5	Packet PDCP delay:1314284	RLC delay of first PDU:+1314284.0ns	MCS of the first PDU:1 

 Numerology:2	 Packet of :1000 bytes	#Slots:1	#Symbols:9	Packet PDCP delay:778570	RLC delay of first PDU:+778570.0ns	MCS of the first PDU:1 

 Numerology:3	 Packet of :1000 bytes	#Slots:2	#Symbols:17	Packet PDCP delay:528569	RLC delay of first PDU:+466069.0ns	MCS of the first PDU:1 

 Numerology:4	 Packet of :1000 bytes	#Slots:3	#Symbols:34	Packet PDCP delay:399105	RLC delay of first PDU:+283034.0ns	MCS of the first PDU:1 

 Numerology:5	 Packet of :1000 bytes	#Slots:6	#Symbols:69	Packet PDCP delay:341070	RLC delay of first PDU:+191516.0ns	MCS of the first PDU:1

Test of numerology FDM 
==============================================================================
To test the FDM of numerologies, we have implemented 
the ``MmWaveTestFdmOfNumerologiesTestSuite``, in which the gNB is configured to operate with 
2 BWPs. The test checks if the achieved throughput of a flow over a specific BWP is proportional to the 
bandwidth of the BWP through which it is multiplexed.

Test for NR schedulers
==============================================================================
To test the NR schedulers, we have implemented a system test called ``MmWaveSystemTestSchedulers`` whose purpose is to test that the 
NR schedulers provide a required amount of resources to all UEs, for both cases, the downlink and the uplink. The topology consists of a single gNB and 
variable number of UEs, which are distributed among variable number of beams. Test cases are designed in such a way that the offered rate for the flow 
of each UE is dimensioned in such a way that each of the schedulers under the selected topology shall provide at least the required service to each of the UEs.
The system test suite for NR schedulers creates a various number of test cases that check different system configuration by choosing 
different number of UEs, number of beams, numerology, traffic direction (DL, UL, DL and UL), modes of scheduling (OFDMA and TDMA) and 
different scheduling algorithms (RR, PR, MR).


Test for OFDMA
==============================================================================
Test case called ``MmWaveSystemTestOfdma`` validates that the NR scheduling in OFDMA mode provides expected interference situations in the scenario. 
The test consists of a simple topology where 2 gNBs transmit to 2 UEs, and these two simultaneous transmissions are on the same transmission directions, the opposite transmission directions or 
 the perpendicular transmission directions.
Test configures a new simple test scheduler to use only a limited portion of RBGs for scheduling. Test case where assigned RBGs for gNBs overlap is 
expected to affect the MCS and performance, while in the case when RBGs assigned to gNBs are not overlapping shall be the same as in the case when only a 
single gNB is transmitting.


References
------------

.. [TR38912] 3GPP TR 38.912 "Study on New Radio (NR) access technology", (Release 14) TR 38.912v14.0.0 (2017-03), 3rd Generation Partnership Project, 2017.

.. [mmwave-module] NYU WIRELESS, University of Padova, "ns-3 module for simulating mmwave-based cellular systems," Available at https://github.com/nyuwireless/ns3-mmwave.

.. [TR38900] 3GPP TR 38.900 "Study on channel model for frequency above 6GHz", (Release 14) TR 38.912v14.0.0 (2016-12), 3rd Generation Partnership Project, 2016.

.. [ns-3-3gpp-cm] Menglei Zhang, Michele Polese, Marco Mezzavilla, Sundeep Rangan, Michele Zorzi, "ns-3 Implementation of the 3GPP MIMO Channel Model for Frequency Spectrum above 6 GHz", pages: 71-78, 	doi 10.1145/3067665.3067678, 17 Proceedings of the Workshop on ns-3, 2017.

.. [end-to-end-mezz] Marco Mezzavilla, Menglei Zhang, Michele Polese, Russell Ford, Sourjya Dutta, Sundeep Rangan, Michele Zorzi, "End-to-End Simulation of 5G mmWave Networks,", in IEEE Communication Surveys and Tutorials, vol. 13, No 20,  pp. 2237-2263, April 2018.

.. [WNS32018-NR]  B. Bojovic, S. Lagen, L. Giupponi, Implementation and Evaluation of Frequency Division Multiplexing of Numerologies for 5G New Radio in ns-3 , in Workshop on ns-3, June 2018, Mangalore, India.

.. [CAMAD2018-NR] N. Patriciello, S. Lagen, L. Giupponi, B. Bojovic, 5G New Radio Numerologies and their Impact on the End-To-End Latency , in Proceedings of IEEE International Workshop on Computer-Aided Modeling Analysis and Design of Communication Links and Networks (IEEE CAMAD), 17-19 September 2018, Barcelona (Spain). 

.. [3GPPTSGSSA] 3GPP TS 23.501 V15.0.0, System Architecture for the 5G System; Stage 2 (Release 15), Dec. 2017

.. [CA-WNS32017] B. Bojovic, D. Abrignani Melchiorre, M. Miozzo, L. Giupponi, N. Baldo, Towards LTE-Advanced and LTE-A Pro Network Simulations: Implementing Carrier Aggregation in LTE Module of ns-3, in Proceedings of the Workshop on ns-3, Porto, Portugal, June 2017.

.. [TS38214] 3GPP TS 38.214 V15.2.0 (2018-06), "NR; Physical layer procedures for data", (Release 15), V15.2.0 (2018-06), 3rd Generation Partnership Project, 2018

.. [ff-api] FemtoForum , "LTE MAC Scheduler Interface v1.11", Document number: FF_Tech_001_v1.11 , Date issued: 12-10-2010.


