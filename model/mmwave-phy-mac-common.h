/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
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
 *   Author: Marco Miozzo <marco.miozzo@cttc.es>
 *           Nicola Baldo  <nbaldo@cttc.es>
 *
 *   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
 *                Sourjya Dutta <sdutta@nyu.edu>
 *                Russell Ford <russell.ford@nyu.edu>
 *                Menglei Zhang <menglei@nyu.edu>
 *                Biljana Bojovic <bbojovic@cttc.es>
 */

#ifndef SRC_MMWAVE_MODEL_MMWAVE_PHY_MAC_COMMON_H
#define SRC_MMWAVE_MODEL_MMWAVE_PHY_MAC_COMMON_H

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <deque>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/string.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/component-carrier.h>
#include <ns3/enum.h>
#include <memory>


namespace ns3 {

struct GetFirst
{
  template <class First, class Second>
  First& operator() (std::pair<First, Second>& p)
  {
    return p.first;
  }

  template <class First, class Second>
  First& operator() (typename std::map<First, Second>::iterator& p)
  {
    return p.first;
  }

  template <class First, class Second>
  const First& operator() (const std::pair<First, Second>& p)
  {
    return p.first;
  }

  template <class First, class Second>
  const First& operator() (const typename std::map<First, Second>::iterator& p)
  {
    return p.first;
  }
};

struct GetSecond
{
  template <class First, class Second>
  Second& operator() (std::pair<First, Second>& p)
  {
    return p.second;
  }

  template <class First, class Second>
  Second& operator() (typename std::map<First, Second>::iterator& p)
  {
    return p.second;
  }

  template <class First, class Second>
  Second& operator() (typename std::unordered_map<First, Second>::iterator& p)
  {
    return p.second;
  }

  template <class First, class Second>
  const Second& operator() (const std::pair<First, Second>& p)
  {
    return p.second;
  }

  template <class First, class Second>
  const Second& operator() (const typename std::map<First, Second>::iterator& p)
  {
    return p.second;
  }

  template <class First, class Second>
  const Second& operator() (const typename std::unordered_map<First, Second>::iterator& p)
  {
    return p.second;
  }
};

struct SfnSf
{
  SfnSf () = default;

  SfnSf (uint16_t frameNum, uint8_t sfNum, uint16_t slotNum, uint8_t varTtiNum)
    : m_frameNum (frameNum),
    m_subframeNum (sfNum),
    m_slotNum (slotNum),
    m_varTtiNum (varTtiNum)
  {
  }

  uint64_t
  Encode () const
  {
    uint64_t ret = 0ULL;
    ret = (static_cast<uint64_t> (m_frameNum) << 32 ) |
      (static_cast<uint64_t> (m_subframeNum) << 24) |
      (static_cast<uint64_t> (m_slotNum) << 8) |
      (static_cast<uint64_t> (m_varTtiNum));
    return ret;
  }

  static uint64_t
  Encode (const SfnSf &p)
  {
    uint64_t ret = 0ULL;
    ret = (static_cast<uint64_t> (p.m_frameNum) << 32 ) |
      (static_cast<uint64_t> (p.m_subframeNum) << 24) |
      (static_cast<uint64_t> (p.m_slotNum) << 8) |
      (static_cast<uint64_t> (p.m_varTtiNum));
    return ret;
  }

  void
  Decode (uint64_t sfn)
  {
    m_frameNum    = (sfn & 0x0000FFFF00000000) >> 32;
    m_subframeNum = (sfn & 0x00000000FF000000) >> 24;
    m_slotNum     = (sfn & 0x0000000000FFFF00) >> 8;
    m_varTtiNum   = (sfn & 0x00000000000000FF);
  }

  static SfnSf
  FromEncoding (uint64_t sfn)
  {
    SfnSf ret;
    ret.m_frameNum    = (sfn & 0x0000FFFF00000000) >> 32;
    ret.m_subframeNum = (sfn & 0x00000000FF000000) >> 24;
    ret.m_slotNum     = (sfn & 0x0000000000FFFF00) >> 8;
    ret.m_varTtiNum   = (sfn & 0x00000000000000FF);
    return ret;
  }

  SfnSf
  IncreaseNoOfSlots (uint32_t slotsPerSubframe, uint32_t subframesPerFrame) const
  {
    return IncreaseNoOfSlotsWithLatency (1, slotsPerSubframe, subframesPerFrame);
  }

