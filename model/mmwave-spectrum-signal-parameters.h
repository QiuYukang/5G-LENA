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


#ifndef MMWAVE_SPECTRUM_SIGNAL_PARAMETERS_H
#define MMWAVE_SPECTRUM_SIGNAL_PARAMETERS_H

#include <list>
#include <ns3/spectrum-signal-parameters.h>

namespace ns3 {

class PacketBurst;
class MmWaveControlMessage;

/**
 * \ingroup spectrum
 *
 * \brief Data signal representation for the module
 *
 * This struct provides the generic signal representation to be used by the module
 * for what regards the data part.
 */
struct MmwaveSpectrumSignalParametersDataFrame : public SpectrumSignalParameters
{

  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();

  /**
   * \brief MmwaveSpectrumSignalParametersDataFrame
   */
  MmwaveSpectrumSignalParametersDataFrame ();


  /**
   * \brief MmwaveSpectrumSignalParametersDataFrame copy constructor
   * \param p the object from which we have to copy things
   */
  MmwaveSpectrumSignalParametersDataFrame (const MmwaveSpectrumSignalParametersDataFrame& p);

  Ptr<PacketBurst> packetBurst;                       //!< Packet burst
  std::list<Ptr<MmWaveControlMessage> > ctrlMsgList;  //!< List of contrl messages
  uint16_t cellId;                                    //!< CellId
  uint8_t slotInd;                                    //!< Slot indication (?)
};

/**
 * \ingroup gnb-phy
 * \ingroup ue-phy
 *
 * \brief DL CTRL signal representation for the module
 *
 * This struct provides the generic signal representation to be used by the module
 * for what regards the downlink control part.
 */
struct MmWaveSpectrumSignalParametersDlCtrlFrame : public SpectrumSignalParameters
{

  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();

  /**
   * \brief MmWaveSpectrumSignalParametersDlCtrlFrame
   */
  MmWaveSpectrumSignalParametersDlCtrlFrame ();

  /**
   * \brief MmWaveSpectrumSignalParametersDlCtrlFrame copy constructor
   * \param p the object from which we have to copy from
   */
  MmWaveSpectrumSignalParametersDlCtrlFrame (const MmWaveSpectrumSignalParametersDlCtrlFrame& p);


  std::list<Ptr<MmWaveControlMessage> > ctrlMsgList;  //!< CTRL message list
  bool pss;                                           //!< PSS (?)
  uint16_t cellId;                                    //!< cell id
};

/**
 * \ingroup gnb-phy
 * \ingroup ue-phy
 *
 * \brief UL CTRL signal representation for the module
 *
 * This struct provides the generic signal representation to be used by the module
 * for what regards the UL CTRL part.
 */
struct MmWaveSpectrumSignalParametersUlCtrlFrame : public SpectrumSignalParameters
{

  // inherited from SpectrumSignalParameters
  virtual Ptr<SpectrumSignalParameters> Copy ();

  /**
   * \brief MmWaveSpectrumSignalParametersUlCtrlFrame
   */
  MmWaveSpectrumSignalParametersUlCtrlFrame ();


  /**
   * \brief MmWaveSpectrumSignalParametersUlCtrlFrame copy constructor
   * \param p the object from which we have to copy from
   */
  MmWaveSpectrumSignalParametersUlCtrlFrame (const MmWaveSpectrumSignalParametersUlCtrlFrame& p);


  std::list<Ptr<MmWaveControlMessage> > ctrlMsgList;  //!< CTRL message list
  uint16_t cellId;                                    //!< cell id
};


}  // namespace ns3


#endif /* MMWAVE_SPECTRUM_SIGNAL_PARAMETERS_H */
