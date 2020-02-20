5G-LENA Changes       {#changes}
===============

NR module: API and model change history
---------------------------------------

<!-- This ChangeLog is updated in the reverse order with the most recent changes coming first.  Date format:  DD-MM-YYYY -->

ns-3 is an evolving system and there will be API or behavioral changes
from time to time.   Users who try to use scripts or models across
versions of ns-3 may encounter problems at compile time, run time, or
may see the simulation output change.

We have adopted the development policy that we are going to try to ease
the impact of these changes on users by documenting these changes in a
single place (this file), and not by providing a temporary or permanent
backward-compatibility software layer.

A related file is the RELEASE_NOTES.md file in the top level directory.
This file complements RELEASE_NOTES.md by focusing on API and behavioral
changes that users upgrading from one release to the next may encounter.
RELEASE_NOTES attempts to comprehensively list all of the changes
that were made.  There is generally some overlap in the information
contained in RELEASE_NOTES.md and this file.

The goal is that users who encounter a problem when trying to use older
code with newer code should be able to consult this file to find
guidance as to how to fix the problem.  For instance, if a method name
or signature has changed, it should be stated what the new replacement
name is.

Note that users who upgrade the simulator across versions, or who work
directly out of the development tree, may find that simulation output
changes even when the compilation doesn't break, such as when a
simulator default value is changed.  Therefore, it is good practice for
_anyone_ using code across multiple ns-3 releases to consult this file,
as well as the RELEASE_NOTES.md, to understand what has changed over time.

This file is a best-effort approach to solving this issue; we will do
our best but can guarantee that there will be things that fall through
the cracks, unfortunately.  If you, as a user, can suggest improvements
to this file based on your experience, please contribute a patch or drop
us a note on ns-developers mailing list.

---

## Changes from NR-v0.4 to NR-dev

### New API:

### Changes to existing API:
* Functions MmWaveEnbPhy::ReceiveUlHarqFeedback and MmWaveLteUePhy::ReceiveLteDlHarqFeedback
are renamed to MmWaveLteEnbPhy::ReportUlHarqFeedback and MmWaveLteUePhy::EnqueueDlHarqFeedback,
respectively to avoid confusion. mmwave-helper callbacks are updated accordingly

* Removed attribute NumberOfComponentCarriers and UseCa from MmWaveHelper.

### Changed behavior:

---

## Changes from NR-v0.3 to NR-v0.4

### New API:

### Changes to existing API:

* Replaced SINR-BLER curves for MCS Table1 and Table2 of the NR error model
  with more accurate (and complete) values.

### Changed behavior:

* In the DCI message,  the gNb reports the K{0,2} delay instead of the
sfn number.

---

## Changes from NR-v0.2 to NR-v0.3

### New API:

* Added NrErrorModel as interface for any error model. It is used by NrAmc and
MmWaveSpectrumPhy.
* Added attribute *ErrorModelType* to the class NrAmc, which must be kept in
sync with the attribute *ErrorModelType* of MmWaveSpectrumPhy
* Added attribute *ErrorModelType* to the class MmWaveSpectrumPhy
* Added class NrEesmErrorModel to model the NR PHY abstraction according to LDPC
coding, block segmentation, and including MCS/CQI table 1 and 2. It has an attribute
to select the HARQ method (HarqCc or HarqIr) and another attribute to select the
MCS/CQI table of NR to be used (McsTable1 or McsTable2). In this release, the
BLER-SINR are not completed yet, and so it is recommended not to use this model.
* Added attributes AntennaNumDim1, AntennaNumDim2, AntennaArrayType to
MmWaveEnbPhy and MmWaveUePhy. And these two classes are responsible for an
antenna array installation. This means that there is an instance of
AntennaArrayBasicModel per MmWavePhy.
* MmWavePhy has new functions Set/Get functions for configuring and
obtaining AntennaNumDim1, AntennaNumDim2, AntennaArrayType attributes. The
configuration of these parameters is allowed through MmWavePhy object since
we would like to have access to these attributes through the Config system
and that we can specify different values for an antenna for MmWaveUePhy and
MmWaveEnbPhy. Function to set AntennaArrayType can be called only for the
initial configuration. If it is called after the antenna object is created the
program will be halted and the user will be informed of the error.
* AntennaArray3gppModel has new attribute RandomUeorientation. If set to true
the 3D antenna orientation will be generated randomly throwing a dice by
using a UniformRandomVariable.
* MmWave3gppChannel has a new attribute UpdateBeamformingVectorsIdeally which
determines if the update of the beamforming vectors will be ideal, i.e. if
the beamforming vectors will be adjusted every time that the channel is updated.
* AntennaArrayBasicModel has a new function GetBeamformingVectorUpdateTime
that returns the last time at which the beamforming vector toward a specified
device has been updated.
* EnbPhy now can perform beamforming, in a very initial form (every connected ue
will perform the beamforming, and it will take 0 simulated time). The periodicity
of this beamforming depends on the value of the new attribute *BeamformingPeriodicity*
of the class MmWaveEnbPhy
* MmWaveSpectrumPhy now has two more traces: TxCtrlTrace and TxDataTrace, to extract
information about how the channel is occupied
* Now MmWaveSpectrumPhy can enable interference for all links, also UE->UE and GNB->GNB,
configured through the attribute EnableAllInterferences.
* Now MmWave3gppChannel can enable generation of propagation losses for all links,
also UE->UE and GNB->GNB, configured through the attribute EnableAllChannels. This
feature, nowadays, applies *only* to Indoor Hotspot scenarios, for which the 3GPP pathloss
model is valid.
* Introduced Listen-Before-Talk after MAC. The interface of a channel access manager
is inside the file *nr-ch-access-manager.h*.

### Changes to existing API:

* Renamed MmWaveAmc into NrAmc
* Renamed MmWaveMiErrorModel in NrLteMiErrorModel, and adapted it to the new
NrErrorModel interface
* MmWave3ppChannelModel does not anymore have an attribute for
MmWavePhyMacCommon class and thus it does not have any more the corresponding
functions to Set/Get the corresponding instance, SetConfigurationParameters
and GetConfigurationParameters, respectively. It instead has a new attribute
that needs to be configured at the beginning of the simulation.
This attribute is Bandwidth. The two most important attributes of this class
are now CenterFrequency and Bandwidth.
* AntennaArrayBasicModel has now a pointer to a SpectrumModel.
* MmWaveSpectrumValueHelper.
* Common functions of MmWaveEnbPhy and MmWaveUePhy,
CreateTxPowerSpectralDensity and CreateNoisePowerSpectralDensity are moved to
MmWaveUePhy and renamed to GetNoisePowerSpectralDensity and
GetTxPowerSpectralDensity to distinguish them from the original functions that
belong to MmWaveSpectrumValueHelper.
* MmWaveSpectrumValueHelper does not depend any more on MmWavePhyMacCommon
class. Functions that had up to know this class as an input, now have the
minimum subset of parameters that are used in the corresponding function.
* MmWaveSpectrumValueHelper does not have anymore empty function for
CreateTxPowerSpectralDensity that had as an input a power map (powerTxMap).
* MmWave3gppChannel can now be used by any other module, it is not any more
only mmwave specific spectrum propagation model. This means that any subclass
of NetDevice can be attached to a channel using this SpectrumPropagationModel.
An additional requirement is that the technology uses AntennaModel that is
implementing AntennaArrayBasicModel interface. The
dependencies from mmwave module-specific classes are removed, e.g. dependency on
MmWaveEnbNetDevice, MmWaveUeNetDevice, MmWaveUePhy, MmWaveEnbPhy.
* Removed MmWaveMacSchedulerNs3 attribute McsDefaultDl/McsDefaultUl. Its
functionality is now taken by the attribute StartingMcsDl/StartingMcsUl
* Renamed MmWavePointToPointEpcHelper into NrPointToPointEpcHelper. All
attributes that before were in the form "ns3::MmWavePointToPointEpcHelper"
must now be referred to "ns3::PointToPointEpcHelper". In fact, NrPointToPointEpcHelper
is now inheriting from PointToPointEpcHelper.
* AntennaArrayModel has the beamforming vector that emulates omni reception and
transmission.
* Removed attribute *BeamformingEnabled* and *UpdateBeamformingVectorsIdeally*
from MmWave3gppChannel.
* Removed attribute *EnbComponentCarrierManager* from MmWaveHelper. The only allowed
CCManager is now BwpManagerGnb.
* Removed attribute *UeComponentCarrierManager* from MmWaveHelper. The only allowed
manager is now UeManagerGnb.
* Removed attribute *Bandwidth* from MmWave3gppChannel.

### Changed behavior:
* BeamSearchBeamforming and LongTermCovMatrixBeamforming functions of
3gppChannelModel can now be called from the outside of this class. This means
that the update of the beamforming vectors can be triggered at any time and
does not have to be related to the channel update event.
* TxPower default value for a UE passed from 30.0 dBm to 2.0 dBm
* TxPower default value for a gNb passed from 30.0 dBm to 2.0 dBm
* MmWave3gppChannel supports calculation of the beamforming gain when one of
devices is in omni mode. This is possible because AntennaArrayModel has new
beamforming vector that emulates omni reception and transmission.
* The Slot Indication is asked to the MAC at the beginning of the slot, before
doing everything else at PHY level.
* UE and ENB PHY does not rely on the order of the allocation to understand if
an allocation is DL/UL, DATA/CTRL. Also, we support now the possibility to start
an allocation at a symbol != 0.
* The scheduler is now informed of RACH messages, even if (for the moment) it
does nothing to allocate space for UL RRC message, it reserve some ideal
space in the DL CTRL link to send the RAR response.
* The UE will ignore the UL CTRL slot if there are no messages to send. Practically speaking,
the UE will not set power on the RBG, not generating interference.
* The GNB will not put power during the DL CTRL symbol if there aren't messages
to send to the UEs.
* The control message timings are implemented in such a way that the delay can
be adjusted in a flexible manner. In particular, when a control message is passed
from PHY to MAC or from MAC to PHY, the user can adjust the delay according to each
release specifications. As default we apply a delay of 1 ms, exept of some cases
(e.g. RAR msg), where the delay is set to 2 ms.
WIP is focused on defining K0, K1, K2 and K3 delays according to TS 38.213.
The UlSchedDelay is replaced by K2Delay.
The UE PHY DL HARQ feedback is scheduled based to: (K1Delay - L1L2CtrlLatency) so that
we take into account the latency by the EnqueueCtrlMessage ().

---

## Changes from NR-v0.1 to NR-v0.2

### New API:

* A new AntennaArray3gppModel is introduced that inherits all the features of the AntennaArrayModel, but it considers 3GPP directional antenna elements instead of ISO antenna elements.
* 3gppChannelModel has a new attribute "speed" for configuring the speed. Previous and currently the default behaviour is that 3gppChannelModel calculates the relative speed between the transmitter and the receiver based on their positions. However, this parameter can be configured when the static scenario is being used but is desired to imitate small scale fading effects that would exist in a mobile scenario.
* New traces sources are added to the Interference class for collecting the SNR and RSSI values.

### Changes to existing API:

* The supported bearers QCI are only the ones defined in Rel. 15.
* The Ue NoiseFigure is not anymore hard-coded value, instead can be configured through the new MmWaveUePhy attribute called "NoiseFigure".
* MmWaveEnbDevice and MmWaveUeDevice have new attributes AntennaNumDim1 and AntennaNumDim2 that allow the configuration of a rectangular antenna. Previously only squared antenna was supported.
* GnbAntennaArrayModelType and UeAntennaArrayModelType attributes are added to MmWaveHelper that allow the configuration of the antenna type (ISO vs 3gppp) of gNb and UE, respectively.
* AntennaArrayModel has a new attribute AntennaOrientation that can be used to configure the antenna orientation which can be horizontal in XY plane, or vertical in ZY plane.
* The attributes _MmWaveEnbNetDevice::MmWaveEnbPhy_ and _MmWaveEnbNetDevice::MmWaveEnbMac_ have been removed as they are replaced by the component carrier map. GetPhy () and GetMac () for UE and gNB NetDevices are now deleted, instead it is necessary to use the version with the ccId.
* Times in _PhyMacCommon_ class are now expressed with ns3::Time.
Removed attribute _PhyMacCommon::WbCqiPeriod_ as it was unused.

### Changed behavior:
* The SR is now sent only for Component Carrier ID == 0. The Enb Mac will receive it and forward to the CC manager; then, the CCM will route it to the most appropriate MAC, so the assignation for the SR will be done on the appropriate CC.
* In any given slot, the scheduler will now ignore UEs with new data to transmit if it assigned resources for retransmitting an HARQ process.
*  Default value of the UpdatePeriod parameter of the MmWave3gppChannel is changed to be 1 ms. Note that this dramatically slows down the simulation execution since the channel matrix will be updated every 1 ms. Previously, the default parameter was 0, which means that unless configured differently the channel matrix was not being updated.
* Previously there was a single propagation loss model, no matter how many BWPs there are in the simulation. Now, there is a propagation loss model instance per BWP. Long story short. MmWaveHelper instantiates a channel instance per BWP. The channel of each BWP is configured by using its own instance of the MmWavePhyMacCommon channel parameters. Since different BWPs are on different frequencies each BWP shall have its own propagation loss model that is configured with the frequency of the corresponding BWP.
* The default value of the UpdatePeriod parameter of the MmWave3gppChannel is returned to 0 ms. This is because it is detected that there are many occasions when the update of the channel matrix is not needed in the simulation example or the test, hence when the update is enabled by default the execution time of these simulations unnecessarily is slowed down.
* The SINR traces in MmWaveSpectrumPhy are now printed based on the TB of the UE, and not the entire band as it was before.
* The scheduler now assigns the maximum amount of data that is possible to the various LCs. Before, if there was only one LC with only 3 bytes, the assignation would have been of 3 bytes, despite of the TBS. Now, all the TBS can be assigned to him. We also manage cases in which multiple LC or LCG are active, but (as before) only in a round-robin fashion.

---

## Changes from NR-v0.0 to NR-v0.1

We did not track changes since the beginning. However, the main features we added to the mmWave module (from which we forked) are the FDM of numerologies and a new and improved MAC scheduling layer.

### New API:

* ND

### Changes to existing API:

* ND

### Changed behavior:

* ND
