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


#ifndef CC_BWP_HELPER_H
#define CC_BWP_HELPER_H


#include <ns3/mmwave-phy-mac-common.h>
#include <ns3/mmwave-control-messages.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/mmwave-3gpp-channel.h>
#include <memory>

namespace ns3 {


/*
 * Upper limits of the number of component carriers used for Carrier
 * Aggregation (CA). In NR, this number depends on the CC contiguousness.
 * Eventually, the number of CCs may also depend on the operation frequency
 */
static const uint8_t MAX_CC_INTRA_BAND = 8;  //!< In NR Rel. 16, up to 8 CCs can be aggregated in the same operation band
static const uint8_t MAX_CC_INTER_BAND = 16; //!< The maximum number of aggregated CCs is 16 in NR Rel. 16 (in more than one operation band)


/**
 * \brief Bandwidth part configuration information
 */
struct BandwidthPartInfo
{
  uint8_t m_bwpId {0};             //!< BWP id
  double m_centralFrequency {0};   //!< BWP central frequency
  double m_lowerFrequency {0};     //!< BWP lower frequency
  double m_higherFrequency {0};    //!< BWP higher frequency
  uint32_t m_bandwidth {0};        //!< BWP bandwidth

  Ptr<SpectrumChannel> m_channel;         //!< Channel for the Bwp. Leave it nullptr to let the helper fill it
  Ptr<PropagationLossModel> m_propagation;//!< Propagation model. Leave it nullptr to let the helper fill it
  Ptr<MmWave3gppChannel> m_3gppChannel;   //!< MmWave Channel. Leave it nullptr to let the helper fill it
};

typedef std::unique_ptr<BandwidthPartInfo> BandwidthPartInfoPtr;

/**
 * \brief Component carrier configuration element
 */
struct ComponentCarrierInfo
{
  uint8_t m_ccId {0};          //!< CC id
  double m_centralFrequency {0};   //!< BWP central frequency
  double m_lowerFrequency {0};     //!< BWP lower frequency
  double m_higherFrequency {0};    //!< BWP higher frequency
  double m_bandwidth {0};          //!< BWP bandwidth

  std::unordered_map<uint32_t, BandwidthPartInfoPtr> m_bwp;  //!< Space for BWP

  /**
   * \brief Adds a bandwidth part configuration to the carrier
   *
   * \param bwp Description of the BWP to be added
   */
  bool AddBwp (uint32_t id, BandwidthPartInfoPtr &&bwp);
};

typedef std::unique_ptr<ComponentCarrierInfo> ComponentCarrierInfoPtr;


/**
 * \brief Operation band information structure
 *
 * Defines the range of frequencies of an operation band and includes a list of
 * component carriers (CC) and their contiguousness
 */
struct OperationBandInfo
{
  uint8_t m_bandId          {0};    //!< Operation band id
  double m_centralFrequency {0.0};  //!< Operation band central frequency
  double m_lowerFrequency   {0.0};  //!< Operation band lower frequency
  double m_higherFrequency  {0.0};  //!< Operation band higher frequency
  double m_bandwidth      {0};    //!< Operation band bandwidth

  std::unordered_map<uint32_t, ComponentCarrierInfoPtr> m_cc;

  /**
   * \brief Adds the component carrier definition given as an input reference
   * to the current operation band configuration
   *
   * \param id Where to put the cc
   * \param cc The information of the component carrier to be created
   */
  bool AddCc (uint32_t id, ComponentCarrierInfoPtr &&cc);

  /**
   * @brief GetBwpAt
   * @param ccId
   * @param bwpId
   * @return
   */
  BandwidthPartInfoPtr & GetBwpAt (uint32_t ccId, uint32_t bwpId) const;
};

/**
 * \brief Manages the correct creation of operation bands, component carriers and bandwidth parts
 */
class CcBwpCreator
{
public:
  /**
   * \brief Minimum configuration requirements for a OperationBand
   */
  struct SimpleOperationBandConf
  {
    /**
     * \brief Default constructor
     * \param centralFreq Central Frequency
     * \param bw Bandwidth
     * \param num Numerology
     * \param sched Scheduler
     */
    SimpleOperationBandConf (double centralFreq = 28e9, double bw = 400e6, uint8_t numCc = 1)
      : m_centralFrequency (centralFreq), m_bandwidth (bw), m_numCc (numCc)
    {
    }
    double m_centralFrequency {28e9};   //!< Central Freq
    double m_bandwidth        {400e6};  //!< Total Bandwidth of the operation band
    uint8_t m_numCc           {1};      //!< Number of CC
  };

  /**
   * \brief Creates an operation band by splitting the available bandwidth into
   * equally-large contiguous carriers. Carriers will have common parameters like numerology.
   *
   * \param conf Minimum configuration
   */
  OperationBandInfo CreateOperationBandContiguousCc (const SimpleOperationBandConf &conf);

  /**
   * \brief Creates an operation band with non-contiguous CC.
   *
   * \param conf Minimum configuration for every CC.
   */
  OperationBandInfo CreateOperationBandNonContiguousCc (const std::vector<SimpleOperationBandConf> &configuration);

  /**
   * \brief Plots the CA/BWP configuration using GNUPLOT. There must be a valid
   * configuration
   *
   * \param filename The path to write the output gnuplot file
   */
  static void PlotNrCaBwpConfiguration (const std::vector<OperationBandInfo> &bands,
                                        const std::string &filename);

  /**
   * \brief Plots the CA/BWP configuration using GNUPLOT. There must be a valid
   * configuration
   *
   * \param filename The path to write the output gnuplot file
   */
  static void PlotLteCaConfiguration (const std::vector<OperationBandInfo> &bands,
                                      const std::string &filename);


private:
  void InitializeCc (std::unique_ptr<ComponentCarrierInfo> &cc,
                     double ccBandwidth, double lowerFreq, uint32_t ccId);
  std::unique_ptr<ComponentCarrierInfo> CreateCc (double ccBandwidth, double lowerFreq, uint32_t ccId);

  /**
   * \brief Plots a 2D rectangle defined by the input points and places a label
   *
   * \param outFile The output
   * \param index The drawn rectangle id
   * \param xmin The minimum value of the rectangle in the horizontal (x) axis
   * \param xmax The maximum value of the rectangle in the horizontal (x) axis
   * \param ymin The minimum value of the rectangle in the vertical (y) axis
   * \param ymax The minimum value of the rectangle in the vertical (y) axis
   * \param label The text to be printed inside the rectangle
   */
  static void PlotFrequencyBand (std::ofstream &outFile,
                                 uint16_t index,
                                 double xmin,
                                 double xmax,
                                 double ymin,
                                 double ymax,
                                 const std::string &label);

  uint8_t m_bwpCounter {0};
  uint8_t m_opBandCounter {0};
};

}

#endif /* CC_BWP_HELPER_H */