  SfnSf
  CalculateUplinkSlot (uint32_t ulSchedDelay, uint32_t slotsPerSubframe, uint32_t subframesPerFrame) const
  {
    return IncreaseNoOfSlotsWithLatency (ulSchedDelay, slotsPerSubframe, subframesPerFrame);
  }

  SfnSf
  IncreaseNoOfSlotsWithLatency (uint32_t latency, uint32_t slotsPerSubframe, uint32_t subframesPerFrame) const
  {
    SfnSf retVal = *this;
    // currently the default value of L1L2 latency is set to 2 and is interpreted as in the number of slots
    // will be probably reduced to order of symbols
    retVal.m_frameNum += (this->m_subframeNum + (this->m_slotNum + latency) / slotsPerSubframe) / subframesPerFrame;
    retVal.m_subframeNum = (this->m_subframeNum + (this->m_slotNum + latency) / slotsPerSubframe) % subframesPerFrame;
    retVal.m_slotNum = (this->m_slotNum + latency) % slotsPerSubframe;
    return retVal;
  }

  /**
   * \brief Add to this SfnSf a number of slot indicated by the first parameter
   * \param slotN Number of slot to add
   * \param slotsPerSubframe Number of slot per subframe
   * \param subframesPerFrame Number of subframes per frame
   */
  void
  Add (uint32_t slotN, uint32_t slotsPerSubframe, uint32_t subframesPerFrame)
  {
    m_frameNum += (m_subframeNum + (m_slotNum + slotN) / slotsPerSubframe) / subframesPerFrame;
    m_subframeNum = (m_subframeNum + (m_slotNum + slotN) / slotsPerSubframe) % subframesPerFrame;
    m_slotNum = (m_slotNum + slotN) % slotsPerSubframe;
  }

  /**
   * \brief operator < (less than)
   * \param rhs other SfnSf to compare
   * \return true if this SfnSf is less than rhs
   *
   * The comparison is done on m_frameNum, m_subframeNum, and m_slotNum: not on varTti
   */
  bool operator < (const SfnSf& rhs) const
  {
    if (m_frameNum < rhs.m_frameNum)
      {
        return true;
      }
    else if ((m_frameNum == rhs.m_frameNum ) && (m_subframeNum < rhs.m_subframeNum))
      {
        return true;
      }
    else if (((m_frameNum == rhs.m_frameNum ) && (m_subframeNum == rhs.m_subframeNum)) && (m_slotNum < rhs.m_slotNum))
      {
        return true;
      }
    else
      {
        return false;
      }
  }

  /**
   * \brief operator ==, compares only frame, subframe, and slot
   * \param o other instance to compare
   * \return true if this instance and the other have the same frame, subframe, slot
   *
   * Used in the MAC operation, in which the varTti is not used (or, it is
   * its duty to fill it).
   *
   * To check the varTti, please use this operator and IsTtiEqual()
   */
  bool operator == (const SfnSf &o) const
  {
    return (m_frameNum == o.m_frameNum) && (m_subframeNum == o.m_subframeNum)
           && (m_slotNum == o.m_slotNum);
  }

  /**
   * \brief Compares frame, subframe, slot, and varTti
   * \param o other instance to compare
   * \return true if this instance and the other have the same frame, subframe, slot, and varTti
   *
   * Used in PHY or in situation where VarTti is needed.
   */
  bool IsTtiEqual (const SfnSf &o) const
  {
    return (*this == o) && (m_varTtiNum == o.m_varTtiNum);
  }

  uint16_t m_frameNum   {0}; //!< Frame Number
  uint8_t m_subframeNum {0}; //!< SubFrame Number
  uint16_t m_slotNum    {0}; //!< Slot number (a slot is made by 14 symbols)
  uint8_t m_varTtiNum   {0}; //!< Equivalent to symStart: it is the symbol in which the sfnsf starts
};

struct TbInfoElement
{
  TbInfoElement () :
    m_isUplink (0), m_varTtiIdx (0), m_rbBitmap (0), m_rbShift (0), m_rbStart (
      0), m_rbLen (0), m_symStart (0), m_numSym (0), m_resAlloc (0), m_mcs (
      0), m_tbSize (0), m_ndi (0), m_rv (0), m_harqProcess (0)
  {
  }

