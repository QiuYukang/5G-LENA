.. SPDX-License-Identifier: NIST-Software

.. _NrV2xTests:

Tests for sidelink (mode 2) sensing
===================================

The test suite labeled ``nr-sl-sensing`` provides unit tests on the operation of the sensing algorithm.  The sensing code in ``NrSlUeMac`` has been written to permit the unit testing of the method ``NrSlUeMac::GetNrSlCandidateResourcesPrivate(),`` which implements the logic of Section 8.1.4 of [TS38214]_.

The main data that is provided to this algorithm is the sensing data, transmit history, and list of candidate slots within the resource selection window.  The algorithm produces a list of (filtered) candidate resources based on additional parameters passed into the method, such as the requested number of contiguous subchannels for the selected candidate resources.  The unit tests are able to create specific test conditions by creating artificial sensing data and transmit history and passing it to the algorithm, and then by inspecting the resulting candidate resources.  In addition, enabling the ns-3 log component ``NrSlSensingTest`` will allow details of the sensing data, transmit history, and operation of the algorithm to be provided as logging output.

The first test reuses the basic configuration of the example ``cttc-nr-v2x-demo-simple.cc`` but with a resource pool consisting of two subchannels instead of one, and checks the following:

1.  The test artificially injects sensing data into the first of two subchannels in all slots within the sensing window, and this artificial data is provided to the algorithm with a request to find resources that are one subchannel wide.  The resulting candidates are inspected to check that the expected number of sidelink slots in the selection window are available and that only the upper subchannel is listed as a candidate resource in each slot.

2.  The test repeats the previous test with the previous sensing data, but the request for candidate resources instead requests candidates that are two subchannels wide.  As a result, the first pass through the algorithm will yield no candidates, because the sensing data presence in the lower subchannel blocks the inclusion of each slot.  However, in this case, the algorithm will raise the RSRP threshold by 3 dB and try again, and the second attempt will succeed to find candidates in all slots.  This output condition is checked.

3.  The test repeats the previous test but with different sensing data, occupying only two slots within the sensing window, and the algorithm is requested to find candidate resources.  The test checks that the candidate resources do not include slots that correspond to the projection of the slots with sensing data into the selection window, based on the configured RRI of 100 ms.

4.  The test repeats the previous test but with no sensing data.  In its place, an artificial transmit history is injected that specifies that transmissions took place in the same two slots within the sensing window.  The test code again checks that the corresponding slots in the selection window (projected forward by the RRI of 100 ms), are not candidates.

5.  A separate test is provided to check that the method ``NrSlUeMac::RemoveOldSensingData()`` correctly removes old sensing data.

Tests for sidelink HARQ
=======================

The test suite labeled ``nr-test-sidelink-harq`` sets up a small network of sidelink UEs with small test applications to generate traffic.  It instruments trace sources to check for specific outcomes induced by the traffic sending pattern and an error model that can force reception drops to test HARQ operation.

The following test cases are implemented:

1.  Check basic broadcast, groupcast, and unicast HARQ configurations with no packet losses and the number of UEs varying from 2 to 9

2.  Check that when HARQ is disabled by TFT configuration that no HARQ feedback is sent despite the presence of a PSFCH channel.

3.  Check that when HARQ is enabled but PSFCH period is zero (no PSFCH channel), that blind retransmissions are used

4.  Check the basic operation of HARQ under different TDD and SL bitmap patterns and different numerologies (0, 1, 2, 3, 4).

5.  Check behavior for PSSCH loss without HARQ enabled

6.  Check behavior for both dynamic and SPS grants under no loss conditions

7.  Check behavior for both dynamic and SPS grants when SCI-1 is lost (there should be no HARQ feedback)

8.  Check behavior for both dynamic and SPS grants when SCI-2A is lost (there should be no HARQ feedback)

9.  Check behavior for both dynamic and SPS grants when (HARQ timeouts and process ID deallocations) when both TBs are lost (HARQ feedback occurs)

10.  Check behavior for both dynamic and SPS grants when (HARQ timeouts and process ID deallocations) when the first TB is lost (HARQ NACK) but second TB is received successfully (HARQ ACK)

11.  Check that IPv4 and IPv6 behavior is the same for a few cases.

12.  Check that when two senders collide (collision of TB upon reception), that HARQ retransmissions recover the TBs, for unicast, groupcast, and broadcast.

13.  Check the assignment of PSFCH slots to the overall SL pattern.

Tests for the fixed MCS scheduler
=================================

A number of tests of the default fixed MCS scheduler have been defined in the test program ``nr-sl-simple-multi-lc-example-tests.cc``.  This test uses the ns-3 `ExampleAsTestSuite` framework to run the example program in different ways and checks the output of the program against reference output stored in the test directory `sl-test-data`.

The following tests are implemented:

1.  The first test is dynamic grants, groupcast, HARQ enabled.  The output demonstrates that all three LCs are scheduled in the same grant (going to the same destination), and that three transmissions can be fit into the selection window.

2.  The next test is dynamic grants, groupcast, HARQ disabled.  The output demonstrates that all three LCs are scheduled in the same grant, but only one transmission is scheduled in the grant.

3.  Two configurations demonstrate the prioritization operation when there is no LC prioritization configured.  schedTypeConfig 3 configures dynamic grants on flows 1 and 2, and SPS grants on flow 3.  By default, the variable `prioToSps` is false, and as a result, the first grant scheduled is the dynamic grant for LCs 4 and 5. When the `prioToSps` flag is true, the SPS grant (LC 6) is scheduled first.

4.  The next tests check the assignment of flows to logical channels.  In the first such test, when the `dstL2IdConfig` parameter is set to 3, the first flow to `dstL2Id=2` will be sent as unicast, the second to `dstL2Id=254` will be sent as groupcast, and the third to `dstL2Id=255` will be sent as broadcast.  This will cause all flows to have to use a separate LC.  The `priorityConfig` value of 2 will cause the broadcast flow to be scheduled with highest priority.

5.  In the second test of LC assignment, when the `priorityConfig` value set to 3, the groupcast and unicast flow will have equal priority value of 2, above that of the broadcast (1).  Which one is selected will depend on a random variable draw.  With `RngRun=1`, the groupcast (`dstL2Id=254`) will be scheduled first, while with `RngRun=2`, the unicast one (`dstL2Id=4`) will be scheduled first.

6.  The final test illustrates that use of a non-uniform RRI among all flows will prevent all LCs from being scheduled in the same grant.  `rriConfig=2` and `schedTypeConfig=2` will allow LCIDs 4 and 6, but not 5 (flow 2) to be scheduled in the same grant.
