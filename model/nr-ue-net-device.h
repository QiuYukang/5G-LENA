// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_UE_NET_DEVICE_H
#define NR_UE_NET_DEVICE_H

#include "nr-net-device.h"

namespace ns3
{

class Packet;
class PacketBurst;
class Node;
class NrUePhy;
class NrUeMac;
class NrUeComponentCarrierManager;
class NrEpcUeNas;
class NrUeRrc;
class NrGnbNetDevice;
class BandwidthPartUe;
class BwpManagerUe;

/**
 * \ingroup ue
 * \brief The User Equipment NetDevice
 *
 * This class represent the netdevice of the UE. This class is the contact
 * point between the TCP/IP part (from internet and network modules) and the
 * NR part.
 */
class NrUeNetDevice : public NrNetDevice
{
  public:
    /**
     * \brief GetTypeId
     * \return
     */
    static TypeId GetTypeId();

    /**
     * \brief NrUeNetDevice
     */
    NrUeNetDevice();

    /**
     * \brief ~NrUeNetDevice
     */
    ~NrUeNetDevice() override;

    /**
     * \brief GetCsgId ?
     * \return ?
     */
    uint32_t GetCsgId() const;

    /**
     * \brief SetCsgId ?
     * \param csgId ?
     */
    void SetCsgId(uint32_t csgId);

    /**
     * \brief Obtain a pointer to the PHY at the index specified
     * \param index bandwidth part index
     * \return the pointer to the PHY selected
     */
    Ptr<NrUePhy> GetPhy(uint8_t index) const;

    /**
     * \brief Obtain a pointer to the MAC at the index specified
     * \param index bandwidth part index
     * \return the pointer to the MAC selected
     */
    Ptr<NrUeMac> GetMac(uint8_t index) const;

    /**
     * \brief Get the bandwidth part manager
     * \return a pointer to the BWP manager
     */
    Ptr<BwpManagerUe> GetBwpManager() const;

    /**
     * \brief Get the Imsi
     * \return UE imsi
     */
    uint64_t GetImsi() const;

    /**
     * \brief Get the CellId
     * \return cell ID
     */
    uint16_t GetCellId() const;

    /**
     * \brief Get a pointer to the Nas
     * \return the NAS pointer
     */
    Ptr<NrEpcUeNas> GetNas() const;

    /**
     * \brief Get a Rrc pointer
     * \return RRC pointer
     */
    Ptr<NrUeRrc> GetRrc() const;

    /**
     * \brief Set the GNB to which this UE is attached to
     * \param gnb GNB to attach to
     *
     * This method may change once we implement handover.
     */
    void SetTargetGnb(Ptr<NrGnbNetDevice> gnb);

    /**
     * \brief Obtain a pointer to the target gnb
     * \return a pointer to the target gnb
     */
    Ptr<const NrGnbNetDevice> GetTargetGnb() const;

    /**
     * \brief Set the NrComponentCarrier Map for the UE
     * \param ccm the map of ComponentCarrierUe
     */
    void SetCcMap(std::map<uint8_t, Ptr<BandwidthPartUe>> ccm);

    /**
     * \brief Get the NrComponentCarrier Map for the UE
     * \returns the map of ComponentCarrierUe
     */
    std::map<uint8_t, Ptr<BandwidthPartUe>> GetCcMap();

    /**
     * \brief Get the size of the component carriers map
     * \return the number of cc that we have
     */
    uint32_t GetCcMapSize() const;

    /**
     * \brief Spectrum has calculated the HarqFeedback for one DL transmission,
     * and give it to the NetDevice of the UE.
     *
     * The NetDevice find the best BWP to forward the Harq Feedback, and then
     * forward it to the PHY of the selected BWP.
     *
     * \param m feedback
     */
    void EnqueueDlHarqFeedback(const DlHarqInfo& m) const;

    /**
     * \brief The UE received a CTRL message list.
     *
     * The UE should divide the messages to the BWP they pertain to.
     *
     * \param msgList Message list
     * \param sourceBwpId BWP Id from which the list originated
     */
    void RouteIngoingCtrlMsgs(const std::list<Ptr<NrControlMessage>>& msgList, uint8_t sourceBwpId);

    /**
     * \brief Route the outgoing messages to the right BWP
     * \param msgList the list of messages
     * \param sourceBwpId the source bwp of the messages
     */
    void RouteOutgoingCtrlMsgs(const std::list<Ptr<NrControlMessage>>& msgList,
                               uint8_t sourceBwpId);

    /**
     * \brief Update the RRC config. Must be called only once.
     */
    void UpdateConfig();

  protected:
    // inherited from Object
    void DoInitialize() override;
    void DoDispose() override;

    // inherited from NetDevice
    bool DoSend(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) override;

  private:
    Ptr<NrGnbNetDevice> m_targetGnb; //!< GNB pointer
    Ptr<NrUeRrc> m_rrc;              //!< RRC pointer
    Ptr<NrEpcUeNas> m_nas;           //!< NAS pointer
    uint64_t m_imsi;                 //!< UE IMSI
    uint32_t m_csgId;                //!< ?_?

    std::map<uint8_t, Ptr<BandwidthPartUe>> m_ccMap;            ///< component carrier map
    Ptr<NrUeComponentCarrierManager> m_componentCarrierManager; ///< the component carrier manager
};

} // namespace ns3
#endif /* NR_UE_NET_DEVICE_H */
