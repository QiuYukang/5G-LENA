<!--
/**
 * \page changes Changes
 * \brief Changelog of 5G-LENA
 */
-->

Changes
=======

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

---

## Changes from NR-v0.0 to NR-v0.1

We did not track changes since the beginning. However, the main features we added to the mmWave module (from which we forked) are the FDM of numerologies and a new and improved MAC scheduling layer.

### New API:

* ND

### Changes to existing API:

* ND

### Changed behavior:

* ND
