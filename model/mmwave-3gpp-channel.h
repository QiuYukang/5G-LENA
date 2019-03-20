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
 */


#ifndef MMWAVE_3GPP_CHANNEL_H_
#define MMWAVE_3GPP_CHANNEL_H_


#include <ns3/spectrum-value.h>
#include <complex>
#include <ns3/spectrum-signal-parameters.h>
#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <map>
#include <ns3/angles.h>
#include <ns3/random-variable-stream.h>
#include <ns3/spectrum-propagation-loss-model.h>
#include "mmwave-3gpp-propagation-loss-model.h"
#include "mmwave-3gpp-buildings-propagation-loss-model.h"
#include "antenna-array-basic-model.h"

#define AOA_INDEX 0
#define ZOA_INDEX 1
#define AOD_INDEX 2
#define ZOD_INDEX 3

#define PHI_INDEX 0
#define X_INDEX 1
#define THETA_INDEX 2
#define Y_INDEX 3
#define R_INDEX 4

namespace ns3 {


typedef std::vector<double> doubleVector_t;
typedef std::vector<doubleVector_t> double2DVector_t;
typedef std::vector< std::complex<double> > complexVector_t;
typedef std::vector<complexVector_t> complex2DVector_t;
typedef std::vector<complex2DVector_t> complex3DVector_t;
typedef std::pair<Ptr<NetDevice>, Ptr<NetDevice> > key_t;

std::ostream & operator<< (std::ostream & os, doubleVector_t const & item);
std::ostream & operator<< (std::ostream & os, double2DVector_t const & item);
std::ostream & operator<< (std::ostream & os, complexVector_t const & item);
std::ostream & operator<< (std::ostream & os, complex2DVector_t const & item);
std::ostream & operator<< (std::ostream & os, complex3DVector_t const & item);

struct InputParams3gpp
{
  bool m_los {false};
  bool m_o2i {false};
  Vector m_speed {0,0,0};
  double m_dis2D {0};
  double m_dis3D {0};
  key_t m_key {0,0}; //!< the key formed of the pointers to the TX and RX device, respectively
  key_t m_keyReverse {0,0}; //!< //!< the key formed of the pointers to the RX and TX device, respectively

  bool m_initialized {false};

public:

  inline InputParams3gpp()
   {
    m_initialized = false;
   }
  inline InputParams3gpp (bool los, bool o2i, Vector speed, double dis2D, double dis3D, key_t key, key_t keyReverse):
    m_los(los), m_o2i(o2i), m_speed(speed), m_dis2D (dis2D), m_dis3D (dis3D), m_key(key), m_keyReverse (keyReverse)
  {
    m_initialized = true;
  }

  inline InputParams3gpp (const InputParams3gpp &p)
   {
     m_los = p.m_los;
     m_o2i = p.m_o2i;
     m_speed = p.m_speed;
     m_dis2D = p.m_dis2D;
     m_dis3D = p.m_dis3D;
     m_key = p.m_key;
     m_keyReverse = p.m_keyReverse;
     m_initialized = p.m_initialized;
   }

  inline bool GetLos () const {return m_los;}
  inline bool Geto2i () const {return m_o2i;}
  inline Vector GetSpeed ()const {return m_speed;}
  inline double GetDis2D () const {return m_dis2D;}
  inline double GetDis3D () const {return m_dis3D;}
  inline key_t GetKey () const {return m_key;}
  inline key_t GetKeyReverse () const {return m_keyReverse;}
  inline bool IsInitialized ()const {return m_initialized;}

};


/**
 * Data structure that stores a channel realization
 */
struct Params3gpp : public SimpleRefCount<Params3gpp>
{
  InputParams3gpp m_input;
  complex3DVector_t      m_channel; // channel matrix H[u][s][n], u - number of antennas of receiver, s - number of antennas of transmitter, n - number of clusters
  doubleVector_t      m_delay; //!< cluster delay.
  double2DVector_t    m_angle; //!<cluster angle angle[direction][n], where direction = 0(aoa), 1(zoa), 2(aod), 3(zod) in degree.
  complexVector_t     m_longTerm; //!< long term component per cluster
  complexVector_t m_txW; //!< transmit beamforming vector for which is calculated this long-term matrix
  complexVector_t m_rxW; //!< receive beamforming vector for which is calculated this long-term matrix
  Time m_longTermUpdateTime {0}; //!< the last time at which the long term matrix was updated
  Time m_generatedTime {0}; //!< the last time at which the channel matrix was updated

