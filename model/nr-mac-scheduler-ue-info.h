// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "beam-id.h"
#include "nr-amc.h"
#include "nr-mac-harq-vector.h"
#include "nr-mac-sched-sap.h"
#include "nr-mac-scheduler-lcg.h"

#include "ns3/matrix-array.h"

#include <functional>
#include <optional>
#include <unordered_map>

namespace ns3
{

class NrMacSchedulerUeInfo;
/**
 * @brief Shared pointer to an instance of NrMacSchedulerUeInfo
 */
typedef std::shared_ptr<NrMacSchedulerUeInfo> UePtr;

/**
 * @ingroup scheduler
 * @brief The representation of an user for any Mac scheduler
 *
 * Basic representation for an UE inside any scheduler. The class is responsible
 * to store all the UE-related field that can be used by any scheduler.
 *
 * If a scheduler needs to store additional information, it is necessary to
 * create a subclass and store the information there. Then, the scheduler
 * will return a pointer to a newly created instance through
 * NrMacSchedulerNs3::CreateUeRepresentation.
 *
 * The class stores information such as RBG, MCS, and CQI. Information that
 * should be reset after each slot (such as RBG count) should be reset in the
 * method ResetDlSchedInfo() and ResetUlSchedInfo().
 *
 * When a scheduler assign new resources to the UE, it will call
 * the method UpdateDlMetric() or UpdateUlMetric(). Make sure all the relevant
 * information that should be updated for a correct sorting are updated there.
 *
 * @see NrMacSchedulerUeInfoRR
 * @see NrMacSchedulerUeInfoPF
 * @see NrMacSchedulerUeInfoMR
 */
class NrMacSchedulerUeInfo
{
  public:
    /**
     * @brief Default Constructor (deleted)
     */
    NrMacSchedulerUeInfo() = delete;

    typedef std::function<uint32_t()> GetRbPerRbgFn;

    /**
     * @brief Create a new UE representation
     * @param rnti the RNTI of the UE
     * @param beamId the BeamID of the UE (can be updated later)
     */
    NrMacSchedulerUeInfo(uint16_t rnti, BeamId beamId, const GetRbPerRbgFn& fn);

    /**
     * @brief ~NrMacSchedulerUeInfo deconstructor
     */
    virtual ~NrMacSchedulerUeInfo();

    /**
     * @brief GetDlRBG
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static std::vector<uint16_t>& GetDlRBG(const UePtr& ue);
    /**
     * @brief GetUlRBG
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static std::vector<uint16_t>& GetUlRBG(const UePtr& ue);
    /**
     * @brief GetDlSym
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static std::vector<uint8_t>& GetDlSym(const UePtr& ue);
    /**
     * @brief GetUlSym
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static std::vector<uint8_t>& GetUlSym(const UePtr& ue);

    enum class McsCsiSource
    {
        AVG_MCS,      //!< Estimate MCS based on the average MCS of allocated RBGs
        AVG_SPEC_EFF, //!< Estimate MCS based on the average spectral efficiency of allocated RBGs
        AVG_SINR,     //!< Estimate MCS based on the average SINR of allocated RBGs
        WIDEBAND_MCS  //!< Wideband MCS
    };
    /**
     * @brief Get the downlink MCS, given by the wideband CQI, or
     *        the sub-band CQIs of the currently allocated RBGs, if available
     * @return downlink mcs
     */
    uint8_t GetDlMcs() const;
    /**
     * @brief GetDlMcs
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static uint8_t& GetDlMcs(const UePtr& ue);
    /**
     * @brief GetUlMcs
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static uint8_t& GetUlMcs(const UePtr& ue);
    /**
     * @brief GetDlTBS
     * @param ue UE pointer from which obtain the value
     * @return The TB size
     */
    static uint32_t& GetDlTBS(const UePtr& ue);

    /**
     * @brief GetUlTBS
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static uint32_t& GetUlTBS(const UePtr& ue)
    {
        return ue->m_ulTbSize;
    }

    /**
     * @brief GetDlLCG
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static std::unordered_map<uint8_t, LCGPtr>& GetDlLCG(const UePtr& ue);
    /**
     * @brief GetUlLCG
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static std::unordered_map<uint8_t, LCGPtr>& GetUlLCG(const UePtr& ue);
    /**
     * @brief GetDlHarqVector
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static NrMacHarqVector& GetDlHarqVector(const UePtr& ue);
    /**
     * @brief GetUlHarqVector
     * @param ue UE pointer from which obtain the value
     * @return
     */
    static NrMacHarqVector& GetUlHarqVector(const UePtr& ue);

    /**
     * @brief Prints information related to the QCI of a UEs LC
     * @param ue UE for which we want to print the LC info
     * @param lcgId The logical channel group ID
     * @param lcId The logical channel ID
     * @param qci The QCI of this LC
     * @param P The priority associated to the QCI of this LC
     * @return
     */
    static void PrintLcInfo(uint16_t ue,
                            uint8_t lcgId,
                            uint8_t lcId,
                            uint8_t cqi,
                            uint8_t P,
                            uint8_t minP);

    typedef std::function<std::unordered_map<uint8_t, LCGPtr>&(const UePtr& ue)> GetLCGFn;
    typedef std::function<NrMacHarqVector&(const UePtr& ue)> GetHarqVectorFn;

    /**
     * @brief Reset DL information
     *
     * Called after each slot. It should reset all the information that are
     * slot-dependent.
     */
    virtual void ResetDlSchedInfo();

    /**
     * @brief Reset UL information
     *
     * Called after each slot. It should reset all the information that are
     * slot-dependent.
     */
    virtual void ResetUlSchedInfo();