  bool m_isUplink;              // is uplink grant?
  uint8_t m_varTtiIdx;          // var tti index
  uint32_t m_rbBitmap;          // Resource Block Group bitmap
  uint8_t m_rbShift;            // shift for res alloc type 1
  uint8_t m_rbStart;            // starting RB index for uplink res alloc type 0
  uint16_t m_rbLen;
  uint8_t m_symStart;           // starting symbol index for flexible TTI scheme
  uint8_t m_numSym;             // number of symbols for flexible TTI scheme
  uint8_t m_resAlloc;           // resource allocation type
  uint8_t m_mcs;
  uint32_t m_tbSize;
  uint8_t m_ndi;
  uint8_t m_rv;
  uint8_t m_harqProcess;
};

struct DlDciInfoElementTdma
{
  DlDciInfoElementTdma () :
    m_symStart (0), m_numSym (0), m_mcs (0), m_tbSize (0), m_ndi (0), m_rv (
      0), m_harqProcess (0)
  {
  }

  uint8_t m_symStart {0};   // starting symbol index for flexible TTI scheme
  uint8_t m_numSym   {0};     // number of symbols for flexible TTI scheme
  uint8_t m_mcs      {2};
  uint32_t m_tbSize  {0};
  uint8_t m_ndi      {0};
  uint8_t m_rv       {0};
  uint8_t m_harqProcess {14};
};

/**
 * \brief Scheduling information. Despite the name, it is not TDMA.
 */
struct DciInfoElementTdma
{
  enum DciFormat
  {
    DL = 0,
    UL = 1
  };

  DciInfoElementTdma () = delete;

  /**
   * \brief Constructor used in MmWaveUePhy to build local DCI for DL and UL control
   * \param symStart Sym start
   * \param numSym Num sym
   * \param rbgBitmask Bitmask of RBG
   */
  DciInfoElementTdma (uint8_t symStart, uint8_t numSym,
                      const std::vector<uint8_t> &rbgBitmask)
    : m_symStart (symStart),
    m_numSym (numSym),
    m_rbgBitmask (rbgBitmask)
  {
  }

  /**
   * \brief Construct to build brand new DCI. Please remember to update manually
   * the HARQ process ID and the RBG bitmask
   *
   * \param rnti
   * \param format
   * \param symStart
   * \param numSym
   * \param mcs
   * \param tbs
   * \param ndi
   * \param rv
   */
  DciInfoElementTdma (uint16_t rnti, DciFormat format, uint8_t symStart,
                      uint8_t numSym, uint8_t mcs, uint32_t tbs, uint8_t ndi,
                      uint8_t rv)
    : m_rnti (rnti), m_format (format), m_symStart (symStart),
    m_numSym (numSym), m_mcs (mcs), m_tbSize (tbs), m_ndi (ndi), m_rv (rv)
  {
  }


  /**
   * \brief Copy constructor except for some values that have to be overwritten
   * \param symStart Sym start
   * \param numSym Num sym
   * \param ndi New Data Indicator: 0 for Retx, 1 for New Data
   * \param rv Retransmission value
   * \param o Other object from which copy all that is not specified as parameter
   */
  DciInfoElementTdma (uint8_t symStart, uint8_t numSym, uint8_t ndi, uint8_t rv,
                      const DciInfoElementTdma &o)
    : m_rnti (o.m_rnti),
      m_format (o.m_format),
      m_symStart (symStart),
      m_numSym (numSym),
      m_mcs (o.m_mcs),
      m_tbSize (o.m_tbSize),
      m_ndi (ndi),
      m_rv (rv),
      m_harqProcess (o.m_harqProcess),
      m_rbgBitmask (o.m_rbgBitmask)
  {
  }

  const uint16_t m_rnti       {0};
  const DciFormat m_format    {DL};
  const uint8_t m_symStart    {0};   // starting symbol index for flexible TTI scheme
  const uint8_t m_numSym      {0};   // number of symbols for flexible TTI scheme
  const uint8_t m_mcs         {0};
  const uint32_t m_tbSize     {0};
  const uint8_t m_ndi         {0};   // By default is retransmission
  const uint8_t m_rv          {0};   // not used for UL DCI
  uint8_t m_harqProcess {0};
  std::vector<uint8_t> m_rbgBitmask  {};   //!< RBG mask: 0 if the RBG is not used, 1 otherwise
};

struct TbAllocInfo
{
  TbAllocInfo () :
    m_rnti (0)
  {

  }
  //struct
  SfnSf m_sfnSf;
  uint16_t m_rnti;
  std::vector<unsigned> m_rbMap;
  TbInfoElement m_tbInfo;
};

struct DciInfoElement
{
  DciInfoElement () :
    m_rnti (0), m_cceIndex (0), m_format (0), m_tddBitmap (0)
  {
  }