  double2DVector_t    m_nonSelfBlocking; //!< store the blockages
  /*The following parameters are stored for spatial consistent updating*/
  Vector m_preLocUT {0,0,0}; //!< location of UT when generating the previous channel
  double2DVector_t m_norRvAngles; //!< stores the normal variable for random angles angle[cluster][id] generated for equation (7.6-11)-(7.6-14), where id = 0(aoa),1(zoa),2(aod),3(zod)
  double m_DS; //!< delay spread
  double m_K; //!< K factor
  uint8_t m_numCluster; //!< reduced cluster number;
  double2DVector_t m_clusterPhase;
  double m_losPhase;

};

/**
 * Data structure that stores the parameters of 3GPP TR 38.900, Table 7.5-6, for a certain scenario
 */
struct ParamsTable : public Object
{
  uint8_t m_numOfCluster = 0;
  uint8_t m_raysPerCluster = 0;
  double m_uLgDS = 0;
  double m_sigLgDS = 0;
  double m_uLgASD = 0;
  double m_sigLgASD = 0;
  double m_uLgASA = 0;
  double m_sigLgASA = 0;
  double m_uLgZSA = 0;
  double m_sigLgZSA = 0;
  double m_uLgZSD = 0;
  double m_sigLgZSD = 0;
  double m_offsetZOD = 0;
  double m_cDS = 0;
  double m_cASD = 0;
  double m_cASA = 0;
  double m_cZSA = 0;
  double m_uK = 0;
  double m_sigK = 0;
  double m_rTau = 0;
  double m_shadowingStd = 0;

  double m_sqrtC[7][7];

  ParamsTable ()
  {
  }
  void SetParams (uint8_t numOfCluster, uint8_t raysPerCluster, double uLgDS, double sigLgDS,
                  double uLgASD, double sigLgASD, double uLgASA, double sigLgASA,
                  double uLgZSA, double sigLgZSA, double uLgZSD, double sigLgZSD, double offsetZOD,
                  double cDS, double cASD, double cASA, double cZSA,
                  double uK, double sigK, double rTau, double shadowingStd)
  {
    m_numOfCluster = numOfCluster;
    m_raysPerCluster = raysPerCluster;
    m_uLgDS = uLgDS;
    m_sigLgDS = sigLgDS;
    m_uLgASD = uLgASD;
    m_sigLgASD = sigLgASD;
    m_uLgASA = uLgASA;
    m_sigLgASA = sigLgASA;
    m_uLgZSA = uLgZSA;
    m_sigLgZSA = sigLgZSA;
    m_uLgZSD = uLgZSD;
    m_sigLgZSD = sigLgZSD;
    m_offsetZOD = offsetZOD;
    m_cDS = cDS;
    m_cASD = cASD;
    m_cASA = cASA;
    m_cZSA = cZSA;
    m_uK = uK;
    m_sigK = sigK;
    m_rTau = rTau;
    m_shadowingStd = shadowingStd;
  }

};

/**
 * \brief This class implements the fading computation of the 3GPP TR 38.900 channel model and performs the
 * beamforming gain computation. It implements the SpectrumPropagationLossModel interface
 */
class MmWave3gppChannel : public SpectrumPropagationLossModel
{

  friend class NrTest3gppChannelTestCase;

public:

  typedef std::map< key_t, Ptr<Params3gpp> > channelMap_t;

  /**
    * Constructor
    */
  MmWave3gppChannel ();
  /**
     * Destructor
     */
  virtual ~MmWave3gppChannel ();

  // inherited from Object
  static TypeId GetTypeId (void);
  void DoDispose ();

  /**
   * Register the connection between two devices
   * @param a pointer to a NetDevice
   * @param a pointer to a NetDevice
   */
  void ConnectDevices (Ptr<NetDevice> dev1, Ptr<NetDevice> dev2);

