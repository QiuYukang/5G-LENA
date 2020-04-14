/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef MMWAVE_SPECTRUM_PHY_H
#define MMWAVE_SPECTRUM_PHY_H

#include <functional>
#include <ns3/random-variable-stream.h>
#include <ns3/traced-callback.h>
#include <ns3/spectrum-channel.h>
#include <ns3/net-device.h>
#include "mmwave-phy.h"
#include "mmwave-harq-phy.h"
#include "mmwave-interference.h"
#include "mmwave-spectrum-signal-parameters.h"
#include "mmwave-control-messages.h"
#include <ns3/lte-chunk-processor.h>

namespace ns3 {

/**
 * \ingroup ue-phy
 * \ingroup gnb-phy
 * \ingroup spectrum
 *
 * \brief MmWaveSpectrumPhy models some of the basic
 * physical layer functionalities such as transmitting CTRL or DATA,
 * receiving the signals, decoding them, and distinguishing whether the
 * signal is useful (CTRL, DATA), i.e. sent to this NR device MmWavePhy
 * instance, or it should be considered as interference. It
 * is also responsible for obtaining measurements, and for HARQ feedback
 * generation by leveraging HARQ module.
 *
 * This class implements the interface between the MmWavePhy
 * and the spectrum channel, and provides to MmWavePhy aforementioned
 * functionalities. Each NR device's MmWavePhy has its
 * own MmWaveSpectrumPhy that is in charge of providing these basic
 * phy layer functionalities. In order to be able to receive signals from the channel,
 * each MmWaveSpectrumPhy should be registered to listen events from its channel.
 * To achieve that, during the configuration of NR device's MmWavePhy
 * at some point should be called AddRx function of the spectrum channel
 * to register the MmWaveSpectrumPhy instance as a receiver on that channel.
 *
 * This class also has the interface with mmWaveInterference class to
 * pass the necessary information for the interference calculation and to
 * obtain the interference calculation results.
 *
 * Also it has interface with HARQ module, to which it passes necessary
 * information for the HARQ feedback generation, which is then forwarded
 * to MmWavePhy.
 */
class MmWaveSpectrumPhy : public SpectrumPhy
{
public:
  /**
   * \brief Get the object TypeId
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief MmWaveSpectrumPhy constructor
   */
  MmWaveSpectrumPhy ();

  /**
   * \brief ~MmWaveSpectrumPhy
   */
  virtual ~MmWaveSpectrumPhy () override;

  /**
   * \brief Enum that defines possible states of the spectrum phy
   */
  enum State
  {
    IDLE = 0,  //!< IDLE state (no action in progress)
    TX,        //!< Transmitting state (data or ctrl)
    RX_DATA,   //!< Receiving data
    RX_DL_CTRL,//!< Receiveing DL CTRL
    RX_UL_CTRL,//!< Receiving UL CTRL
    CCA_BUSY   //!< BUSY state (channel occupied by another entity)
  };

  //callbacks typefefs and setters
  /**
   * \brief This callback method type is used to notify that DATA is received
   */
  typedef Callback< void, const Ptr<Packet> &> MmWavePhyRxDataEndOkCallback;
  /**
   * \brief This callback method type is used to notify that CTRL is received
   */
  typedef std::function<void (const std::list<Ptr<MmWaveControlMessage> > &, uint8_t)> MmWavePhyRxCtrlEndOkCallback;
  /**
   * This callback method type is used by the MmWaveSpectrumPhy to notify the PHY about
   * the status of a DL HARQ feedback
   */
  typedef Callback< void, const DlHarqInfo& > MmWavePhyDlHarqFeedbackCallback;
  /**
   * This callback method type is used by the MmWaveSpectrumPhy to notify the PHY about
   * the status of a UL HARQ feedback
   */
  typedef Callback< void, const UlHarqInfo &> MmWavePhyUlHarqFeedbackCallback;

