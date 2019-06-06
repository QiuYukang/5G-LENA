/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Biljana Bojovic <biljana.bojovic@cttc.es>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <ns3/test.h>
#include <ns3/object-factory.h>
#include <ns3/mmwave-3gpp-channel.h>
#include <ns3/mmwave-3gpp-propagation-loss-model.h>
#include <ns3/double.h>
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include <ns3/spectrum-converter.h>
#include <ns3/antenna-array-model.h>
#include "ns3/config-store.h"
#include "ns3/core-module.h"

/**
 * \file nr-test-3gpp-channel.cc
 * \ingroup test
 * \brief Unit-testing the MmWave3gppChannel in conjunction with 3gpp pathloss models.
 */
namespace ns3 {


/**
 * \brief The main test class NrTest3gppChannelTestCase.
 */
class NrTest3gppChannelTestCase : public TestCase
{
  struct ChannelPhyConf {

    double m_centerFrequency {28e9};
    double m_bandwidth {400e6};
    double m_numberOfSubcarriersPerRb {12};
    uint32_t m_numerology {4};
    // will be configured in constructor
    uint32_t m_numberOfRb {0};
    double m_subcarrierSpacing {0};

  public:

    ChannelPhyConf (double centerFrequency, double bandwidth, double numerology)
      {
       m_centerFrequency = centerFrequency;
       m_bandwidth = bandwidth;
       m_numerology = numerology;
       //configure the rest of the numerology specific parameters;
       m_numberOfSubcarriersPerRb = 12;
       m_subcarrierSpacing = 15 * std::pow (2, numerology) * 1000;
       m_numberOfRb = m_bandwidth / (m_subcarrierSpacing * m_numberOfSubcarriersPerRb);
      }

      double
      GetBandwidth () const
      {
        return m_bandwidth;
      }

      double
      GetCenterFrequency () const
      {
        return m_centerFrequency;
      }

      uint32_t
      GetNumberOfRb () const
      {
        return m_numberOfRb;
      }

      double
      GetNumberOfSubcarriersPerRb () const
      {
        return m_numberOfSubcarriersPerRb;
      }

      uint32_t
      GetNumerology () const
      {
        return m_numerology;
      }

      double
      GetSubcarrierSpacing () const
      {
        return m_subcarrierSpacing;
      }

      Ptr<const SpectrumModel> GetSpectrumModel ()
      {
        double f = 0.00;
        NS_ASSERT_MSG (m_centerFrequency != 0, "The carrier frequency cannot be set to 0");

        f = m_centerFrequency - (m_numberOfRb * m_subcarrierSpacing * m_numberOfSubcarriersPerRb / 2.0);

        Bands rbs; // A vector representing all resource blocks
        for (uint32_t numrb = 0; numrb < m_numberOfRb; ++numrb)
          {
            BandInfo rb;
            rb.fl = f;
            f += m_subcarrierSpacing * m_numberOfSubcarriersPerRb / 2;
            rb.fc = f;
            f += m_subcarrierSpacing * m_numberOfSubcarriersPerRb / 2;
            rb.fh = f;
            rbs.push_back (rb);
          }

        return Create<SpectrumModel> (rbs);
    };

  };

  struct TestParams
  {
    Ptr<AntennaArrayModel> ueAnt {nullptr};
    Ptr<AntennaArrayModel> gnbAnt {nullptr};
    Ptr<SimpleNetDevice> ueDevice {nullptr};
    Ptr<SimpleNetDevice> gnbDevice {nullptr};
    Ptr<MobilityModel> ueMm {nullptr};
    Ptr<MobilityModel> gnbMm {nullptr};
  };

  std::string m_channelCondition; //!< The channel condition to be configured in the test scenario
  double m_centerFrequency; //! The center frequency to be configured to the channel
  uint32_t m_rxNumerology; //! The numerology to be used by the receiver
  uint32_t m_txNumerology; //! The numerology to be used by the transmiter
  double m_bandwidth; //! The bandwidth to be configured to the channel used in the test

public:

  /**
   * \brief Create NrTest3gppChannelTestCase with the specified test case parameters
   * @param name The specific name for the test
   * @param channelCondition The channel condition to be used in the test case
   * @param numerology The numerology to be used in the specific test cases
   */
  NrTest3gppChannelTestCase (const std::string &name, std::string channelCondition, uint32_t numerology)
    : TestCase (name)
   {
     m_channelCondition = channelCondition;
     m_centerFrequency = 28e9;
     m_bandwidth = 400e6;
     m_rxNumerology = 4;
     m_txNumerology = numerology;
   }