  /**
   * Check if the devices are connected
   * @param a The first device's mobility model
   * @param b The second device's mobility model
   * @return boolean value true if the devices are connected and false if they are not
   */
  bool AreConnected (Ptr<const MobilityModel> a , Ptr<const MobilityModel> b) const;

  /**
   * Check if the channel matrix between a and b exists
   * @param a MobilityModel of the first device
   * @param b MobilityModel of the second device
   * @return boolean value true if there is channel matrix, and false if there is no channel matrix
   */
  bool ChannelMatrixExist (Ptr<const MobilityModel> a , Ptr<const MobilityModel> b) const;

  /**
   * Check if the channel matrix needs an update
   * @param a MobilityModel of the first device
   * @param b MobilityModel of the second device
   * @param los whether the link is LOS
   * @return true if the channel matrix needs to be updated, otherwise false
   */
  bool ChannelMatrixNeedsUpdate (Ptr<const MobilityModel> a , Ptr<const MobilityModel> b, bool los) const;

  /**
   * Checks if the device a UE device
   * @param dev1 pointer to the NetDevice object
   * @return true if the device is a UE device, otherwise it returns false
   */
  bool IsUeDevice (Ptr<NetDevice> dev1) const;

  /**
   * Get position of the UE device. It is expected that one of the
   * two device is the UE device, otherwise this function will call
   * NS_ABORT_MSG.
   * @param a mobility model of the first device
   * @param b mobility model of the second deviec
   * @return Position of the
   */
  Vector GetLocUT (Ptr<const MobilityModel> a, Ptr<const MobilityModel> b) const;

  /**
   * Register the connection between the UE and BS device
   * @param ueDevice The UE device
   * @param ueDeviceAntenna The UE antenna array
   * @param bsDevice The BS device
   * @param bsDeviceAntenna The BS antenna array model
   */
  void CreateInitialBeamformingVectors (Ptr<NetDevice> ueDevice,
                                        Ptr<AntennaArrayBasicModel> ueDeviceAntenna,
                                        Ptr<NetDevice> bsDevice,
                                        Ptr<AntennaArrayBasicModel> bsDeviceAntenna);
  /**
   * Sets the center frequency of the channel map of this instance of MmWave3gppChannel
   * @param centerFrequency center frequency of the channel map of this instance of MmWave3gppChannel
   */
  void SetCenterFrequency (double centerFrequency);

  /**
   * Get center frequency of the channel map of this instance of MmWave3gppChannel
   * @return centerFrequency of the channel map of this instance of MmWave3gppChannel
   */
  double GetCenterFrequency () const;

  /**
   * Set the pathloss model associated to this class
   * @param a pointer to the pathloss model, which has to implement the PropagationLossModel interface
   */
  void SetPathlossModel (Ptr<PropagationLossModel> pathloss);


  /**
   * Perform the configured beamforming method, e.g. beam search beamforming or
   * long-term covariation matrix method
   * @param a MobilityModel of the transmitter
   * @param b MobilityModel of the receiver
   */
  void PerformBeamforming(const Ptr<const NetDevice> &a,
                          const Ptr<const NetDevice> &b) const;

private:
  /**
   * Scan all sectors with predefined code book and select the one returns maximum gain.
   * The BF vector is stored in the Params3gpp object passed as parameter
   * @param a mobility model of the transmitter
   * @param b mobility model of the receiver
   */
  void BeamSearchBeamforming (Ptr<const MobilityModel> a,
                              Ptr<const MobilityModel> b) const;

  /**
   * Compute the optimal BF vector with the Power Method (Maximum Ratio Transmission method).
   * The vector is stored in the Params3gpp object passed as parameter
   * @param a mobility model of the transmitter node
   * @param b mobility model of the receiver node
   */
  void LongTermCovMatrixBeamforming (Ptr<const MobilityModel> a,
                                     Ptr<const MobilityModel> b) const;

  /**
   * This function prepares 3gpp parameters that are necessary for the channel
   * realization calculation or update
   * @param a Mobility model of the transmitter device
   * @param b Mobility model of the receiver device
   * @return Input3gppParams structure that contains input parameter for 3gpp channel calculations
   *
   */
  InputParams3gpp GetInput3gppParameters (Ptr<const MobilityModel> a,
                                          Ptr<const MobilityModel> b) const;