  /**
   * \brief Sets the callback to be called when DATA is received successfully
   * \param c the callback function
   */
  void SetPhyRxDataEndOkCallback (const MmWavePhyRxDataEndOkCallback& c);
  /**
   * \brief Sets the callback to be called when CTRL is received successfully
   * \param c the callback function
   */
  void SetPhyRxCtrlEndOkCallback (const MmWavePhyRxCtrlEndOkCallback& c);
  /**
   * \brief Sets the callback to be called when DL HARQ feedback is generated
   */
  void SetPhyDlHarqFeedbackCallback (const MmWavePhyDlHarqFeedbackCallback& c);
  /**
   * \brief Sets the callback to be called when UL HARQ feedback is generated
   */
  void SetPhyUlHarqFeedbackCallback (const MmWavePhyUlHarqFeedbackCallback& c);

  //Methods inherited from spectrum phy
  void SetDevice (Ptr<NetDevice> d) override;
  Ptr<NetDevice> GetDevice () const override;
  void SetMobility (Ptr<MobilityModel> m) override;
  Ptr<MobilityModel> GetMobility () override;
  void SetChannel (Ptr<SpectrumChannel> c) override;
  Ptr<const SpectrumModel> GetRxSpectrumModel () const override;
  /**
   * \brief Inherited from SpectrumPhy
   * Note: Implements GetRxAntenna function from SpectrumPhy. This
   * function should not be called for NR devices, since NR devices do not use
   * AntennaModel. This is because 3gpp channel model implementation only
   * supports ThreeGppAntennaArrayModel antenna type.
   * \return should not return anything
   */
  virtual Ptr<AntennaModel> GetRxAntenna () override;
  /**
   * \brief Inherited from SpectrumPhy. When this function is called
   * this spectrum phy starts receiving a signal from its spectrum channel.
   * \param params SpectrumSignalParameters object that will be used to process this signal
   */
  void StartRx (Ptr<SpectrumSignalParameters> params) override;

  // Attributes setters
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
  /**
   * \brief Sets whether to perform in unclicensed mode in which the channel monitoring is enabled
   * \param unlicensedMode if true the unlicensed mode is enabled
   */
  void SetUnlicensedMode (bool unlicensedMode);
  /**
   * \brief Enables or disabled data error model
   * \param dataErrorModelEnabled boolean saying whether the data error model should be enabled
   */
  void SetDataErrorModelEnabled (bool dataErrorModelEnabled);
  /**
   * \brief Sets the error model type
   */
  void SetErrorModelType (TypeId errorModelType);