  uint16_t m_rnti;
  uint8_t m_cceIndex;
  uint8_t m_format;     // to support different DCI types
  uint16_t m_tddBitmap;         // 0 == DL, 1 == UL
  std::vector<TbInfoElement> m_tbInfoElements;
};

struct RlcPduInfo
{
  RlcPduInfo () = default;
  RlcPduInfo (const RlcPduInfo &o) = default;
  ~RlcPduInfo () = default;

  RlcPduInfo (uint8_t lcid, uint32_t size) :
    m_lcid (lcid), m_size (size)
  {
  }
  uint8_t m_lcid  {0};
  uint32_t m_size {0};
};

struct VarTtiAllocInfo
{
  enum TddMode
  {
    NA = 0, DL = 1, UL = 2,
  };

  enum VarTtiType
  {
    CTRL_DATA = 0, DATA = 1, CTRL = 2
  };

  enum CtrlTxMode
  {
    ANALOG = 0, DIGITAL = 1, OMNI = 2
  };

  VarTtiAllocInfo () = delete;
  VarTtiAllocInfo (const VarTtiAllocInfo &o) = default;

  VarTtiAllocInfo (TddMode tddMode, VarTtiType varTtiType,
                   const std::shared_ptr<DciInfoElementTdma> &dci)
    : m_tddMode (tddMode), m_isOmni (0), m_varTtiType (varTtiType), m_dci (dci)
  {
  }

  TddMode m_tddMode       {NA};
  bool m_isOmni           {false};  // Beamforming disabled, true if omnidirectional
  VarTtiType m_varTtiType {CTRL_DATA};
  std::shared_ptr<DciInfoElementTdma> m_dci;
  std::vector<RlcPduInfo> m_rlcPduInfo;

  bool operator < (const VarTtiAllocInfo& o) const
  {
    return (m_dci->m_symStart < o.m_dci->m_symStart);
  }
};

struct SlotAllocInfo
{
  SlotAllocInfo () = default;

  SlotAllocInfo (SfnSf sfn)
    : m_sfnSf (sfn)
  {
  }

  /**
   * \brief Merge the input parameter to this SlotAllocInfo
   * \param other SlotAllocInfo to merge in this allocation
   *
   * After the merge, order the allocation by symStart in DCI
   */
  void Merge (const SlotAllocInfo & other);

  SfnSf m_sfnSf          {};
  uint32_t m_numSymAlloc {0};    // number of allocated slots
  std::deque<VarTtiAllocInfo> m_varTtiAllocInfo;
};

typedef std::vector<VarTtiAllocInfo::VarTtiType> TddVarTtiTypeList;

struct DlCqiInfo
{
  uint16_t m_rnti {0};
  uint8_t m_ri    {0};
  enum DlCqiType
  {
    WB, SB
  } m_cqiType {WB};
  std::vector<uint8_t> m_rbCqi;   // CQI for each Rsc Block, set to -1 if SINR < Threshold
  uint8_t m_wbCqi {0};   // Wide band CQI
  uint8_t m_wbPmi {0};
};

struct UlCqiInfo
{
  //std::vector <uint16_t> m_sinr;
  std::vector<double> m_sinr;
  enum UlCqiType
  {
    SRS, PUSCH, PUCCH_1, PUCCH_2, PRACH
  } m_type;
};

struct MacCeValue
{
  MacCeValue () :
    m_phr (0), m_crnti (0)
  {
  }
  uint8_t m_phr;
  uint8_t m_crnti;
  std::vector<uint8_t> m_bufferStatus;
};

/**
 * \brief See section 4.3.14 macCEListElement
 */
struct MacCeElement
{
  MacCeElement () :
    m_rnti (0)
  {
  }
  uint16_t m_rnti;
  enum MacCeType
  {
    BSR, PHR, CRNTI
  } m_macCeType;
  struct MacCeValue m_macCeValue;
};

struct RlcListElement
{
  std::vector<struct RlcPduInfo> m_rlcPduElements;
};

struct SchedInfo
{
  SchedInfo () :
    m_frameNum (0), m_subframeNum (0), m_slotNum (0), m_rnti (0)
  {
  }
  SchedInfo (unsigned int numVarTti) :
    m_frameNum (0), m_subframeNum (0), m_slotNum (0), m_rnti (0)
  {
  }