  /**
   * Function checks if the link is between UE and BS, if it is then it returns
   * true. Otherwise if the link is between two UEs or two BS it returns false.
   * @param a Mobility model of the first device
   * @param b Mobility model of the second device
   * @return whether the link is a valid link, and the valid link is considered
   * when it is between UE and BS
   */
  bool IsValidLink (Ptr<const MobilityModel> a,
                    Ptr<const MobilityModel> b) const;

  void DoUpdateLongTerm (Ptr<const MobilityModel> a,
                         Ptr<const MobilityModel> b) const;

  Ptr<Params3gpp> DoGetChannel (Ptr<const MobilityModel> a,
                                   Ptr<const MobilityModel> b) const;

  /**
   * Returns a reference to the channel map that corresponding to central carrier
   * frequency of this instance of MmWave3gppChannel
   * @return reference to the channel map
   */
  std::map< key_t, Ptr<Params3gpp> >& GetChannelMap() const;

  /**
   * Returns the channel condition for the given transmitter and receiver
   * @param a Mobility model of the transmitter
   * @param b Mobility model of the receiver
   * @return the channel condition based on the used propagation loss model
   */
  char DoGetChannelCondition (Ptr<const MobilityModel> a,
                              Ptr<const MobilityModel> b) const;

  /**
   * Inherited from SpectrumPropagationLossModel, it returns the PSD at the receiver
   * @params the transmitted PSD
   * @params the mobility model of the transmitter
   * @params the mobility model of the receiver
   * @returns the received PSD
   */
  Ptr<SpectrumValue> DoCalcRxPowerSpectralDensity (Ptr<const SpectrumValue> txPsd,
                                                   Ptr<const MobilityModel> a,
                                                   Ptr<const MobilityModel> b) const;

  /**
   * Get a new realization of the channel
   * @param table3gpp the ParamsTable for the specific scenario
   * @param a mobility model of the transmitter node
   * @param b mobility model of the receiver node
   * @param inputParams input 3gpp params
   * @return a new realization of the channel
   */
  Ptr<Params3gpp> GetNewChannel (Ptr<ParamsTable> table3gpp,
                                 Ptr<const MobilityModel> a,
                                 Ptr<const MobilityModel> b,
                                 InputParams3gpp inputParams) const;

  /**
   * Update the channel realization with procedure A of TR 38.900 Sec 7.6.3.2
   * for the spatial consistency
   * @param params3gpp the previous channel realization in a Params3gpp object
   * @param table3gpp the ParamsTable for the specific scenario
   * @param a mobility model of the transmitter node
   * @param b mobility model of the receiver node
   * @return
   */
  Ptr<Params3gpp> UpdateChannel (Ptr<Params3gpp> params3gpp,
                                 Ptr<ParamsTable> table3gpp,
                                 Ptr<const MobilityModel> a,
                                 Ptr<const MobilityModel> b) const;

  /**
   * Get the antenna array of the device, this function is technology specific
   * and thus the implementation will be moved to the corresponding child class
   * or will be made generic but requiring that the provided device implements
   * some additional interface
   * @param device
   * @return
   */
  Ptr<AntennaArrayBasicModel> GetAntennaArray (Ptr<NetDevice> device) const;


