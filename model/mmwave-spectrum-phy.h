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
 */


#ifndef SRC_MMWAVE_MODEL_MMWAVE_SPECTRUM_PHY_H_
#define SRC_MMWAVE_MODEL_MMWAVE_SPECTRUM_PHY_H_


#include <ns3/spectrum-phy.h>
#include "mmwave-spectrum-signal-parameters.h"
#include "mmwave-control-messages.h"
#include "nr-error-model.h"
#include <functional>

namespace ns3 {

class ThreeGppAntennaArrayModel;
class NetDevice;
class SpectrumValue;
class SpectrumChannel;
class MmWavePhy;
class mmWaveChunkProcessor;
class mmWaveInterference;
class MmWaveHarqPhy;

class MmWaveSpectrumPhy : public SpectrumPhy
{
public:
  static TypeId GetTypeId (void);

  MmWaveSpectrumPhy ();
  virtual ~MmWaveSpectrumPhy () override;

  enum State
  {
    IDLE = 0,
    TX,
    RX_DATA,
    RX_DL_CTRL,
    RX_UL_CTRL,
    CCA_BUSY
  };

  typedef Callback< void, const Ptr<Packet> &> MmWavePhyRxDataEndOkCallback;
  typedef std::function<void (const std::list<Ptr<MmWaveControlMessage> > &, uint8_t)> MmWavePhyRxCtrlEndOkCallback;

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
   * \brief Typedef for a channel occupancy. Used by different traces.
   */
  typedef TracedCallback <Time> ChannelOccupiedTracedCallback;


  void InstallPhy (const Ptr<const MmWavePhy> &phyModel);

  // Attributes
  /**
   * \brief Set clear channel assessment (CCA) threshold
   * \param thresholdDBm - CCA threshold in dBms
   */
  void SetCcaMode1Threshold (double thresholdDBm);

  /**
   * Returns clear channel assesment (CCA) threshold
   * \return CCA threshold in dBms
   */
  double GetCcaMode1Threshold (void) const;

  // Inherited
  void SetDevice (Ptr<NetDevice> d) override;
  Ptr<NetDevice> GetDevice () const override;
  void SetMobility (Ptr<MobilityModel> m) override;
  Ptr<MobilityModel> GetMobility () override;
  void SetChannel (Ptr<SpectrumChannel> c) override;
  Ptr<const SpectrumModel> GetRxSpectrumModel () const override;

  /**
   * Implements GetRxAntenna function from SpectrumPhy. This
   * function should not be called for NR devices, since NR devices do not use
   * AntennaModel. This is because 3gpp channel model implementation only
   * supports ThreeGppAntennaArrayModel antenna type.
   *
   * @return
   */
  virtual Ptr<AntennaModel> GetRxAntenna () override;

  /**
   * \brief Returns ThreeGppAntennaArrayModel instance of the device using this
   * SpectrumPhy instance.
   */
  Ptr<const ThreeGppAntennaArrayModel> GetAntennaArray();

  void SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd);
  void SetTxPowerSpectralDensity (Ptr<SpectrumValue> TxPsd);
  void StartRx (Ptr<SpectrumSignalParameters> params) override;
  void StartRxData (Ptr<MmwaveSpectrumSignalParametersDataFrame> params);
  void StartRxDlCtrl (Ptr<MmWaveSpectrumSignalParametersDlCtrlFrame> params);
  void StartRxUlCtrl (Ptr<MmWaveSpectrumSignalParametersUlCtrlFrame> params);
  Ptr<SpectrumChannel> GetSpectrumChannel ();

  bool StartTxDataFrames (Ptr<PacketBurst> pb, std::list<Ptr<MmWaveControlMessage> > ctrlMsgList, Time duration, uint8_t slotInd);

  bool StartTxDlControlFrames (const std::list<Ptr<MmWaveControlMessage> > &ctrlMsgList, const Time &duration);   // control frames from enb to ue

  bool StartTxUlControlFrames (const std::list<Ptr<MmWaveControlMessage> > &ctrlMsgList, const Time &duration);

  void SetPhyRxDataEndOkCallback (MmWavePhyRxDataEndOkCallback c);
  void SetPhyRxCtrlEndOkCallback (const MmWavePhyRxCtrlEndOkCallback &c);
  void SetPhyDlHarqFeedbackCallback (MmWavePhyDlHarqFeedbackCallback c);
  void SetPhyUlHarqFeedbackCallback (MmWavePhyUlHarqFeedbackCallback c);

  void AddDataPowerChunkProcessor (Ptr<mmWaveChunkProcessor> p);
  void AddDataSinrChunkProcessor (Ptr<mmWaveChunkProcessor> p);

  void UpdateSinrPerceived (const SpectrumValue& sinr);

  void InstallHarqPhyModule (Ptr<MmWaveHarqPhy> harq);

  Ptr<MmWaveHarqPhy> GetHarqPhyModule () const;

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
   * \return the cell id
   */
  uint16_t GetCellId () const;
  /**
   * \return the bwp id
   */
  uint16_t GetBwpId () const;
  /**
   * \return true if this class is inside an enb/gnb
   */
  bool IsEnb () const;

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
   * reception (RX_DL_CTRL or RX_UL_CTRL or RX_DATA) function MaybeCcaBusy should be called instead to check
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

  State m_state;

  MmWavePhyRxCtrlEndOkCallback    m_phyRxCtrlEndOkCallback;
  MmWavePhyRxDataEndOkCallback    m_phyRxDataEndOkCallback;

  ChannelOccupiedTracedCallback m_channelOccupied;
  ChannelOccupiedTracedCallback m_txDataTrace;
  ChannelOccupiedTracedCallback m_txCtrlTrace;

  MmWavePhyDlHarqFeedbackCallback m_phyDlHarqFeedbackCallback;
  MmWavePhyUlHarqFeedbackCallback m_phyUlHarqFeedbackCallback;

  TracedCallback<RxPacketTraceParams> m_rxPacketTraceEnb;
  TracedCallback<RxPacketTraceParams> m_rxPacketTraceUe;

  TracedCallback<EnbPhyPacketCountParameter > m_txPacketTraceEnb;

  SpectrumValue m_sinrPerceived;

  Ptr<UniformRandomVariable> m_random;

  bool m_dataErrorModelEnabled;   // when true (default) the phy error model is enabled

  Ptr<MmWaveHarqPhy> m_harqPhyModule;

  double m_ccaMode1ThresholdW;  //!< Clear channel assessment (CCA) threshold in Watts

  bool m_unlicensedMode {false};

  EventId m_checkIfIsIdleEvent; //!< Event used to check if state should be switched from CCA_BUSY to IDLE.

  Time m_busyTimeEnds {Seconds (0)}; //!< Used to schedule switch from CCA_BUSY to IDLE, this is absolute time

  bool m_enableAllInterferences {false}; //!< If true, enables gNB-gNB and UE-UE interferences, if false, they are not taken into account

  Ptr<const MmWavePhy> m_phy;
};

}


#endif /* SRC_MMWAVE_MODEL_MMWAVE_SPECTRUM_PHY_H_ */