  uint16_t m_frameNum;
  uint8_t m_subframeNum;
  uint16_t m_slotNum;
  uint16_t m_rnti;

  SlotAllocInfo m_slotAllocInfo;
  struct DciInfoElement m_dci;
  std::map<uint8_t, std::vector<struct RlcPduInfo> > m_rlcPduMap;   // RLC PDU elems for each MAC TB
};

struct UePhyPacketCountParameter
{
  uint64_t m_imsi;
  uint32_t m_noBytes;
  bool m_isTx;   //Set to false if Rx and true if tx
  uint32_t m_subframeno;
};

struct EnbPhyPacketCountParameter
{
  uint64_t m_cellId;
  uint32_t m_noBytes;
  bool m_isTx;   //Set to false if Rx and true if tx
  uint32_t m_subframeno;
};

struct RxPacketTraceParams
{
  uint64_t m_cellId;
  uint16_t m_rnti;
  uint32_t m_frameNum;
  uint8_t m_subframeNum;
  uint16_t  m_slotNum;
  uint8_t m_varTtiNum;
  uint8_t m_symStart;
  uint8_t m_numSym;
  uint32_t m_tbSize;
  uint8_t m_mcs;
  uint8_t m_rv;
  double m_sinr;
  double m_sinrMin;
  double m_tbler;
  bool m_corrupt;
  uint8_t m_ccId;
  uint32_t m_rbAssignedNum;
};

/**
 * \brief Store information about HARQ
 *
 * \see DlHarqInfo
 * \see UlHarqInfo
 */
struct HarqInfo
{
  virtual ~HarqInfo ()
  {
  }
  uint16_t m_rnti          {55};   //!< RNTI
  uint8_t m_harqProcessId  {15};   //!< ProcessId
  uint8_t m_numRetx        {5};    //!< Num of Retx

  /**
   * \return true if the HARQ should be eliminated, since the info has been
   * correctly received
   */
  virtual bool IsReceivedOk () const = 0;
};

/**
 * \brief A struct that contains info for the DL HARQ
 */
struct DlHarqInfo : public HarqInfo
{
  /**
   * \brief Status of the DL Harq: ACKed or NACKed
   */
  enum HarqStatus
  {
    ACK, NACK
  } m_harqStatus {NACK};   //!< HARQ status

  virtual bool IsReceivedOk () const override
  {
    return m_harqStatus == ACK;
  }
};

/**
 * \brief A struct that contains info for the UL HARQ
 */
struct UlHarqInfo : public HarqInfo
{
  std::vector<uint16_t> m_ulReception;
  enum ReceptionStatus
  {
    Ok, NotOk, NotValid
  } m_receptionStatus;
  uint8_t m_tpc;

  virtual bool IsReceivedOk () const override
  {
    return m_receptionStatus == Ok;
  }
};


class MmWavePhyMacCommon : public Object
{
public:
  MmWavePhyMacCommon (void);

  ~MmWavePhyMacCommon (void);

  // inherited from Object
  virtual void DoInitialize (void);

  virtual void DoDispose (void);

  static TypeId GetTypeId (void);

  Time GetSymbolPeriod (void) const;

  uint32_t GetCtrlSymbols (void) const;

  uint8_t GetDlCtrlSymbols (void) const;

  uint8_t GetUlCtrlSymbols (void) const;

  uint8_t GetSymbolsPerSlot (void) const;

  Time GetSlotPeriod () const;

  uint32_t GetVarTtisPerSlot (void) const;

  uint32_t GetSubframesPerFrame (void) const;

  uint32_t GetSlotsPerSubframe (void) const;

  uint32_t GetNumReferenceSymbols (void);

  uint8_t GetUlSchedDelay (void) const;

  uint32_t GetNumScsPerRb (void) const;

  double GetSubcarrierSpacing (void) const;

  uint32_t GetNumRefScPerRb (void) const;

  uint32_t GetBandwidthInRbg () const;

  // for TDMA, number of reference subcarriers across entire bandwidth (default to 1/4th of SCs)
  uint32_t GetNumRefScPerSym (void) const;

  uint32_t GetNumRbPerRbg (void) const;

  uint32_t GetNumerology (void) const;

  double GetBandwidth (void) const;

