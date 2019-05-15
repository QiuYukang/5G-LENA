/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *
 *   Author: Biljana Bojovic <biljana.bojovic@cttc.es>
 *   Inspired by lte-specterum-phy.h
 *
 */



#ifndef SRC_MMWAVE_MODEL_MMWAVE_SPECTRUM_PHY_H_
#define SRC_MMWAVE_MODEL_MMWAVE_SPECTRUM_PHY_H_


#include <ns3/object-factory.h>
#include <ns3/event-id.h>
#include <ns3/spectrum-value.h>
#include <ns3/mobility-model.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include <ns3/net-device.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-interference.h>
#include <ns3/data-rate.h>
#include <ns3/generic-phy.h>
#include <ns3/packet-burst.h>
#include "mmwave-spectrum-signal-parameters.h"
#include "ns3/random-variable-stream.h"
#include "mmwave-interference.h"
#include "mmwave-control-messages.h"
#include "mmwave-harq-phy.h"
#include "nr-error-model.h"

namespace ns3 {

typedef Callback< void, const Ptr<Packet> &> MmWavePhyRxDataEndOkCallback;
typedef Callback< void, const std::list<Ptr<MmWaveControlMessage> > &> MmWavePhyRxCtrlEndOkCallback;

/**
 * This method is used by the LteSpectrumPhy to notify the PHY about
 * the status of a certain DL HARQ process
 */
typedef Callback< void, const DlHarqInfo& > MmWavePhyDlHarqFeedbackCallback;

/**
 * This method is used by the LteSpectrumPhy to notify the PHY about
 * the status of a certain UL HARQ process
 */
typedef Callback< void, const UlHarqInfo &> MmWavePhyUlHarqFeedbackCallback;

/**
 * This traced callback is used to notify when is occupied either
 * by some transmission of our own PHY instance or by someone else which
 * is detected by energy detection method.
 */
typedef TracedCallback < Time> ChannelOccupiedTracedCallback;


class MmWaveSpectrumPhy : public SpectrumPhy
{
public:
  MmWaveSpectrumPhy ();
  virtual ~MmWaveSpectrumPhy ();

  enum State
  {
    IDLE = 0,
    TX,
    RX_DATA,
    RX_CTRL,
    CCA_BUSY
  };

  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  void SetDevice (Ptr<NetDevice> d);

  /**
   * \brief Set clear channel assessment (CCA) threshold
   * @param thresholdDBm - CCA threshold in dBms
   */
  void SetCcaMode1Threshold (double thresholdDBm);

  /**
   * Returns clear channel assesment (CCA) threshold
   * @return CCA threshold in dBms
   */
  double GetCcaMode1Threshold (void) const;

  Ptr<NetDevice> GetDevice () const;
  void SetMobility (Ptr<MobilityModel> m);
  Ptr<MobilityModel> GetMobility ();
  void SetChannel (Ptr<SpectrumChannel> c);
  Ptr<const SpectrumModel> GetRxSpectrumModel () const;

  Ptr<AntennaModel> GetRxAntenna ();
  void SetAntenna (Ptr<AntennaModel> a);

  void SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd);
  void SetTxPowerSpectralDensity (Ptr<SpectrumValue> TxPsd);
  void StartRx (Ptr<SpectrumSignalParameters> params);
  void StartRxData (Ptr<MmwaveSpectrumSignalParametersDataFrame> params);
  void StartRxCtrl (Ptr<SpectrumSignalParameters> params);
  Ptr<SpectrumChannel> GetSpectrumChannel ();
  void SetCellId (uint16_t cellId);
  /**
   *
   * \param componentCarrierId the component carrier id
   */
  void SetComponentCarrierId (uint8_t componentCarrierId);

  bool StartTxDataFrames (Ptr<PacketBurst> pb, std::list<Ptr<MmWaveControlMessage> > ctrlMsgList, Time duration, uint8_t slotInd);

  bool StartTxDlControlFrames (const std::list<Ptr<MmWaveControlMessage> > &ctrlMsgList, const Time &duration);   // control frames from enb to ue
  bool StartTxUlControlFrames (void);   // control frames from ue to enb

  void SetPhyRxDataEndOkCallback (MmWavePhyRxDataEndOkCallback c);
  void SetPhyRxCtrlEndOkCallback (MmWavePhyRxCtrlEndOkCallback c);
  void SetPhyDlHarqFeedbackCallback (MmWavePhyDlHarqFeedbackCallback c);
  void SetPhyUlHarqFeedbackCallback (MmWavePhyUlHarqFeedbackCallback c);

  void AddDataPowerChunkProcessor (Ptr<mmWaveChunkProcessor> p);
  void AddDataSinrChunkProcessor (Ptr<mmWaveChunkProcessor> p);

  void UpdateSinrPerceived (const SpectrumValue& sinr);

  void SetHarqPhyModule (Ptr<MmWaveHarqPhy> harq);

  Ptr<mmWaveInterference> GetMmWaveInterference (void) const;

