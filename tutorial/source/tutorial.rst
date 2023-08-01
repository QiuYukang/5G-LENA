.. Copyright (c) 2022 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
..
.. SPDX-License-Identifier: GPL-2.0-only

cttc-nr-demo Tutorial
---------------------

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Overview
********

This is a tutorial focused on the data plane operation of the example program `cttc-nr-demo`_
found in the ``examples/`` directory of the |ns3| ``nr`` module.  The objective of the tutorial is
to provide a detailed, layer-by-layer walk through of a basic NR example, with a focus on the
typical lifecycle of packets as they traverse the Radio Access Network (RAN).  The tutorial
points out all of the locations in the RAN model where packets may be delayed or dropped, and how
to trace such events.

This document assumes that you have already installed |ns3| and the ``nr`` module, are familiar
with how |ns3| works.  If this is not the case, please review the |ns3| Installation Guide and
Tutorial as needed.  The `Getting Started page`_ of the ``nr`` module should also be reviewed.

The companion to this tutorial is the detailed manual for the ``nr`` module, which goes into
more detail about the design and testing of each of the components of the 5G NR module.

To check if you are ready to work through this tutorial, check first if you can run the following
program:

.. sourcecode:: bash

    $ ./ns3 run cttc-nr-demo

and that it outputs the following output

.. sourcecode:: text

    Flow 1 (1.0.0.2:49153 -> 7.0.0.2:1234) proto UDP
        Tx Packets: 6000
        Tx Bytes:   768000
        TxOffered:  10.240000 Mbps
        Rx Bytes:   767744
        Throughput: 10.236587 Mbps
        Mean delay:  0.271518 ms
        Mean jitter:  0.030006 ms
        Rx Packets: 5998
    Flow 2 (1.0.0.2:49154 -> 7.0.0.3:1235) proto UDP
        Tx Packets: 6000
        Tx Bytes:   7680000
        TxOffered:  102.400000 Mbps
        Rx Bytes:   7671040
        Throughput: 102.280533 Mbps
        Mean delay:  0.835065 ms
        Mean jitter:  0.119991 ms
        Rx Packets: 5993

    Mean flow throughput: 56.258560
    Mean flow delay: 0.553292

The tutorial also makes extensive use of the |ns3| logging framework.  To check if logs are enabled
in your |ns3| libraries, try the following command and check if it outputs some additional verbose
output:

.. sourcecode:: bash

    $ NS_LOG="CttcNrDemo" ./ns3 run cttc-nr-demo

Program Overview
================

From what it can be deduced, the demo simulates two *Flows*, each of them relying on a unicast and uni-directional communication. Such flows relies on the User Datagram Protocol (UDP) to carry application data from an origin with IPv4 ``1.0.0.2`` to two recipients with IPv4 ``7.0.0.2`` and ``7.0.0.3`` for *Flow 1* and *Flow 2*, respectively.

The purpose of this example is to simulate a downlink scenario. Two data flows originate from a remote host, with specific characteristics. One flow emphasizes low-latency communications, while the other focuses on high data rate. *These scenarios are well-documented in the context of the "5G Triangle," which recognizes the need to support both Ultra Reliable and Low-Latency Communications (URLLC) and Enhanced Mobile Broadband (eMBB)* **[This not happens in the BWP configuration]**. In the provided demo output, it is evident that the low-latency flow exhibits significantly lower mean delay and jitter compared to the high data rate flow, whereas the opposite is true for the data rate. In the code, the low-latency communication is referred to as ``LowLat`` to indicate its low-latency nature, while the high data rate communication is referred to as ``Voice`` to reflect the traditional traffic associated with high-quality voice communications.

For this communication, the source is an IPv4 address, specifically ``1.0.0.2``, which is referred to as the "remoteHost." The recipients of the data are two User Equipments (UEs).

To support such communications, a 5G Radio Access Network is configured, together with a Long Term Evolution (LTE) Core Network, referred to as the Evolved Packet Core (EPC). The entire architecture is defined as 5G Non-Standalone (NSA).

This demo is characterized by quasi-ideal conditions. For instance, the S1-U link, which interconnects the Serving Gateway (SGW) with the gNB, has no delay. Furthermore, Direct Path Beamforming is used, which is a kind of ideal beamforming algorithm **[Paper ref?]**. Shadowing is not considered, as buildings and any other kind of obstacles that could impair normal Line of Sight (LoS) conditions are absent. Finally, the channel model is updated only once, at the start of the simulation, given that the scenario is static, i.e., it does not change over time.

While both UEs are characterized by a Uniform Planar Array of 2x4 isotropic antennas, the gNB has the same array with a configuration of 4x8.