  // other methods
  /**
   * \brief Sets noise power spectral density to be used by this device
   * \param noisePsd SpectrumValue object holding noise PSD
   */
  void SetNoisePowerSpectralDensity (const Ptr<const SpectrumValue>& noisePsd);
  /**
   * \brief Sets transmit power spectral density
   * \param txPsd transmit power spectral density to be used for the upcoming transmissions by this spectrum phy
   */
  void SetTxPowerSpectralDensity (const Ptr<SpectrumValue>& txPsd);
  /**
   * \brief Starts transmission of data frames on connected spectrum channel object
   * \param pb packet burst to be transmitted
   * \param ctrlMsgList conrol message list
   * \param duration the duration of transmission
   * \param slotInd the slot indication
   */
  void StartTxDataFrames (const Ptr<PacketBurst>& pb, const std::list<Ptr<MmWaveControlMessage> >& ctrlMsgList, Time duration, uint8_t slotInd);
  /**
   * \brief Starts transmission of DL CTRL
   * \param duration the duration of this transmission
   */
  void StartTxDlControlFrames (const std::list<Ptr<MmWaveControlMessage> > &ctrlMsgList, const Time &duration);   // control frames from enb to ue
  /**
   * \brief Start transmission of UL CTRL
   * \param ctrlMsgList the list of control messages to be transmitted in UL
   * \param duration the duration of the CTRL messages transmission
   */
  void StartTxUlControlFrames (const std::list<Ptr<MmWaveControlMessage> > &ctrlMsgList, const Time &duration);
  /**
   * \brief Adds the chunk processor that will process the power for the data
   * \param p the chunk processor
   */
  void AddDataPowerChunkProcessor (const Ptr<LteChunkProcessor>& p);
  /**
   * \brief Adds the chunk processor that will process the interference
   * \param p the chunk processor
   */
  void AddDataSinrChunkProcessor (const Ptr<LteChunkProcessor>& p);
  /**
   * \brief SpectrumPhy that will be called when the SINR for the received
   * DATA is being calculated by the interference object over DATA chunk
   * processor
   * \param sinr the resulting SINR spectrum value
   */
  void UpdateSinrPerceived (const SpectrumValue& sinr);
  /**
   * \brief Install HARQ phy module of this spectrum phy
   * \param harq Harq module of this spectrum phy
   */
  void InstallHarqPhyModule (const Ptr<MmWaveHarqPhy>& harq);
  /**
   * \brief Set MmWavePhy of this spectrum phy in order to be able
   * to obtain information such as cellId, bwpId, etc.
   */
  void InstallPhy (const Ptr<const MmWavePhy> &phyModel);
  /**
   * \return Returns ThreeGppAntennaArrayModel instance of this spectrum phy
   */
  Ptr<const ThreeGppAntennaArrayModel> GetAntennaArray (void) const;
  /**
   * \brief Returns spectrum channel object to which is attached this spectrum phy instance
   */
  Ptr<SpectrumChannel> GetSpectrumChannel (void) const;
  /**
   * \return HARQ module of this spectrum phy
   */
  Ptr<MmWaveHarqPhy> GetHarqPhyModule (void) const;
  /**
   * \return mmWaveInterference instance of this spectrum phy
   */
  Ptr<mmWaveInterference> GetMmWaveInterference (void) const;
  /**
   * \brief Instruct the Spectrum Model of a incoming transmission.
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

protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;

private:

  /**
   * \brief Function is called when what is being received is holding data
   * \para params spectrum parameters that are holding information regarding data frame
   */
  void StartRxData (const Ptr<MmwaveSpectrumSignalParametersDataFrame>& params);
  /**
   * \brief Function that is called when is being received DL CTRL
   * \param params holds DL CTRL frame signal parameters structure
   */
  void StartRxDlCtrl (const Ptr<MmWaveSpectrumSignalParametersDlCtrlFrame>& params);
  /**
   * \brief Function that is called when is being received UL CTRL
   * \param params holds UL CTRL frame signal parameters structure
   */
  void StartRxUlCtrl (const Ptr<MmWaveSpectrumSignalParametersUlCtrlFrame>& params);
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
   * \brief Update the state of the spectrum phy. The states are:
   *  IDLE, TX, RX_DATA, RX_DL_CTRL, RX_UL_CTRL, CCA_BUSY.
   * \param newState the new state
   * \param duration how much time the spectrum phy will be in the new state
   */
  void ChangeState (State newState, Time duration);
  /**
   * \brief Function that is called when the transmission has ended. It is
   * used to update spectrum phy state.
   */
  void EndTx ();
  /**
   * \brief Function that is called when the spectrum phy finishes the reception of DATA. This
   * function processed the data being received and generated HARQ feedback.
   * It also updates spectrum phy state.
   */
  void EndRxData ();
  /**
   * \brief Function that is called when the spectrum phy finishes the reception of CTRL.
   * It stores CTRL messages and updates spectrum phy state.
   */
   void EndRxCtrl ();
   /**
    * \brief Check if the channel is busy. If yes, updates the spectrum phy state.
    */
   void MaybeCcaBusy ();
   /**
    * \brief Function used to schedule an event to check if state should be switched from CCA_BUSY to IDLE.
    * This function should be used only for this transition of state machine. After finishing
    * reception (RX_DL_CTRL or RX_UL_CTRL or RX_DATA) function MaybeCcaBusy should be called instead to check
    * if to switch to IDLE or CCA_BUSY, and then new event may be created in the case that the
    * channel is BUSY to switch back from busy to idle.
    */
   void CheckIfStillBusy ();
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