  /**
   * \brief Destroy the test case object instance
   */
  virtual ~NrTest3gppChannelTestCase () override {}

protected:

  /**
   * \brief Test that the initialisation of the beamforming vectors and the links between UEs and BSs
   * is performed correctly.
   * @param channel The 3gpp channel instance to be used
   * @param ueDev UE device
   * @param ueAnt UE antenna array object
   * @param eNbDev BS device
   * @param gnbAnt BS antenna array object
   */
  void TestCreateInitialBeamformingVectors (Ptr<MmWave3gppChannel>& channel,
                                            Ptr<SimpleNetDevice> ueDev,
                                            Ptr<AntennaArrayBasicModel> ueAnt,
                                            Ptr<SimpleNetDevice> eNbDev,
                                            Ptr<AntennaArrayBasicModel> gnbAnt);

  /**
   * \brief Test whether the get channel condition returns the correct value.
   * @param channel The 3gpp channel instance to be used
   * @param ueMm UE mobility model
   * @param gnbMm BS mobility model
   * @param channelConditionTestValue The channel condition test value
   */
  void TestDoGetChannelCondition (Ptr<MmWave3gppChannel>& channel,
                                  Ptr<MobilityModel> ueMm,
                                  Ptr<MobilityModel> gnbMm,
                                  std::string channelConditionTestValue);

  /**
   * \brief Test whether the beamsearch beamforming method is executing properly.
   * @param channel The 3gpp channel instance to be used
   * @param testParams Parameters necessary to perform the testing of the beamforming method
   */
  void TestBeamSearchBeamforming(Ptr<MmWave3gppChannel>& channel, TestParams& testParams );

  /**
   * \brief Test whether long term covariation matrix beamforming method is
   * executing properly.
   * @param channel The 3gpp channel instance to be used
   * @param testParams Parameters necessary to perform the testing of the beamforming method
   */
  void TestLongTermCovMatrixBeamforming(Ptr<MmWave3gppChannel>& channel, TestParams& testParams);

