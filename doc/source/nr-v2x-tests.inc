.. SPDX-License-Identifier: NIST-Software

.. _NrV2xTests:

Test for V2X sensing
====================

The test suite labeled ``nr-sl-sensing`` provides unit tests on the operation of the sensing algorithm.  The sensing code in ``NrSlUeMac`` has been written to permit the unit testing of the method ``NrSlUeMac::GetNrSlCandidateResourcesPrivate(),`` which implements the logic of Section 8.1.4 of [TS38214]_.

The main data that is provided to this algorithm is the sensing data, transmit history, and list of candidate slots within the resource selection window.  The algorithm produces a list of (filtered) candidate resources based on additional parameters passed into the method, such as the requested number of contiguous subchannels for the selected candidate resources.  The unit tests are able to create specific test conditions by creating artificial sensing data and transmit history and passing it to the algorithm, and then by inspecting the resulting candidate resources.  In addition, enabling the ns-3 log component ``NrSlSensingTest`` will allow details of the sensing data, transmit history, and operation of the algorithm to be provided as logging output.

The first test reuses the basic configuration of the example ``cttc-nr-v2x-demo-simple.cc`` but with a resource pool consisting of two subchannels instead of one, and checks the following:

1.  The test artificially injects sensing data into the first of two subchannels in all slots within the sensing window, and this artificial data is provided to the algorithm with a request to find resources that are one subchannel wide.  The resulting candidates are inspected to check that the expected number of sidelink slots in the selection window are available and that only the upper subchannel is listed as a candidate resource in each slot.

2.  The test repeats the previous test with the previous sensing data, but the request for candidate resources instead requests candidates that are two subchannels wide.  As a result, the first pass through the algorithm will yield no candidates, because the sensing data presence in the lower subchannel blocks the inclusion of each slot.  However, in this case, the algorithm will raise the RSRP threshold by 3 dB and try again, and the second attempt will succeed to find candidates in all slots.  This output condition is checked.

3.  The test repeats the previous test but with different sensing data, occupying only two slots within the sensing window, and the algorithm is requested to find candidate resources.  The test checks that the candidate resources do not include slots that correspond to the projection of the slots with sensing data into the selection window, based on the configured RRI of 100 ms.

4.  The test repeats the previous test but with no sensing data.  In its place, an artificial transmit history is injected that specifies that transmissions took place in the same two slots within the sensing window.  The test code again checks that the corresponding slots in the selection window (projected forward by the RRI of 100 ms), are not candidates.

5.  A separate test is provided to check that the method ``NrSlUeMac::RemoveOldSensingData()`` correctly removes old sensing data.