  //attributes
  TypeId m_errorModelType {Object::GetTypeId()}; //!< Error model type by default is NrLteMiErrorModel
  bool m_dataErrorModelEnabled {true}; //!< whether the phy error model for DATA is enabled, by default is enabled
  double m_ccaMode1ThresholdW {0}; //!< Clear channel assessment (CCA) threshold in Watts, attribute that it configures it is
                                   //   CcaMode1Threshold and is configured in dBm
  bool m_unlicensedMode {false}; //!< Whether this spectrum phy is configure to work in an unlicensed mode.
                                 //   Unlicensed mode additionally to licensed mode allows channel monitoring to discover if is busy before transmission.

  Ptr<SpectrumChannel> m_channel {nullptr}; //!< channel is needed to be able to connect listener spectrum phy (AddRx) or to start transmission StartTx
  Ptr<const SpectrumModel> m_rxSpectrumModel {nullptr}; //!< the spectrum model of this spectrum phy
  Ptr<MobilityModel> m_mobility {nullptr}; //!< the mobility model of the node to which belongs this spectrum phy
  Ptr<NetDevice> m_device {nullptr}; //!< the device to which belongs this spectrum phy
  Ptr<const MmWavePhy> m_phy {nullptr}; //!< a pointer to phy instance to which belongs this spectrum phy
  Ptr<MmWaveHarqPhy> m_harqPhyModule {nullptr}; //!< the HARQ module of this spectrum phy instance
  Ptr<mmWaveInterference> m_interferenceData {nullptr}; //!<the interference object used to calculate the interference for this spectrum phy
  Ptr<SpectrumValue> m_txPsd {nullptr}; //!< tx power spectral density
  Ptr<UniformRandomVariable> m_random {nullptr}; //!< the random variable used for TB decoding

  std::unordered_map<uint16_t, TransportBlockInfo> m_transportBlocks; //!< Transport block map per RNTI of TBs which are expected to be received by reading DL or UL DCIs
  std::list<Ptr<PacketBurst> > m_rxPacketBurstList; //!< the list of received packets
  std::list<Ptr<MmWaveControlMessage> > m_rxControlMessageList; //!< the list of received control messages

  Time m_firstRxStart {Seconds (0)}; //!< this is needed to save the time at which we lock down onto signal
  Time m_firstRxDuration {Seconds (0)}; //!< the duration of the current reception
  State m_state {IDLE}; //!<spectrum phy state
  SpectrumValue m_sinrPerceived; //!< SINR that is being update at the end of the DATA reception and is used for TB decoding
  EventId m_checkIfIsIdleEvent; //!< Event used to check if state should be switched from CCA_BUSY to IDLE.
  Time m_busyTimeEnds {Seconds (0)}; //!< Used to schedule switch from CCA_BUSY to IDLE, this is absolute time

  //callbacks for CTRL and DATA, and UL/DL HARQ
  MmWavePhyRxCtrlEndOkCallback m_phyRxCtrlEndOkCallback; //!< callback that is notified when the CTRL is received
  MmWavePhyRxDataEndOkCallback m_phyRxDataEndOkCallback; //!< callback that is notified when the DATA is received
  MmWavePhyDlHarqFeedbackCallback m_phyDlHarqFeedbackCallback; //!< callback that is notified when the DL HARQ feedback is being generated
  MmWavePhyUlHarqFeedbackCallback m_phyUlHarqFeedbackCallback; //!< callback that is notified when the UL HARQ feedback is being generated

  //traces
  TracedCallback <Time> m_channelOccupied; //!< trace callback that is notifying of total time that this spectrum phy sees the channel occupied, by others and by itself
  TracedCallback <Time> m_txDataTrace; //!< trace callback that is notifying when this spectrum phy starts to occupy the channel with data transmission
  TracedCallback <Time> m_txCtrlTrace; //!< trace callback that is notifying when this spectrum phy starts to occupy the channel with transmission of CTRL
  TracedCallback<RxPacketTraceParams> m_rxPacketTraceEnb; //!< trace callback that is notifying when eNb received the packet
  TracedCallback<RxPacketTraceParams> m_rxPacketTraceUe; //!< trace callback that is notifying when UE received the packet
  TracedCallback<EnbPhyPacketCountParameter > m_txPacketTraceEnb; //!< trace callback that is notifying when eNb transmts the packet
};

}


#endif /* MMWAVE_SPECTRUM_PHY_H */
