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
- ArchLinux 2018.10.08 with g++-8.2.1 and Python 3.7.1
- Ubuntu 16.04 (64 bit) with g++-5.4.0 and Python 2.7.12/3.5.2

New user-visible features (old first)
-------------------------

- (error model) Added the NrEesmErrorModel class. It models the NR PHY
abstraction according to LDPC coding, block segmentation, and including
MCS/CQI table 1 and 2.

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