  /**
   * \brief Instruct the Spectrum Model of a incoming transmission.
   *
   * \param rnti RNTI
   * \param ndi New data indicator (0 for retx)
   * \param size TB Size
   * \param mcs MCS of the transmission
   * \param rbMap Resource Block map (PHY-ready vector of SINR indices)
   * \param harqId ID of the HARQ process in the MAC
   * \param rv Redundancy Version: number of times the HARQ has been retransmitted
   * \param downlink indicate if it is downling
   * \param symStart Sym start
   * \param numSym Num of symbols
   */
  void AddExpectedTb (uint16_t rnti, uint8_t ndi, uint32_t size, uint8_t mcs, const std::vector<int> &rbMap,
                      uint8_t harqId, uint8_t rv, bool downlink, uint8_t symStart, uint8_t numSym);

private:
  /**
   * \brief Information about the expected transport block at a certain point in the slot
   *
   * Information passed by the PHY through a call to AddExpectedTb
   */
  struct ExpectedTb
  {
    ExpectedTb (uint8_t ndi, uint32_t tbSize, uint8_t mcs, const std::vector<int> &rbBitmap,
                uint8_t harqProcessId, uint8_t rv, bool isDownlink, uint8_t symStart,
                uint8_t numSym) :
      m_ndi (ndi),
      m_tbSize (tbSize),
      m_mcs (mcs),
      m_rbBitmap (rbBitmap),
      m_harqProcessId (harqProcessId),
      m_rv (rv),
      m_isDownlink (isDownlink),
      m_symStart (symStart),
      m_numSym (numSym) { }
    ExpectedTb () = delete;
    ExpectedTb (const ExpectedTb &o) = default;

    uint8_t m_ndi               {0}; //!< New data indicator
    uint32_t m_tbSize           {0}; //!< TBSize
    uint8_t m_mcs               {0}; //!< MCS
    std::vector<int> m_rbBitmap;     //!< RB Bitmap
    uint8_t m_harqProcessId     {0}; //!< HARQ process ID (MAC)
    uint8_t m_rv                {0}; //!< RV
    bool m_isDownlink           {0}; //!< is Downlink?
    uint8_t m_symStart          {0}; //!< Sym start
    uint8_t m_numSym            {0}; //!< Num sym
  };

  struct TransportBlockInfo
  {
    TransportBlockInfo (const ExpectedTb &expected) :
      m_expected (expected) { }
    TransportBlockInfo () = delete;

    ExpectedTb m_expected;                //!< Expected data from the PHY. Filled by AddExpectedTb
    bool m_isCorrupted {false};           //!< True if the ErrorModel indicates that the TB is corrupted.
                                          //    Filled at the end of data rx/tx
    bool m_harqFeedbackSent {false};      //!< Indicate if the feedback has been sent for an entire TB
    Ptr<NrErrorModelOutput> m_outputOfEM; //!< Output of the Error Model (depends on the EM type)
    double m_sinrAvg {0.0};               //!< AVG SINR (only for the RB used to transmit the TB)
    double m_sinrMin {0.0};               //!< MIN SINR (only between the RB used to transmit the TB)
  };

  //typedef std::unordered_map<uint16_t, TransportBlockInfo> TBMap; //!< Transport map with RNTI as key

  std::unordered_map<uint16_t, TransportBlockInfo> m_transportBlocks; //!< Transport block map
  TypeId m_errorModelType; //!< Error model type

  void ChangeState (State newState, Time duration);
  void EndTx ();
  void EndRxData ();
  void EndRxCtrl ();

  void MaybeCcaBusy ();

  /**
   * \brief Function used to schedule event to check if state should be switched from CCA_BUSY to IDLE.
   * This function should be used only for this transition of state machine. After finishing
   * reception (RX_CTRL or RX_DATA) function MaybeCcaBusy should be called instead to check
   * if to switch to IDLE or CCA_BUSY, and then new event may be created in the case that the
   * channel is BUSY to switch back from busy to idle.
   */
  void CheckIfStillBusy ();

  Ptr<mmWaveInterference> m_interferenceData;
  Ptr<MobilityModel> m_mobility;
  Ptr<NetDevice> m_device;
  Ptr<SpectrumChannel> m_channel;
  Ptr<const SpectrumModel> m_rxSpectrumModel;
  Ptr<SpectrumValue> m_txPsd;
  //Ptr<PacketBurst> m_txPacketBurst;
  std::list<Ptr<PacketBurst> > m_rxPacketBurstList;
  std::list<Ptr<MmWaveControlMessage> > m_rxControlMessageList;

  Time m_firstRxStart;
  Time m_firstRxDuration;

  Ptr<AntennaModel> m_antenna;

  uint16_t m_cellId;

  uint8_t m_componentCarrierId;   ///< the component carrier ID

  State m_state;

  MmWavePhyRxCtrlEndOkCallback    m_phyRxCtrlEndOkCallback;
  MmWavePhyRxDataEndOkCallback                m_phyRxDataEndOkCallback;

  ChannelOccupiedTracedCallback  m_channelOccupied;

  MmWavePhyDlHarqFeedbackCallback m_phyDlHarqFeedbackCallback;
  MmWavePhyUlHarqFeedbackCallback m_phyUlHarqFeedbackCallback;

  TracedCallback<RxPacketTraceParams> m_rxPacketTraceEnb;
  TracedCallback<RxPacketTraceParams> m_rxPacketTraceUe;

  TracedCallback<EnbPhyPacketCountParameter > m_txPacketTraceEnb;

  SpectrumValue m_sinrPerceived;

  Ptr<UniformRandomVariable> m_random;

  bool m_dataErrorModelEnabled;   // when true (default) the phy error model is enabled
  bool m_ctrlErrorModelEnabled;   // when true (default) the phy error model is enabled for DL ctrl frame

  Ptr<MmWaveHarqPhy> m_harqPhyModule;

  bool m_isEnb;

  double m_ccaMode1ThresholdW;  //!< Clear channel assessment (CCA) threshold in Watts

  bool m_unlicensedMode {false};

  EventId m_checkIfIsIdleEvent; //!< Event used to check if state should be switched from CCA_BUSY to IDLE.

  Time m_busyTimeEnds {Seconds (0)}; //!< Used to schedule switch from CCA_BUSY to IDLE, this is absolute time

};

}


#endif /* SRC_MMWAVE_MODEL_MMWAVE_SPECTRUM_PHY_H_ */
