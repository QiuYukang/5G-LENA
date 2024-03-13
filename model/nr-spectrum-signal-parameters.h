/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_SPECTRUM_SIGNAL_PARAMETERS_H
#define NR_SPECTRUM_SIGNAL_PARAMETERS_H

#include <ns3/spectrum-signal-parameters.h>

#include <list>

namespace ns3
{

class PacketBurst;
class NrControlMessage;
class NrSlHarqFeedbackMessage;

/**
 * \ingroup spectrum
 *
 * \brief Data signal representation for the module
 *
 * This struct provides the generic signal representation to be used by the module
 * for what regards the data part.
 */
struct NrSpectrumSignalParametersDataFrame : public SpectrumSignalParameters
{
    // inherited from SpectrumSignalParameters
    Ptr<SpectrumSignalParameters> Copy() const override;

    /**
     * \brief NrSpectrumSignalParametersDataFrame
     */
    NrSpectrumSignalParametersDataFrame();

    /**
     * \brief NrSpectrumSignalParametersDataFrame copy constructor
     * \param p the object from which we have to copy things
     */
    NrSpectrumSignalParametersDataFrame(const NrSpectrumSignalParametersDataFrame& p);

    Ptr<PacketBurst> packetBurst;                 //!< Packet burst
    std::list<Ptr<NrControlMessage>> ctrlMsgList; //!< List of control messages
    uint16_t cellId;                              //!< CellId
    uint16_t rnti{0};                             //!< RNTI of the transmitting or receiving UE
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
struct NrSpectrumSignalParametersDlCtrlFrame : public SpectrumSignalParameters
{
    // inherited from SpectrumSignalParameters
    Ptr<SpectrumSignalParameters> Copy() const override;

    /**
     * \brief NrSpectrumSignalParametersDlCtrlFrame
     */
    NrSpectrumSignalParametersDlCtrlFrame();

    /**
     * \brief NrSpectrumSignalParametersDlCtrlFrame copy constructor
     * \param p the object from which we have to copy from
     */
    NrSpectrumSignalParametersDlCtrlFrame(const NrSpectrumSignalParametersDlCtrlFrame& p);

    std::list<Ptr<NrControlMessage>> ctrlMsgList; //!< CTRL message list
    bool pss;                                     //!< PSS (?)
    uint16_t cellId;                              //!< cell id
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
struct NrSpectrumSignalParametersUlCtrlFrame : public SpectrumSignalParameters
{
    // inherited from SpectrumSignalParameters
    Ptr<SpectrumSignalParameters> Copy() const override;

    /**
     * \brief NrSpectrumSignalParametersUlCtrlFrame
     */
    NrSpectrumSignalParametersUlCtrlFrame();

    /**
     * \brief NrSpectrumSignalParametersUlCtrlFrame copy constructor
     * \param p the object from which we have to copy from
     */
    NrSpectrumSignalParametersUlCtrlFrame(const NrSpectrumSignalParametersUlCtrlFrame& p);

    std::list<Ptr<NrControlMessage>> ctrlMsgList; //!< CTRL message list
    uint16_t cellId;                              //!< cell id
};

// NR SL

/**
 * \ingroup gnb-phy
 * \ingroup ue-phy
 *
 * Signal parameters for NR SL Frame
 */
struct NrSpectrumSignalParametersSlFrame : public SpectrumSignalParameters
{
    // inherited from SpectrumSignalParameters
    Ptr<SpectrumSignalParameters> Copy() const override;

    /**
     * \brief NrSlSpectrumSignalParametersSlFrame default constructor
     */
    NrSpectrumSignalParametersSlFrame();

    /**
     * \brief NrSlSpectrumSignalParametersSlFrame copy constructor
     * \param p The NrSlSpectrumSignalParametersSlFrame
     */
    NrSpectrumSignalParametersSlFrame(const NrSpectrumSignalParametersSlFrame& p);

    Ptr<PacketBurst> packetBurst; //!< The packet burst being transmitted with this signal
    uint32_t nodeId{std::numeric_limits<uint32_t>::max()}; //!< Node id
                                                           // TODO
    // uint64_t slssId; //!< The Sidelink synchronization signal identifier of the transmitting UE
};

/**
 * \ingroup gnb-phy
 * \ingroup ue-phy
 *
 * Signal parameters for NR SL CTRL Frame (PSCCH)
 */
struct NrSpectrumSignalParametersSlCtrlFrame : public NrSpectrumSignalParametersSlFrame
{
    // inherited from SpectrumSignalParameters
    Ptr<SpectrumSignalParameters> Copy() const override;

    /**
     * NrSlSpectrumSignalParametersSlCtrlFrame default constructor
     */
    NrSpectrumSignalParametersSlCtrlFrame();

    /**
     * \brief NrSlSpectrumSignalParametersSlCtrlFrame copy constructor
     * \param p The NrSlSpectrumSignalParametersSlFrame
     */
    NrSpectrumSignalParametersSlCtrlFrame(const NrSpectrumSignalParametersSlCtrlFrame& p);
};

/**
 * \ingroup gnb-phy
 * \ingroup ue-phy
 *
 * Signal parameters for NR SL DATA Frame (PSSCH)
 */
struct NrSpectrumSignalParametersSlDataFrame : public NrSpectrumSignalParametersSlFrame
{
    // inherited from SpectrumSignalParameters
    Ptr<SpectrumSignalParameters> Copy() const override;

    /**
     * \brief NrSlSpectrumSignalParametersSlDataFrame default constructor
     */
    NrSpectrumSignalParametersSlDataFrame();

    /**
     * \brief NrSlSpectrumSignalParametersSlDataFrame copy constructor
     * \param p The NrSlSpectrumSignalParametersSlFrame
     */
    NrSpectrumSignalParametersSlDataFrame(const NrSpectrumSignalParametersSlDataFrame& p);
};

/**
 * \ingroup gnb-phy
 * \ingroup ue-phy
 *
 * Signal parameters for NR SL feedback (PSFCH)
 */
struct NrSpectrumSignalParametersSlFeedback : public NrSpectrumSignalParametersSlFrame
{
    // inherited from SpectrumSignalParameters
    Ptr<SpectrumSignalParameters> Copy() const override;

    /**
     * \brief NrSlSpectrumSignalParametersSlFeedback default constructor
     */
    NrSpectrumSignalParametersSlFeedback();

    /**
     * \brief NrSlSpectrumSignalParametersSlFeedback copy constructor
     * \param p The NrSlSpectrumSignalParametersSlFrame
     */
    NrSpectrumSignalParametersSlFeedback(const NrSpectrumSignalParametersSlFeedback& p);
    std::list<Ptr<NrSlHarqFeedbackMessage>> feedbackList; //!< Feedback message list
};

} // namespace ns3

#endif /* NR_SPECTRUM_SIGNAL_PARAMETERS_H */