  /*
   * brief: bandwidth in number of RBs
   */
  uint32_t GetBandwidthInRbs () const;

  double GetCenterFrequency (void) const;

  uint16_t GetL1L2CtrlLatency (void) const;

  uint32_t GetL1L2DataLatency (void) const;

  uint32_t GetNumHarqProcess (void) const;

  uint8_t GetHarqTimeout (void) const;

  uint32_t GetTbDecodeLatency (void) const;

  uint32_t GetMaxTbSize (void) const;

  TypeId GetMacSchedType (void) const
  {
    return m_macSchedType;
  }

  void SetSymbolPeriod (double prdSym);

  void SetSymbolsPerSlot (uint8_t numSym);

  void SetSlotPeriod (double period);

  void SetCtrlSymbols (uint32_t ctrlSymbols);

  void SetDlCtrlSymbols (uint8_t ctrlSymbols);

  void SetUlCtrlSymbols (uint8_t ctrlSymbols);

  void SetVarTtiPerSlot (uint32_t numVarTti);

  void SetSubframePerFrame (uint32_t numSf);

  void SetNumReferenceSymbols (uint32_t refSym);

  void SetUlSchedDelay (uint32_t tti);

  void SetNumScsPrRb (uint32_t numScs);

  void SetNumRefScPerRb (uint32_t numRefSc);

  void SetRbNum (uint32_t numRB);

  /*
   * brief
   * rbgSize size of RBG in number of resource blocks
   */
  void SetNumRbPerRbg (uint32_t rbgSize);

  void SetNumerology (uint32_t numerology);

  /*
   * brief Set bandwidth value in Hz
   * param bandwidth the bandwidth value in Hz
   */
  void SetBandwidth (double bandwidth);

  void SetCentreFrequency (double fc);

  void SetL1L2CtrlLatency (uint32_t delaySfs);

  void SetL1L2DataLatency (uint32_t delayVarTtis);

  void SetNumHarqProcess (uint32_t numProcess);

  void SetHarqDlTimeout (uint8_t harqDlTimeout);

  void SetTbDecodeLatency (uint32_t us);

  void SetMaxTbSize (uint32_t bytes);

  void SetCcId (uint8_t ccId);

  uint8_t GetCcId (void);

private:
  Time m_symbolPeriod;
  uint8_t m_symbolsPerSlot;
  Time m_slotPeriod;
  uint32_t m_ctrlSymbols;
  uint8_t m_dlCtrlSymbols;   // num OFDM symbols for downlink control at beginning of subframe
  uint8_t m_ulCtrlSymbols;   // num OFDM symbols for uplink control at end of subframe
  uint32_t m_fixedTtisPerSlot;   // TODO: check if this is obsolete attribute
  uint32_t m_slotsPerSubframe;   // TODO: perform parameter cleanup, leave only mandatory ones, many redundant settings
  uint32_t m_subframesPerFrame;
  uint32_t m_numRefSymbols;
  uint32_t m_numRbPerRbg;
  uint16_t m_numerology;
  double m_subcarrierSpacing;
  uint32_t m_rbNum;   // replaced nyu chunk
  uint32_t m_numRefScPerRb;
  uint32_t m_numSubCarriersPerRb;
  uint8_t m_numHarqProcess;
  uint8_t m_harqTimeout;
  double m_centerFrequency;
  double m_bandwidth;
  bool m_bandwidthConfigured;
  uint16_t m_l1L2CtrlLatency;   // In no. of sub-frames
  uint32_t m_l1L2DataLatency;   // In no. of slots - TODO: check if this is correct description
  uint32_t m_ulSchedDelay;   // delay between transmission of UL-DCI and corresponding subframe in TTIs
  uint32_t m_wbCqiPeriodUs;     // WB CQI periodicity in microseconds
  uint32_t m_tbDecodeLatencyUs;
  uint32_t m_maxTbSizeBytes;
  std::string m_staticTddPattern;
  TypeId m_macSchedType;
  uint8_t m_componentCarrierId;
};

std::ostream & operator<< (std::ostream & os, VarTtiAllocInfo::TddMode const & item);
std::ostream & operator<< (std::ostream & os, DlHarqInfo const & item);
std::ostream & operator<< (std::ostream & os, UlHarqInfo const & item);
std::ostream & operator<< (std::ostream & os, SfnSf const & item);
}

#endif /* SRC_MMWAVE_MODEL_MMWAVE_PHY_MAC_COMMON_H_ */