  /**
   * Checks if a is transmitter and b receiver or is a "reverse link" meaning oposite,
   * i.e. that b is transmitter and a receiver.
   * @param a mobility model of the first node
   * @param b mobility model of the second node
   * @return boolean that says if the a and b are tx and rx or opposite.
   */
  bool IsReverseLink (Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const;

  /**
   * Creates power spectral density for the given power and
   * channel specific parameters (central frequency and the number of RB)
   * @param powerTx power transmitted
   * @param txSm SpectrumModel of the transmitted
   * @return SpectrumValue representing PSD
   */
  Ptr<const SpectrumValue> GetFakeTxPowerSpectralDensity (double powerTx, Ptr<const SpectrumModel> txSm) const;

  /**
   * Compute and store the long term fading parameters in order to decrease the computational load
   * @param txW beamforming vector of the transmitter antenna
   * @param rxW beamforming vector of the receiver antenna
   * @param delayClusters
   * @param Husn the channel realization
   * @return long term fading parameters
   */
  complexVector_t CalLongTerm (complexVector_t txW, complexVector_t rxW, doubleVector_t delayClusters, complex3DVector_t& Husn) const;

  /**
   * Compute the BF gain, apply frequency selectivity by phase-shifting with the cluster delays
   * and scale the txPsd to get the rxPsd
   * @param txPsd tx PSD
   * @param channel the channel realization
   * @param longTerm the long term fading
   * @param txW antenna weights of the transmitter antenna
   * @param rxW antenna weights of the receiver antenna
   * @param delay delay clusters
   * @param angle angles
   * @param speed relative speed
   * @return the rx PSD
   */
  Ptr<SpectrumValue> CalBeamformingGain (Ptr<const SpectrumValue> txPsd,
                                         complex3DVector_t channel,
                                         complexVector_t longTerm,
                                         complexVector_t txW,
                                         complexVector_t rxW,
                                         doubleVector_t delay,
                                         double2DVector_t angle,
                                         Vector speed) const;

  /**
   * Returns the ParamsTable with the parameters of TR 38.900 Table 7.5-6
   * that apply to a certain scenario
   * @params the los condition
   * @params the o2i condition
   * @params the BS height (i.e., eNB)
   * @params the UT height (i.e., UE)
   * @params the 2D distance
   * @return the ParamsTable structure
   */
  Ptr<ParamsTable> Get3gppTable (bool los,
                                 bool o2i,
                                 double hBS,
                                 double hUT,
                                 double distance2D) const;

  /**
   * Delete the m_channel entry associated to the Params3gpp object of pair (a,b)
   * but keep the other parameters, so that the spatial consistency procedure can be used
   * @params the mobility model of the transmitter
   * @params the mobility model of the receiver
   */
  void DeleteChannel (Ptr<const MobilityModel> a,
                      Ptr<const MobilityModel> b) const;

  /**
   * Returns the attenuation of each cluster in dB after applying blockage model
   * @params the channel realization as a Params3gpp object
   * @params cluster azimuth angle of arrival
   * @params cluster zenith angle of arrival
   * @params locUT UE location
   */
  doubleVector_t CalAttenuationOfBlockage (Ptr<Params3gpp> params,
                                           doubleVector_t clusterAOA,
                                           doubleVector_t clusterZOA,
                                           Vector locUT) const;


  mutable MmWave3gppChannel::channelMap_t m_channelMap;
  mutable std::map< key_t, int > m_connectedPair;
  mutable std::set <Ptr<NetDevice> > m_ueDevices;
  
  Ptr<UniformRandomVariable> m_uniformRv;
  Ptr<UniformRandomVariable> m_uniformRvBlockage;

  Ptr<NormalRandomVariable> m_normalRv; //there is a bug in the NormalRandomVariable::GetValue() function.
  Ptr<NormalRandomVariable> m_normalRvBlockage;
  Ptr<PropagationLossModel> m_3gppPathloss;
  Ptr<ParamsTable> m_table3gpp;
  Time m_updatePeriod;
  bool m_blockage;
  uint16_t m_numNonSelfBloking; //number of non-self-blocking regions.
  bool m_portraitMode; //true (portrait mode); false (landscape mode).
  std::string m_scenario;
  double m_blockerSpeed;
  double m_beamSearchAngleStep; //!< The size of the angle to be used in beam search method
  double m_ueSpeed; //!< The speed of the UE to be used in the calculation instead of the real relative speed
  double m_centerFrequency; //!< The center frequency of this 3gpp channel, in this implementation all the devices using the same channel are on the same central frequency
  bool m_cellScan; //!< If true beam search beamforming is enabled, if false the long term cov. matrix is used
  double m_bandwidth; //!< The total bandwidth for this channel
  std::map <Ptr<NetDevice>, Ptr<AntennaArrayBasicModel> > m_deviceToAntennaArray; //!< The map that holds the mapping between the netDevice and its AntennaArray instance for this channel
};



}  //namespace ns3


#endif /* MMWAVE_3GPP_CHANNEL_H_ */