In terms of spectrum, 2 bands are created to support such communications. The first one operates at 28 GHz, while the second one at 28.2 GHz, both with a band of 100 MHz. In terms of numerology, i.e., the sub-carrier spacing type, the former is 4, while the latter is 2 **[Ref Supported Numerologies Table]**.
This simplifies spectrum allocation, given that each communication will operate on a dedicated Bandwidth Part (BWP), on a single Carrier Component (CC) that occupies the entire band, resulting in the spectrum organized as below::
    Here we should put a figure that explains spectrum allocation, like the following

.. sourcecode:: console

    * The configured spectrum division is:
    * ------------Band1--------------|--------------Band2-----------------
    * ------------CC1----------------|--------------CC2-------------------
    * ------------BWP1---------------|--------------BWP2------------------

Given that there is only one gNB, a total transmission power of 4 dbW is spread among the two BWPs.

In terms of the BWP type and bearer, the former communication is configured to use Non-Guaranteed Bit Rate (NGBR) with Low Latency, also known in the code as ``NGBR_LOW_LAT_EMBB``, while the latter has Guaranteed Bit Rate (GBR) and is named as ``GBR_CONV_VOICE``. A list of other BWP types can be found at **[Doxygen link]**.

On the top of the stack, two UDP applications are configued. The low-latency voice traffic is simulated to send 100 bytes, whereas the high data rate one sends 1252 bytes. Both of these applications send data every 0.1 ms. The ``FlowMonitorHelper`` is used to gather data statistics about the traffic.

Finally, the EPC's Packet Gateway (PGW) is then connected to a remote host with an ideal  Point-to-Point channel: 100 Gbps of data rate with 2500 bytes of Maximum Transmission Unit (MTU) and no delay.

References
==========

[cttc-nr-demo]  cttc-nr-demo program.  Available at: https://gitlab.com/cttc-lena/nr/-/blob/master/examples/cttc-nr-demo.cc

.. _cttc-nr-demo: https://gitlab.com/cttc-lena/nr/-/blob/master/examples/cttc-nr-demo.cc
.. _Getting Started page: https://cttc-lena.gitlab.io/nr/html/getting-started.html

End-to-end observations
***********************

We are mainly interested in observing the packet lifecycle as it moves through the radio access
network (RAN) stack.  We can make a few initial observations about the packet flow.  The code
is using ``UdpClient`` and ``UdpServer`` objects at the application layer.  One client sents
a stream of 1252 byte packets to the server, and the other client sends a stream of 100 byte
packets to the server.  This configuration can be seen in these lines of code:

.. sourcecode:: cpp

    UdpClientHelper dlClientLowLat;
    ...
    dlClientLowLat.SetAttribute("PacketSize", UintegerValue(udpPacketSizeULL));
    ...
    UdpClientHelper dlClientVoice;
    ...
    dlClientVoice.SetAttribute("PacketSize", UintegerValue(udpPacketSizeBe));

Using the following logging-enabled command, generate the log information from the UdpClient and
UdpServer objects in the simulation, and redirect the output to two files, as follows:

.. sourcecode:: bash

    $ NS_LOG="UdpClient=info|prefix_time|prefix_node|prefix_func" ./ns3 run 'cttc-nr-demo' > log.client.out 2>&1
    $ NS_LOG="UdpServer=info|prefix_time|prefix_node|prefix_func" ./ns3 run 'cttc-nr-demo' > log.server.out 2>&1

Looking at the first couple of lines of the ``log.client.out`` file, one can see:

.. sourcecode:: text

    +0.400000000s 6 UdpClient:Send(): TraceDelay TX 100 bytes to 7.0.0.2 Uid: 8 Time: +0.4s
    +0.400000000s 6 UdpClient:Send(): TraceDelay TX 1252 bytes to 7.0.0.3 Uid: 9 Time: +0.4s

These first two packets were sent at the same time to two different UEs, from node 6.  Next, 
observe the first packet arrivals on the UEs via the ``log.server.out`` file:

.. sourcecode:: text

    +0.400408031s 1 UdpServer:HandleRead(): TraceDelay: RX 100 bytes from 1.0.0.2 Sequence Number: 0 Uid: 8 TXtime: +4e+08ns RXtime: +4.00408e+08ns Delay: +408031ns
    ...
    +0.401832140s 2 UdpServer:HandleRead(): TraceDelay: RX 1252 bytes from 1.0.0.2 Sequence Number: 0 Uid: 9 TXtime: +4e+08ns RXtime: +4.01832e+08ns Delay: +1.83214e+06ns

The reception times (and packet delays) are quite different.  One takes only 408 us to be delivered,
the other takes 1832 us to be delivered.  In this tutorial, we will explain why this is so.

This also illustrates that one does not have to let the simulation run for longer than 0.402 seconds
if the focus is on these two packets.  One can use a command-line argument to shrink
the total simulation time, such as:

.. sourcecode:: bash

    ./ns3 run 'cttc-nr-demo --simTime=0.402s'

or one can edit the C++ program directly to change the default simTime value:

.. sourcecode:: cpp

    Time simTime = MilliSeconds(402);

While working through this tutorial, we recommend the latter (temporarily editing the C++ program),
to shorten the log files.  The below example statements omit the ``--simTime`` argument because
it is changed in the C++ program.

RAN lifecycle
*************

The figure XXX depicts the objects that each packet will traverse through the RAN.  This tutorial
will walk through each step of the way, starting with the entry point for these packets-- the
``NrGnbNetDevice``.

EpcEnbApplicaiton
-----------------
The ``EpcEnbApplication`` is responsible for receiving packets tunneled through the EPC model
and sending them into the ``NrGnbNetDevice``.  Conceptually, this is just an application-level
relay function.  Using the following command:

.. sourcecode:: bash

  $ NS_LOG="EpcEnbApplication" ./ns3 run 'cttc-nr-demo' > log.out 2>&1

one can observe this relay function on the first packet, as follows:

.. sourcecode:: text

  +0.400000282s 0 EpcEnbApplication:RecvFromS1uSocket(0x564d4016b3d0, 0x564d400fe520)
  +0.400000282s 0 EpcEnbApplication:SendToLteSocket(0x564d4016b3d0, 0x564d403f7fb0, 2, 2, 128)

The file ``src/lte/model/epc-enb-application.cc`` contains the source code.  In that code, one
can observe the following in ``EpcEnbApplication::SendToLteSocket()``:

.. sourcecode:: text

    if (ipType == 0x04)
    {
        sentBytes = m_lteSocket->Send(packet);
    }
    else if (ipType == 0x06)
    {
        sentBytes = m_lteSocket6->Send(packet);
    }

The LTE sockets are actually created in the EPC helper; specifically, in the file
``src/lte/helper/no-backhaul-epc-helper.cc``.  The socket is created and bound to a NetDevice; in
this case, even though the variable name is ``lteEnbNetDevice``, it will be of type 
``NrGnbNetDevice``:

.. sourcecode:: cpp

      // create LTE socket for the ENB
    Ptr<Socket> enbLteSocket =
        Socket::CreateSocket(enb, TypeId::LookupByName("ns3::PacketSocketFactory"));
    PacketSocketAddress enbLteSocketBindAddress;
    enbLteSocketBindAddress.SetSingleDevice(lteEnbNetDevice->GetIfIndex());
    ...

Packet latency
##############
Packets cannot incur latency in the ``EnbEpcApplication``.

Packet drops
############
Packets cannot be dropped in the ``EnbEpcApplication``.

NrGnbNetDevice
--------------

After the ``EpcEnbApplication`` sends a packet, it is immediately received on the 
``NrGnbNetDevice::DoSend()`` method.  We can observe this in the logs:

.. sourcecode:: bash

  $ NS_LOG="NrGnbNetDevice" ./ns3 run 'cttc-nr-demo' > log.out 2>&1

.. sourcecode:: text

    +0.400000282s 0 NrGnbNetDevice:DoSend(0x5648f6414b40, 0x5648f6747a20, 02-06-ff:ff:ff:ff:ff:ff, 2048)
    +0.400002262s 0 NrGnbNetDevice:DoSend(0x5648f6414b40, 0x5648f6747d40, 02-06-ff:ff:ff:ff:ff:ff, 2048)

The source code for this method is in the file ``contrib/nr/model/nr-gnb-net-device.cc``.  As an
aside, notice how we are bouncing back and forth between the source code directories
``src/lte/`` and ``contrib/nr/``; this is true of the upper layers of the current ``nr`` module,
that it reuses pieces originally implemented for LTE.

The source code reveals that the packets are immediately sent down to the RRC, if some sanity
checks are passed:

.. sourcecode:: bash

    NS_ABORT_MSG_IF(protocolNumber != Ipv4L3Protocol::PROT_NUMBER &&
                        protocolNumber != Ipv6L3Protocol::PROT_NUMBER,
                    "unsupported protocol " << protocolNumber
                                            << ", only IPv4 and IPv6 are supported");
    return m_rrc->SendData(packet);

Packet latency
##############
Packets cannot incur latency in the ``NrGnbNetDevice``.

Packet drops
############
Packets cannot be dropped in the ``NrGnbNetDevice``.

LteEnbRrc
---------
Describe LteEnbRrc::SendPacket ...