    /**
     * @brief Update DL metrics after resources have been assigned
     *
     * The amount of assigned resources is stored inside m_dlRBG by the scheduler.
     */
    virtual void UpdateDlMetric();

    /**
     * @brief ResetDlMetric
     *
     * Called when the scheduler has assigned RBGs, but the sum does not arrive
     * to a TBS > 0. The assignation is, therefore, not transformed in DCI.
     * These RBG will not be assigned, they will be empty in the slot.
     */
    virtual void ResetDlMetric();

    /**
     * @brief Update UL metrics after resources have been assigned
     *
     * The amount of assigned resources is stored inside m_ulRBG by the scheduler.
     */
    virtual void UpdateUlMetric();

    /**
     * @brief ResetUlMetric
     *
     * Called when the scheduler has assigned RBGs, but the sum does not arrive
     * to a TBS > 0. The assignation is, therefore, not transformed in DCI.
     * These RBG will not be assigned, they will be empty in the slot.
     */
    virtual void ResetUlMetric();

    uint32_t GetTotalDlBuffer() const;

    /**
     * @brief Received CQI information
     */
    struct CqiInfo
    {
        /**
         * @brief Type of CQI
         */
        enum CqiType
        {
            WB,          //!< Wide-band
            SB           //!< Sub-band
        } m_cqiType{WB}; //!< CQI type

        std::vector<double> m_sinr;   //!< Vector of SINR for the entire band
        uint8_t m_wbCqi{0};           //!< CQI reported value
        std::vector<uint8_t> m_sbCqi; //!< Sub-band CQI reported values
        uint32_t m_timer{0};          //!< Timer (in slot number).
                                      //!< When the timer is 0, the value is discarded
    };

    void ReleaseLC(uint8_t lcid);

    uint16_t m_rnti{0}; //!< RNTI of the UE
    BeamId m_beamId;    //!< Beam ID of the UE (kept updated as much as possible by MAC)

    std::unordered_map<uint8_t, LCGPtr> m_dlLCG; //!< DL LCG
    std::unordered_map<uint8_t, LCGPtr> m_ulLCG; //!< UL LCG

    uint32_t m_dlMRBRetx{0};       //!< MRB assigned for retx. To update the name,
                                   //!< what is MRB is not defined
    uint32_t m_ulMRBRetx{0};       //!< MRB assigned for retx. To update the name,
                                   //!< what is MRB is not defined
    std::vector<uint16_t> m_dlRBG; //!< DL Resource Block Group assigned in this slot
    std::vector<uint16_t> m_ulRBG; //!< UL Resource Block Group assigned in this slot
    std::vector<uint8_t> m_dlSym;  //!< Corresponding symbol of m_dlRBG in this slot
    std::vector<uint8_t> m_ulSym;  //!< Corresponding symbol of m_ulRBG in this slot

    uint8_t m_dlMcs{0}; //!< DL MCS
    std::optional<uint8_t>
        m_fhMaxMcsAssignable; //!< Maximum DL MCS assignable due to FH limitations
    uint8_t m_ulMcs{0};       //!< UL MCS

    struct SbMcsInfo
    {
        uint8_t cqi;
        uint8_t mcs;
        float specEff;
        float sinr;
    };

    std::vector<SbMcsInfo> m_dlSbMcsInfo; //!< Precomputed MCS, Spectral Efficiency and estimated
                                          //!< SINR for a sub-band CQI associated with a RBG
    std::vector<uint8_t> m_rbgToSb;       //!< Precomputed RBG to SB mapping

    uint32_t m_dlTbSize{0}; //!< DL Transport Block Size, depends on MCS and RBG,
                            //!< updated in UpdateDlMetric()
    uint32_t m_ulTbSize{0}; //!< UL Transport Block Size, depends on MCS and RBG,
                            //!< updated in UpdateDlMetric()
    uint8_t m_dlRank{1};    //!< DL rank (number of MIMO layers)
    uint8_t m_ulRank{1};    //!< UL rank (number of MIMO layers)
    Ptr<const ComplexMatrixArray> m_dlPrecMats{nullptr}; //!< DL Precoding matrices
    Ptr<const ComplexMatrixArray> m_ulPrecMats{nullptr}; //!< UL Precoding matrices

    CqiInfo m_dlCqi; //!< DL CQI information
    CqiInfo m_ulCqi; //!< UL CQI information

    NrMacHarqVector m_dlHarq; //!< HARQ process vector for DL
    NrMacHarqVector m_ulHarq; //!< HARQ process vector for UL

    uint32_t m_srsPeriodicity{0}; //!< SRS periodicity
    uint32_t m_srsOffset{0};      //!< SRS offset
    uint8_t m_startMcsDlUe{0};    //!< Starting DL MCS to be used

    // Settings from the scheduler, that affects MCS, TBS and throughput computation
    Ptr<NrAmc> m_dlAmc;          //!< AMC instance of scheduler associated with DL
    Ptr<NrAmc> m_ulAmc;          //!< AMC instance of scheduler associated with UL
    McsCsiSource m_mcsCsiSource; //!< Source of MCS computation based on CSI feedback

  protected:
    /**
     * @brief Retrieve the number of RB per RBG
     *
     * @return numRbPerRbg. Calls the MAC.
     */
    uint32_t GetNumRbPerRbg() const;

  private:
    const GetRbPerRbgFn m_getNumRbPerRbg; //!< Function that points to a method which knows the
                                          //!< number of RB per RBG.
};

} // namespace ns3