  /**
   * \brief Test whether DoCalcRxPowerSpectralDensity
   * @param channel The 3gpp channel instance to be used
   * @param ueMm UE mobility model
   * @param gnbMm BS mobility model
   * @param ueDev UE device
   * @param gnbDev BS device
   * @param rxNumerology The receivers numerology
   * @param txNumerology The transmitters numerology
   */
  void TestDoCalcRxPowerSpectralDensity(Ptr<MmWave3gppChannel>& channel,
                                        Ptr<MobilityModel> ueMm,
                                        Ptr<MobilityModel> gnbMm,
                                        Ptr<SimpleNetDevice> ueDev,
                                        Ptr<SimpleNetDevice> gnbDev,
                                        uint32_t rxNumerology,
                                        uint32_t txNumerology);

private:
  /**
   * Test run function.
   */
  virtual void DoRun (void) override;

};


void
NrTest3gppChannelTestCase::TestCreateInitialBeamformingVectors (Ptr<MmWave3gppChannel>& channel,
                                                                Ptr<SimpleNetDevice> ueDev,
                                                                Ptr<AntennaArrayBasicModel> ueAnt,
                                                                Ptr<SimpleNetDevice> gnbDev,
                                                                Ptr<AntennaArrayBasicModel> gnbAnt)
{
  channel->RegisterDevicesAntennaArray (ueDev, ueAnt, true);
  channel->RegisterDevicesAntennaArray (gnbDev, gnbAnt);

  NS_TEST_ASSERT_MSG_EQ (channel->IsUeDevice (ueDev), true, "Device is not a UE device");
  NS_TEST_ASSERT_MSG_EQ (channel->IsUeDevice (gnbDev), false, "Device is not a UE device");
}

void
NrTest3gppChannelTestCase::TestDoGetChannelCondition (Ptr<MmWave3gppChannel>& channel,
                                                      Ptr<MobilityModel> ueMm,
                                                      Ptr<MobilityModel> gnbMm,
                                                      std::string channelConditionTestValue)
{

  char channelCondition = channel->DoGetChannelCondition (ueMm,gnbMm);

  NS_TEST_ASSERT_MSG_EQ (channelCondition, *(channelConditionTestValue.c_str()), "Unexpected channel condition!");
}

bool CompareBeamformingVectors (complexVector_t a, complexVector_t b)
{
  if (a.size()!=b.size())
    {
      return false;
    }
  else
    {
      for (uint i = 0; i < a.size(); i++)
        {
          if (a[i]!=b[i])
            {
              return false;
            }
        }
      return true;
    }


}

void
NrTest3gppChannelTestCase::TestBeamSearchBeamforming(Ptr<MmWave3gppChannel>& channel,
                                                     TestParams& testParams)
{


  complexVector_t ueAntVectorBefore =  testParams.ueAnt->GetBeamformingVector(testParams.gnbDevice).first;
  complexVector_t gnbAntVectorBefore =  testParams.gnbAnt->GetBeamformingVector(testParams.ueDevice).first;

  channel->BeamSearchBeamforming(testParams.ueMm, testParams.gnbMm);

  complexVector_t ueAntVectorAfter =  testParams.ueAnt->GetBeamformingVector(testParams.gnbDevice).first;
  complexVector_t gnbAntVectorAfter =  testParams.gnbAnt->GetBeamformingVector(testParams.ueDevice).first;

  NS_TEST_ASSERT_MSG_EQ (CompareBeamformingVectors (ueAntVectorBefore,ueAntVectorAfter), false, "UE antenna beamforming vectors not updated!");
  NS_TEST_ASSERT_MSG_EQ (CompareBeamformingVectors (gnbAntVectorBefore, gnbAntVectorAfter), false, "gnb antenna beamforming vectors not updated!");

}


void
NrTest3gppChannelTestCase::TestDoCalcRxPowerSpectralDensity(Ptr<MmWave3gppChannel>& channel,
                                                            Ptr<MobilityModel> ueMm,
                                                            Ptr<MobilityModel> gnbMm,
                                                            Ptr<SimpleNetDevice> ueDev,
                                                            Ptr<SimpleNetDevice> gnbDev,
                                                            uint32_t rxNumerology,
                                                            uint32_t txNumerology)
{

  // we are creating RX spectrum model by using the default values
  ChannelPhyConf rxPhyConf = ChannelPhyConf (m_centerFrequency, m_bandwidth, rxNumerology);
  Ptr<const SpectrumModel> rxSpectrumModel =  rxPhyConf.GetSpectrumModel ();
  // we create a new configuration in order to create TX spectrum model and TX PSD, by using the provided numerology
  ChannelPhyConf txPhyConf = ChannelPhyConf (m_centerFrequency, m_bandwidth, txNumerology);
  Ptr<const SpectrumModel> txSpectrumModel =  txPhyConf.GetSpectrumModel ();


  double txPowerdBm1 = 23;
  double txPowerdBm2 = 10;

  Ptr<const SpectrumValue> txPsdValue1 = channel->GetFakeTxPowerSpectralDensity(txPowerdBm1,txSpectrumModel);
  Ptr<const SpectrumValue> txPsdValue2 = channel->GetFakeTxPowerSpectralDensity(txPowerdBm2,txSpectrumModel);

  double basePsdWattsHz1 = pow (10.0, (txPowerdBm1 - 30) / 10.0);
  double basePsdWattsHz2 = pow (10.0, (txPowerdBm2 - 30) / 10.0);

  Ptr<const SpectrumValue> convertedTxPsd1, convertedTxPsd2;

  // if the numerology of transmitter is different from the numerology of the receiver we need to perform a conversion between models
  if (rxNumerology != txNumerology)
   {
     SpectrumConverter converter (txSpectrumModel, rxSpectrumModel);
     convertedTxPsd1 = converter.Convert(txPsdValue1);
     convertedTxPsd2 = converter.Convert(txPsdValue2);

     double txPowerConverted1 = Sum(*convertedTxPsd1) * rxPhyConf.GetNumberOfSubcarriersPerRb() * rxPhyConf.GetSubcarrierSpacing();
     double txPowerConverted2 = Sum(*convertedTxPsd2) * rxPhyConf.GetNumberOfSubcarriersPerRb() * rxPhyConf.GetSubcarrierSpacing();

     double txPowerOriginal1 = Sum(*txPsdValue1) * txPhyConf.GetNumberOfSubcarriersPerRb() * txPhyConf.GetSubcarrierSpacing();
     double txPowerOriginal2 = Sum(*txPsdValue2) * txPhyConf.GetNumberOfSubcarriersPerRb() * txPhyConf.GetSubcarrierSpacing();


     NS_TEST_ASSERT_MSG_EQ (txPhyConf.GetNumberOfRb(), txPsdValue1->GetSpectrumModel()->GetNumBands(),
                            "Number of bands in spectrum model should be the same as number of RBs configured for that model.");

     NS_TEST_ASSERT_MSG_EQ (txPsdValue1->GetValuesN(), txPsdValue1->GetSpectrumModel()->GetNumBands(),
                            "The number of values in PSD should be equal to the number of bands of the corresponding spectrum model.");

     NS_TEST_ASSERT_MSG_EQ (convertedTxPsd1->GetValuesN(), rxSpectrumModel->GetNumBands(),
                            "Converted PSD should have the same number of elements as receiver's spectrum model number of bands." );

     NS_TEST_ASSERT_MSG_EQ_TOL(txPowerConverted1,
                               txPowerOriginal1,
                               txPowerOriginal1 * 0.1,
                               "Power of converted tx psd vector should be equal to the power of original psd vector.");

     NS_TEST_ASSERT_MSG_EQ_TOL(txPowerConverted2,
                               txPowerOriginal2,
                               txPowerOriginal2 * 0.1,
                               "Power of converted tx psd vector should be equal to the power of original psd vector.");

     NS_TEST_ASSERT_MSG_EQ_TOL(txPowerConverted1,
                               basePsdWattsHz1,
                               basePsdWattsHz1 * 0.1,
                               "Power of converted tx psd vector should be equal to the power of original psd vector.");

     NS_TEST_ASSERT_MSG_EQ_TOL(txPowerConverted2,
                               basePsdWattsHz2,
                               basePsdWattsHz2 * 0.1,
                               "Power of converted tx psd vector should be equal to the power of original psd vector.");

    }
  else
    {
      convertedTxPsd1 = txPsdValue1;
      convertedTxPsd2 = txPsdValue2;
    }

  // we should provide to DoCalcRxPowerSpectralDensity already converted PSD, this is normally done by multimodel spectrum chanel
  Ptr<const SpectrumValue> rxPsdValue1 = channel->DoCalcRxPowerSpectralDensity (convertedTxPsd1, ueMm, gnbMm);
  Ptr<const SpectrumValue> rxPsdValue2 = channel->DoCalcRxPowerSpectralDensity (convertedTxPsd2, ueMm, gnbMm);

  const SpectrumValue bfGain1Psd = (*rxPsdValue1) / (*convertedTxPsd1);
  const SpectrumValue bfGain2Psd = (*rxPsdValue2) / (*convertedTxPsd2);

  double bfGain1 = Sum (bfGain1Psd)/rxSpectrumModel->GetNumBands();
  double bfGain2 = Sum (bfGain2Psd)/rxSpectrumModel->GetNumBands();

  NS_TEST_ASSERT_MSG_GT (bfGain1, 0, "Beamforming gain should be greater than 0.");
  NS_TEST_ASSERT_MSG_GT (bfGain2, 0, "Beamforming gain should be greater than 0.");

  NS_TEST_ASSERT_MSG_EQ_TOL (bfGain1, bfGain2, bfGain1 * 0.01, "The beamfoming gains should be equal, it does not depend on power.");

}


void
NrTest3gppChannelTestCase::TestLongTermCovMatrixBeamforming (Ptr<MmWave3gppChannel>& channel,
                                                             TestParams& testParams)
{

  Ptr<AntennaArrayBasicModel> ueAnt =  testParams.ueAnt;
  Ptr<AntennaArrayBasicModel> gnbAnt =  testParams.gnbAnt;
  Ptr<SimpleNetDevice> ueDev = testParams.ueDevice;
  Ptr<SimpleNetDevice> gnbDev = testParams.gnbDevice;
  Ptr<MobilityModel> ueMm = testParams.ueMm;
  Ptr<MobilityModel> gnbMm = testParams.gnbMm;

  complexVector_t ueAntVectorBefore =  ueAnt->GetBeamformingVector(gnbDev).first;
  complexVector_t gnbAntVectorBefore =  gnbAnt->GetBeamformingVector(ueDev).first;

  channel->LongTermCovMatrixBeamforming (ueMm, gnbMm);

  complexVector_t ueAntVectorAfter =  ueAnt->GetBeamformingVector(gnbDev).first;
  complexVector_t gnbAntVectorAfter =  gnbAnt->GetBeamformingVector(ueDev).first;

  NS_TEST_ASSERT_MSG_EQ (CompareBeamformingVectors (ueAntVectorBefore,ueAntVectorAfter), false, "UE antenna beamforming vectors not updated!");
  NS_TEST_ASSERT_MSG_EQ (CompareBeamformingVectors (gnbAntVectorBefore, gnbAntVectorAfter), false, "gnb antenna beamforming vectors not updated!");

}


void
NrTest3gppChannelTestCase::DoRun()
{
  Ptr<MmWave3gppChannel> channel = CreateObject<MmWave3gppChannel> ();
  Ptr<MmWave3gppPropagationLossModel> pathLoss = CreateObject<MmWave3gppPropagationLossModel>();

  channel->SetPathlossModel (pathLoss);
  channel->SetAttribute ("CenterFrequency", DoubleValue (m_centerFrequency));
  channel->SetAttribute ("Bandwidth", DoubleValue (m_bandwidth));

  Ptr<Node> ueNode1 = CreateObject<Node>();
  Ptr<Node> gnbNode1 = CreateObject<Node>();

  Ptr<Node> ueNode2 = CreateObject<Node>();
  Ptr<Node> gnbNode2 = CreateObject<Node>();

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 1.5));
  positionAlloc->Add (Vector (0.0, 10.0, 10 ));

  positionAlloc->Add (Vector (1.0, 0.0, 1.5));
  positionAlloc->Add (Vector (1.0, 10.0, 10 ));

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (ueNode1);
  mobility.Install (gnbNode1);
  mobility.Install (ueNode2);
  mobility.Install (gnbNode2);

  Ptr<SimpleNetDevice> ueDev1 =  CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> gnbDev1 =  CreateObject<SimpleNetDevice> ();

  Ptr<SimpleNetDevice> ueDev2 =  CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> gnbDev2 =  CreateObject<SimpleNetDevice> ();

  ueNode1->AddDevice(ueDev1);
  gnbNode1->AddDevice(gnbDev1);
  ueDev1->SetNode (ueNode1);
  gnbDev1->SetNode (gnbNode1);


  ueNode2->AddDevice(ueDev2);
  gnbNode2->AddDevice(gnbDev2);
  ueDev2->SetNode (ueNode2);
  gnbDev2->SetNode (gnbNode2);

  Ptr<AntennaArrayModel> ueAnt1 = CreateObject<AntennaArrayModel>();
  ueAnt1->Initialize();
  Ptr<AntennaArrayModel> gnbAnt1 = CreateObject<AntennaArrayModel>();
  gnbAnt1->Initialize();

  ChannelPhyConf rxPhyConf = ChannelPhyConf (m_centerFrequency, m_bandwidth, m_rxNumerology);
  Ptr<const SpectrumModel> spectrumModel =  rxPhyConf.GetSpectrumModel ();

  ueAnt1->SetSpectrumModel (spectrumModel);
  gnbAnt1->SetSpectrumModel (spectrumModel);

  Ptr<AntennaArrayModel> ueAnt2 = CreateObject<AntennaArrayModel>();
  ueAnt2->Initialize();
  Ptr<AntennaArrayModel> gnbAnt2 = CreateObject<AntennaArrayModel>();
  gnbAnt2->Initialize();

  TestParams testParams;
  testParams.ueMm = ueNode1->GetObject<MobilityModel>();
  testParams.gnbMm = gnbNode1->GetObject<MobilityModel>();
  testParams.ueDevice = ueDev1;
  testParams.gnbDevice = gnbDev1;
  testParams.gnbAnt = gnbAnt1;
  testParams.ueAnt = ueAnt1;

  TestCreateInitialBeamformingVectors (channel, ueDev1, ueAnt1, gnbDev1, gnbAnt1);

  TestCreateInitialBeamformingVectors (channel, ueDev2, ueAnt2, gnbDev2, gnbAnt2);

  pathLoss->m_channelConditions = m_channelCondition;
  TestDoGetChannelCondition (channel, ueNode1->GetObject<MobilityModel>(), gnbNode1->GetObject<MobilityModel>(), m_channelCondition);

  NS_TEST_ASSERT_MSG_EQ(channel->ChannelMatrixExist(ueNode1->GetObject<MobilityModel>(),
                                                    gnbNode1->GetObject<MobilityModel>()), false, "Channel matrix should not exist yet");

  channel->DoGetChannel (ueNode1->GetObject<MobilityModel>(), gnbNode1->GetObject<MobilityModel>());

  channel->DoGetChannel (ueNode1->GetObject<MobilityModel>(), gnbNode2->GetObject<MobilityModel>());

  channel->DoGetChannel (ueNode2->GetObject<MobilityModel>(), gnbNode1->GetObject<MobilityModel>());

  channel->DoGetChannel (ueNode2->GetObject<MobilityModel>(), gnbNode2->GetObject<MobilityModel>());

  NS_TEST_ASSERT_MSG_EQ(channel->ChannelMatrixExist(ueNode1->GetObject<MobilityModel>(),
                                                     gnbNode1->GetObject<MobilityModel>()), true, "Channel matrix should exist at this point");

  NS_TEST_ASSERT_MSG_EQ(channel->ChannelMatrixExist(ueNode1->GetObject<MobilityModel>(),
                                                     gnbNode2->GetObject<MobilityModel>()), true, "Channel matrix should exist at this point");

  NS_TEST_ASSERT_MSG_EQ(channel->ChannelMatrixExist(ueNode2->GetObject<MobilityModel>(),
                                                     gnbNode1->GetObject<MobilityModel>()), true, "Channel matrix should exist at this point");

  NS_TEST_ASSERT_MSG_EQ(channel->ChannelMatrixExist(ueNode2->GetObject<MobilityModel>(),
                                                     gnbNode2->GetObject<MobilityModel>()), true, "Channel matrix should exist at this point");

  NS_TEST_ASSERT_MSG_EQ(channel->ChannelMatrixExist(gnbNode1->GetObject<MobilityModel>(),
                                                    gnbNode2->GetObject<MobilityModel>()), false, "Channel matrix between eNbs should not exist");

  NS_TEST_ASSERT_MSG_EQ(channel->ChannelMatrixExist(ueNode1->GetObject<MobilityModel>(),
                                                    ueNode2->GetObject<MobilityModel>()), false, "Channel matrix between UEs should not exist");


  TestBeamSearchBeamforming (channel, testParams);

  TestLongTermCovMatrixBeamforming (channel, testParams);

  NS_TEST_ASSERT_MSG_EQ (channel->IsValidLink(ueNode1->GetObject<MobilityModel>(), ueNode2->GetObject<MobilityModel>()), false, "Ue<->Ue 3gpp channel link is currently not supported");

  NS_TEST_ASSERT_MSG_EQ (channel->IsValidLink(gnbNode1->GetObject<MobilityModel>(), gnbNode2->GetObject<MobilityModel>()), false, "gnb<->gnb 3gpp channel link is currently not supported");

  NS_TEST_ASSERT_MSG_EQ (channel->IsValidLink(ueNode1->GetObject<MobilityModel>(), gnbNode1->GetObject<MobilityModel>()), true, "Ue<->gNb 3gpp is a valid link");

  NS_TEST_ASSERT_MSG_EQ (channel->IsValidLink(ueNode2->GetObject<MobilityModel>(), gnbNode2->GetObject<MobilityModel>()), true, "Ue<->gNb 3gpp is a valid link");

  TestDoCalcRxPowerSpectralDensity (channel, ueNode1->GetObject<MobilityModel>(), gnbNode1->GetObject<MobilityModel>(), ueDev1, gnbDev1, m_rxNumerology, m_txNumerology);

  TestDoCalcRxPowerSpectralDensity (channel, ueNode1->GetObject<MobilityModel>(), gnbNode2->GetObject<MobilityModel>(), ueDev1, gnbDev2, m_rxNumerology, m_txNumerology);


}

class NrTest3gppChannelTestSuite : public TestSuite
{
public:
  NrTest3gppChannelTestSuite () : TestSuite ("nr-test-3gpp-channel", UNIT)
    {

    std::list<uint32_t>  numerologies  = {0, 1, 2, 3, 4};
    std::list<std::string>  conditions  = {"l", "n"};

    for (const auto & num : numerologies)
      {
        for (const auto & cond : conditions)
          {
            std::stringstream testName;
            testName<<"nr-test-3gpp-channel ";
            if (cond == "l")
              {
                testName << "LOS , ";
              }
            else
              {
                testName << "NLOS ,";
              }

            testName << "Numerology: "<< num;

            AddTestCase(new NrTest3gppChannelTestCase (testName.str(), cond, num), QUICK);
          }
      }
    }

};

static NrTest3gppChannelTestSuite nrTest3gppChannelTestSuite; //!< nrTest3gppChannelTestSuite

}; // namespace ns3
