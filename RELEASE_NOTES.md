5G-LENA Release Notes                         {#releasenotes}
=====================

This file contains release notes for the NR module (most recent releases first).  

All of the ns-3 documentation is accessible from the ns-3 website:
http://www.nsnam.org including tutorials: http://www.nsnam.org/tutorials.html

Consult the file CHANGES.md for more detailed information about changed
API and behavior across releases.

Release NR-dev
--------------

Availability
------------
This release is not yet available.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note:  not all features available on all platforms):

- g++-4.9 or later
- Apple LLVM version 7.0.2 or later
- clang-3.3 or later

In addition, Python 2.7 (Python 2 series) or Python 3.4-3.7 (Python 3 series)

This release has been tested on the following platforms:
- ArchLinux with g++-9.2.1, clang-9.0.1, and Python 3.8.1
- Ubuntu 18.04 (64 bit) with g++-8.3.0 and Python 2.7.12/3.5.2

Important news
--------------
- This release is aligned with CTTC's ns-3-dev commit id
8b9decb34 lte: Add enum for TDD slot type  [ZorazeAli]
that is on top of the nsnam ns-3-dev master commit id
ea3506784 Merge branch 'gsoc-2019-tommaso'  [Natale Patriciello]
To upgrade CTTC's ns-3-dev, please run the following (save any non-official
commit, as they will be deleted):

$ cd /path/to/cttc/ns-3-dev
$ git reset --hard HEAD~200
$ git pull

This module can be updated with the usual

$ git pull


New user-visible features (old first)
-------------------------

- The control message timings (from PHY to MAC or from MAC to PHY) can be adjusted
  in a flexible manner according to each release specifications.
- Processing delays N0, N1, N2 are introduced as attributes of MmWaveEnbPhy.
  The values K0, K1, K2 are then calculated and communicated to the UE in the DCI.
- The UlSchedDelay parameter is replaced by N2Delay.
- UE receives DL data according to K0 and sends UL data according to K2
  (passed from the gNb in the DL and UL DCI, respectively). Moreover,the DL HARQ
  Feedback is scheduled according to K1 delay (passed from the gNb to the UE in
  the DL DCI).
- UE-side control messages could be scheduled with Ul Data. This behavior is now
  changed and therefore control messages are sent only in Ul Ctrl.
- Patterns of size different from 10 (less or greater) are now supported.
- Ported the code to the ThreeGppChannel of ns-3-dev, developed under the GSoC 2019
  framework.
- A new Component Carrier/Bandwidth Part helper has been added in file `cc-bwp-helper.h`.
  With this class, it is easy to divide the spectrum in different regions.
- MmWaveHelper is now refactored to take into account Multi-Cell Configurations
  (i.e., different configuration for different cells).
- Introduced FDD operational mode for a Bandwidth Part.
- Removed PhyMacCommon. Its attributes are now divided among different classes.
  Please check CHANGES.md for the list.
- Separated NrEesmErrorModel in four different classes: NrEesmIrT1, NrEesmIrT2,
  NrEesmCcT1, NrEesmCcT2. These classes encapsulate the properties (harq method, table) that
  were an attribute of NrEesmErrorModel.
- Added an attribute to the MmWaveEnbPhy and MmWaveUePhy to set the bandwidth overhead
  to keep in consideration when calculating the number of usable RB. By default,
  it is now set to 0.04, while before it was set to 0.0 (0.0 means that there are
  no guard bands, and the entire bandwidth is usable).
- Added the LENA error model. To be used only in conjunction with a OFDMA scheduler,
  and without beams.
- N1 default value is now 2 slots. Before it was 4 slots.
- Renamed all mmwave- classes, tests, examples, helpers, to nr-.
- Added SHORT_BSR as MAC CE element, that goes with MAC data, and is evaluated
  by the error model upon delivery.

Bugs fixed
----------
- Removed cttc-nr-tdd-cc-bwp-demo.cc since it was redundant. The simillar configuration is 
present in cttc-nr-cc-bwp-demo.cc example.
- Removed legacy and invalid mmwave-* examples, that were inherited from mmWave codebase.
- The code performing LBT at UE side always assumed a DL CTRL symbol inside a slot.
  With TDD, that may not happen, and the code has been updated to not
  always assume a DL CTRL.
- TDMA Scheduler was assuming that there are always UE to schedule. That may not be true
  if these UEs have scheduled an HARQ retransmission (and hence unable to
  receive a new data DCI). Code updated to remove this assumption.
- Enb was scheduling first DL and then UL, so when it was calculating the symbols to be
  assigned to DL, it was not taking into account the UL symbols. Code has been updated to
  schedule first UL and then DL.
- N1 (used to schedule DL HARQ Feedback) could not be set to zero, because the way the
  calculation was carried out was resulting in negative delay. Calculation has been updated
  to take into account the case of N1=0.
- The Harq timer is now set to 0 every time a retransmission is scheduled and
  transmitted (before, it was just incremented)
- The BwpManagerGnb now redirects BSR and SR to the source bandwidth part, as
  the UE has already done the selection based on the configured QCI.
- LBT at the gNb side is now scheduled (and done) every time there is a DL CTRL
  to transmit, as in FDD configuration, other BWP can inject messages after
  the start of the slot.
- NrLteErrorModel contained a bug that prevented the calculation of the right
  error value.
- Scheduler contained a bug that was forcing, in some situation, a double SR
  scheduling, stopped by a FATAL_ERROR.

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr


Release NR-v0.4
--------------

Availability
------------
Available since Thu Feb 13 2020.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note:  not all features available on all platforms):

- g++-4.9 or later
- Apple LLVM version 7.0.2 or later
- clang-3.3 or later

In addition, Python 2.7 (Python 2 series) or Python 3.4-3.7 (Python 3 series)

This release has been tested on the following platforms:
- ArchLinux with g++-9.2.1, clang-9.0.1, and Python 3.8.1
- Ubuntu 18.04 (64 bit) with g++-8.3.0 and Python 2.7.12/3.5.2

Important news
--------------
- This release is aligned with CTTC's ns-3-dev commit id
217b410c lte-rlc: TM is now sending more than one packet per transmission opportunity
that is on top of the nsnam ns-3-dev master commit id
578c107eb internet: fix packet deduplication test .
To upgrade CTTC's ns-3-dev, please run the following (save any non-official
commit, as they will be deleted):

$ cd /path/to/cttc/ns-3-dev
$ git reset --hard HEAD~200
$ git pull

This module can be updated with the usual

$ git pull


New user-visible features (old first)
-------------------------------------

- (error model) new BLER-SINR tables for MCS table1 and table2
- (performance) Various performance improvements


Bugs fixed
----------
- (scheduler) Correctly schedule beams of users that got a HARQ retx space.

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr


Release NR-v0.3
--------------

Availability
------------
Available since Tue Aug 27 2019.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note:  not all features available on all platforms):

- g++-4.9 or later
- Apple LLVM version 7.0.2 or later
- clang-3.3 or later

In addition, Python 2.7 (Python 2 series) or Python 3.4-3.7 (Python 3 series)

This release has been tested on the following platforms:
- ArchLinux 2018.10.08 with g++-8.2.1 and Python 3.7.1
- Ubuntu 16.04 (64 bit) with g++-5.4.0 and Python 2.7.12/3.5.2

New user-visible features (old first)
-------------------------

- (error model) Added the NrEesmErrorModel class. It models the NR PHY
abstraction according to LDPC coding, block segmentation, and including
MCS/CQI table 1 and 2. The user can select the HARQ method (HarqCc or HarqIr)
as well as the MCS/CQI table of NR to be used (McsTable1 or McsTable2), through
new attributes. In this release, the BLER-SINR tables are not completed yet,
and so it is recommended not to use this error model.

- (3gpp channel model) 3gppChannelModel can now be used by any other module,
i.e. it is not any more mmwave specific spectrum propagation model. This means
that any subclass of NetDevice can be attached to a channel using this
SpectrumPropagationModel. An additional requirement is that the technology uses
AntennaModel that is implementing AntennaArrayBasicModel interface. This
feature is a basic prerequisite for simulating co-existence of different
technologies that are using the 3gpp channel model implementation.

- (3gpp channel model) The beamforming phase has been extracted from the model,
and it is now a duty of the NetDevice. The gNB phy has now a new attribute to
configure the periodicity of the beamforming. Please note that it is still ideal,
i.e., it does not require any simulated time to be performed.

- (3gpp channel model) gNB-gNB and UE-UE pathloss and channel computation can be
allowed (through a new attribute) for Indoor Hotspot scenarios.

- (spectrum phy) gNB-gNB and UE-UE interferences can be enabled (through a new
attribute).

- (RRC) Now all carriers are registered to RRC, to transmit system information
through all the bandwidth parts.

- (SCHED) The scheduler now is informed of RACH preamble messages, and reserve
some space in the DL CTRL symbol to send the RAR messages.

- Added traces that indicate the transmission or the reception of CTRL
messages. For instance, take a look to *EnbMacRxedCtrlMsgsTrace* or
*EnbMacTxedCtrlMsgsTrace* in the Gnb MAC file.

Bugs fixed
----------
- (scheduler) Fixed the use of a static MCS value in the schedulers
- (spectrum-phy) While looping the packets in the packet burst,
  to send the feedback, extract the RNTI for each loop. Before, it was
  asserting when the RNTI changed. The code does not depend on the
  RNTI previous values, so it should be safe.
- (phy) At the beginning, fill some slot (number configured by UL sched delay
  param) with UL CTRL symbol.

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr


Release NR-v0.2
---------------

Availability
------------
Available since Fri Feb 1 2019.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note:  not all features available on all platforms):

- g++-4.9 or later
- Apple LLVM version 7.0.2 or later
- clang-3.3 or later

In addition, Python 2.7 (Python 2 series) or Python 3.4-3.7 (Python 3 series)

This release has been tested on the following platforms:
- ArchLinux 2018.10.08 with g++-8.2.1 and Python 3.7.1
- Ubuntu 16.04 (64 bit) with g++-5.4.0 and Python 2.7.12/3.5.2

New user-visible features (old first)
-------------------------

- (mmwave) Removed any reference to Rlc Low Latency
- (mmwave) The code that was previously under the directory src/mmwave has been
  moved to src/nr.
- (nr) Aligned ComponentCarrierGnb, MmWaveEnbMac, MmWaveUePhy to ns-3-dev
- (nr) Aligned the BwpManager and the various helper/example to the bearer
  definitions in Rel. 15
- (nr) Removed unsupported MmWaveBeamforming and MmWaveChannelMatrix classes.
- (nr) Added a 3GPP-compliant antenna class.
- (nr) Added a 3GPP-compliant UL scheduling request feature.

Bugs fixed
----------

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr
