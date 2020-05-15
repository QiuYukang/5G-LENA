/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nr-module.h"
#include "ns3/sqlite-output.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcSimpleInterferenceExample");

/**
 * \brief Represents a scenario with gnb (or base stations) and UEs (or users)
 */
class Scenario
{
public:
  /**
   * \brief ~Scenario
   */
  virtual ~Scenario ();
  /**
   * \brief Get the list of gnb/base station nodes
   * \return A NodeContainer with all the Gnb (or base stations)
   */
  const NodeContainer & GetGnbs () const;
  /**
   * \brief Get the list of user nodes
   * \return A NodeContainer with all the users
   */
  const NodeContainer & GetUes () const;

protected:
  NodeContainer m_gNb; //!< GNB (or base stations)
  NodeContainer m_ue;  //!< users
};

Scenario::~Scenario ()
{
}

const NodeContainer &
Scenario::GetGnbs () const
{
  return m_gNb;
}

const NodeContainer &
Scenario::GetUes () const
{
  return m_ue;
}

/**
 * \brief The most easy interference scenario: with a very strong interference
 *
 * Please note that this scenario considers one UE per each GNB.
 *
 * \verbatim

                  d = ueY
          |----------------------|
    GNB_i                          UE_i
 \endverbatim
 *
 * The distance between each GNB is for the x value \f$ GNB_{i+1} = GNB_{i} \f$ ,
 * for z \f$ GNB_{i+1} = GNB_{i} \f$ and for y \f$ GNB_{i+1} = GNB_{i} + 0.5 \f$
 */
class SimpleInterferenceScenario : public Scenario
{
public:
  /**
   * \brief SimpleInterferenceScenario constructor: initialize the node position
   * \param gnbNum Number of gnb (or base stations)
   * \param gnbReferencePos reference position for the first GNB (other position
   * will be derived from this information)
   * \param ueY Distance between GNB and UE in meters
   */
  SimpleInterferenceScenario (uint32_t gnbNum, const Vector& gnbReferencePos, double ueY);
  /**
    * \brief destructor
    */
  ~SimpleInterferenceScenario ();
};

SimpleInterferenceScenario::SimpleInterferenceScenario (uint32_t gnbNum,
                                                        const Vector& gnbReferencePos,
                                                        double ueX)
{
  // create base stations and mobile terminals
  static MobilityHelper mobility;

  m_gNb.Create (gnbNum);
  m_ue.Create (gnbNum);

  for (uint32_t i = 0; i < gnbNum; ++i)
    {
      std::stringstream ssGnb, ssUe;
      ssGnb << "gNb" << m_gNb.Get(i)->GetId();
      ssUe << "UE" << m_ue.Get(i)->GetId();

      Names::Add (ssGnb.str(), m_gNb.Get(i));
      Names::Add(ssUe.str(), m_ue.Get(i));

      std::cout << " GNB ID " << m_gNb.Get(i)->GetId() << std::endl;
      std::cout << " UE ID " << m_ue.Get(i)->GetId() << std::endl;
    }

  Ptr<ListPositionAllocator> gnbPos = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> uePos = CreateObject<ListPositionAllocator> ();

  // GNB positions:
  {
    double delta = 0.0;
    for (uint32_t i = 0; i < gnbNum; ++i)
      {
        Vector pos (gnbReferencePos);
        pos.y = pos.y + delta;
        delta += 0.5;
        std::cout << "gnb " << i << " pos " << pos << std::endl;
        gnbPos->Add (pos);
      }
  }

  // UE positions:
  {
    double delta = 0.0;
    for (uint32_t i = 0; i < gnbNum; ++i)
      {
        Vector pos (gnbReferencePos.x + ueX, gnbReferencePos.y + delta, 1.5);
        delta += 0.5;
        std::cout << "ue " << i << " pos " << pos << std::endl;
        uePos->Add(pos);
      }
  }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (gnbPos);
  mobility.Install (m_gNb);

  mobility.SetPositionAllocator (uePos);
  mobility.Install (m_ue);
}

SimpleInterferenceScenario::~SimpleInterferenceScenario ()
{

}

/**
 * \brief No interference (or very little, depends) scenario (limited to 2 GNB and UE)
 *
 * Please note that this scenario considers one UE per each GNB.
 *
 * \verbatim

                  d = ueY
          |----------------------|
    GNB_i                          UE_i
    GNB_i+1
           \
            \
             \
              \
               \
               UE_i+1   (position fixed)
 \endverbatim
 *
 */
class NoInterferenceScenario : public Scenario
{
public:
  /**
   * \brief SimpleInterferenceScenario constructor: initialize the node position
   * \param
   * \param gnbReferencePos reference position for the first GNB (other position
   * will be derived from this information)
   * \param ueY Distance between GNB and UE in meters
   */
  NoInterferenceScenario (const Vector& gnbReferencePos, double ueY);
  /**
    * \brief destructor
    */
  ~NoInterferenceScenario ();
};

NoInterferenceScenario::NoInterferenceScenario (const Vector& gnbReferencePos,
                                                double ueX)
{
  // create base stations and mobile terminals
  static MobilityHelper mobility;

  uint32_t gnbNum = 2;

  m_gNb.Create (gnbNum);
  m_ue.Create (gnbNum);

  for (uint32_t i = 0; i < gnbNum; ++i)
    {
      std::stringstream ssGnb, ssUe;
      ssGnb << "gNb" << m_gNb.Get(i)->GetId();
      ssUe << "UE" << m_ue.Get(i)->GetId();

      Names::Add (ssGnb.str(), m_gNb.Get(i));
      Names::Add(ssUe.str(), m_ue.Get(i));

      std::cout << "GNB ID " << m_gNb.Get(i)->GetId() << std::endl;
      std::cout << "UE ID " << m_ue.Get(i)->GetId() << std::endl;
    }

  Ptr<ListPositionAllocator> gnbPos = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> uePos = CreateObject<ListPositionAllocator> ();

  // GNB positions:
  {
    double delta = 0.0;
    for (uint32_t i = 0; i < gnbNum; ++i)
      {
        Vector pos (gnbReferencePos);
        pos.y = pos.y + delta;
        delta += 0.5;
        std::cout << "gnb " << i << " pos " << pos << std::endl;
        gnbPos->Add (pos);
      }
  }

  // UE positions:
  {
    Vector ue1Pos = Vector (gnbReferencePos.x + ueX, gnbReferencePos.y, 1.5);
    Vector ue2Pos = Vector (gnbReferencePos.x + sqrt(0.5) * ueX,
                            gnbReferencePos.y + sqrt(0.5) * ueX, 1.5);

    uePos->Add (ue1Pos);
    std::cout << "ue0 pos " << ue1Pos << std::endl;
    uePos->Add (ue2Pos);
    std::cout << "ue1 pos " << ue2Pos << std::endl;
  }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (gnbPos);
  mobility.Install (m_gNb);

  mobility.SetPositionAllocator (uePos);
  mobility.Install (m_ue);
}


NoInterferenceScenario::~NoInterferenceScenario ()
{

}


/**
 * \brief Raying interference scenario (limited to 2 GNB and UE)
 *
 * Please note that this scenario considers one UE per each GNB.
 */
class RayingInterferenceScenario : public Scenario
{
public:
  /**
   * \brief RayingInterferenceScenario constructor: initialize the node position
   * \param
   * \param gnbReferencePos reference position for the first GNB (other position
   * will be derived from this information)
   * \param ueY Distance between GNB and UE in meters
   */
  RayingInterferenceScenario (const Vector& gnbReferencePos, double ueY);
  /**
    * \brief destructor
    */
  ~RayingInterferenceScenario ();
};

RayingInterferenceScenario::RayingInterferenceScenario (const Vector& gnbReferencePos,
                                                        double ueX)
{
  // create base stations and mobile terminals
  static MobilityHelper mobility;

  uint32_t gnbNum = 2;

  m_gNb.Create (gnbNum);
  m_ue.Create (gnbNum);

  for (uint32_t i = 0; i < gnbNum; ++i)
    {
      std::stringstream ssGnb, ssUe;
      ssGnb << "gNb" << m_gNb.Get(i)->GetId();
      ssUe << "UE" << m_ue.Get(i)->GetId();

      Names::Add (ssGnb.str(), m_gNb.Get(i));
      Names::Add(ssUe.str(), m_ue.Get(i));

      std::cout << "GNB ID " << m_gNb.Get(i)->GetId() << std::endl;
      std::cout << "UE ID " << m_ue.Get(i)->GetId() << std::endl;
    }

  Ptr<ListPositionAllocator> gnbPos = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> uePos = CreateObject<ListPositionAllocator> ();

  // GNB positions:
  {
    for (int32_t i = 0; i < static_cast<int32_t> (gnbNum); ++i)
      {
        Vector pos (gnbReferencePos);
        pos.y = (i * 0.5) + pos.y;
        pos.x = pos.x + (i * (ueX - 50));
        std::cout << "gnb " << i << " pos " << pos << std::endl;
        gnbPos->Add (pos);
      }
  }

  // UE positions:
  {
    Vector ue1Pos = Vector (gnbReferencePos.x + ueX, gnbReferencePos.y, 1.5);
    Vector ue2Pos = Vector (gnbReferencePos.x + ueX, gnbReferencePos.y + 0.5, 1.5);

    uePos->Add (ue1Pos);
    std::cout << "ue0 pos " << ue1Pos << std::endl;
    uePos->Add (ue2Pos);
    std::cout << "ue1 pos " << ue2Pos << std::endl;
  }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (gnbPos);
  mobility.Install (m_gNb);

  mobility.SetPositionAllocator (uePos);
  mobility.Install (m_ue);
}


RayingInterferenceScenario::~RayingInterferenceScenario ()
{

}

/**
 * \brief The OutputManager interface for retrieve the data and storing it
 */
class OutputManager
{
public:
  /**
   * \brief ~OutputManager
   */
  virtual ~OutputManager ();

  /**
   * \brief Store a SINR value
   * \param networkId Network id (e.g., cellId)
   * \param nodeId Node id
   * \param sinr Value of SINR in dB
   */
  virtual void SinrStore (uint32_t networkId, uint32_t nodeId, double sinr) = 0;
  /**
   * \brief Store a SNR value
   * \param networkId Network id (e.g., cellId)
   * \param nodeId Node id
   * \param sinr Value of SNR
   */
  virtual void SnrStore (uint32_t networkId, uint32_t nodeId, double snr) = 0;
};

OutputManager::~OutputManager ()
{

}

/**
 * \brief Store the values in files (slow and old, just for testing)
 */
class FileOutputManager : public OutputManager
{
public:
  FileOutputManager (const std::string &prefix);
  virtual ~FileOutputManager () override;

  virtual void SinrStore (uint32_t networkId, uint32_t nodeId, double sinr) override;
  virtual void SnrStore (uint32_t networkId, uint32_t nodeId, double snr) override;

private:
  std::string m_prefix;          //!< File prefix
  std::ofstream m_outSinrFile;   //!< SINR file
  std::ofstream m_outSnrFile;    //!< SNR file
  std::ofstream m_outRssiFile;   //!< RSSI file
};

FileOutputManager::FileOutputManager (const std::string &prefix)
{
  m_outSinrFile.open ((prefix + "-sinr.txt").c_str (), std::ios::trunc);
}

FileOutputManager::~FileOutputManager ()
{
  m_outSinrFile.close ();
}

void
FileOutputManager::SinrStore (uint32_t networkId, uint32_t nodeId, double sinr)
{
  m_outSinrFile << networkId << " " << nodeId
                << " " << 10 * log (sinr) / log (10) << std::endl;
}

void
FileOutputManager::SnrStore (uint32_t networkId, uint32_t nodeId, double snr)
{
  m_outSinrFile << networkId << " " << nodeId
                << " " << 10*log10(snr) << std::endl;
}

/**
 * \brief Store the values in files (slow and old, just for testing)
 */
class SqliteOutputManager : public OutputManager
{
public:
  SqliteOutputManager (const std::string &dbName, const std::string &dbLockName,
                       double ueX, uint32_t seed, uint32_t run);
  virtual ~SqliteOutputManager () override;

  virtual void SinrStore (uint32_t networkId, uint32_t nodeId, double sinr) override;
  virtual void SnrStore (uint32_t networkId, uint32_t nodeId, double snr) override;
private:
  void DeleteWhere (uint32_t seed, uint32_t run, const std::string &table);
private:
  SQLiteOutput m_dbOutput;
  std::string m_dbName {""};
  std::string m_sinrTableName {""};
  std::string m_snrTableName {""};
  uint32_t m_seed {0};
  uint32_t m_run  {0};
};

SqliteOutputManager::SqliteOutputManager (const std::string &dbName, const std::string &dbLockName,
                                          double ueX, uint32_t seed, uint32_t run)
  : m_dbOutput (dbName, dbLockName),
    m_dbName (dbName),
    m_seed (seed),
    m_run (run)
{
  std::stringstream ss;
  ss << ueX;

  m_sinrTableName = "sinr_results_" + ss.str();
  m_snrTableName = "snr_results_" + ss.str();

  m_dbOutput.WaitExec ("CREATE TABLE IF NOT EXISTS \"" + m_sinrTableName + "\" "
                       "(NETID                   INT    NOT NULL, "
                       "UID                     INT    NOT NULL, "
                       "SINR                    DOUBLE NOT NULL, "
                       "SEED                    INT    NOT NULL, "
                       "RUN                     INT    NOT NULL"
                       ");");

  m_dbOutput.WaitExec ("CREATE TABLE IF NOT EXISTS \"" + m_snrTableName + "\" "
                       "(NETID                   INT    NOT NULL, "
                       "UID                     INT    NOT NULL, "
                       "SNR                    DOUBLE NOT NULL, "
                       "SEED                    INT    NOT NULL, "
                       "RUN                     INT    NOT NULL"
                       ");");


  DeleteWhere (seed, run, m_sinrTableName);
  DeleteWhere (seed, run, m_snrTableName);
}

SqliteOutputManager::~SqliteOutputManager ()
{
}

void
SqliteOutputManager::DeleteWhere (uint32_t seed, uint32_t run, const std::string &table)
{
  bool ret;
  sqlite3_stmt *stmt;
  ret = m_dbOutput.WaitPrepare (&stmt, "DELETE FROM \"" + table + "\" WHERE SEED = ? AND RUN = ?;");
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 1, seed);
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 2, run);

  ret = m_dbOutput.WaitExec (stmt);
  NS_ABORT_IF (ret == false);
}

void
SqliteOutputManager::SinrStore (uint32_t networkId, uint32_t nodeId, double sinr)
{
  bool ret;
  sqlite3_stmt *stmt;
  ret = m_dbOutput.WaitPrepare (&stmt, "INSERT INTO " + m_sinrTableName + " VALUES (?,?,?,?,?);");
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 1, networkId);
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 2, nodeId);
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 3, 10 * log (sinr) / log (10));
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 4, m_seed);
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 5, m_run);
  NS_ABORT_IF (ret == false);

  ret = m_dbOutput.WaitExec (stmt);
  NS_ABORT_IF (ret == false);
}

void SqliteOutputManager::SnrStore(uint32_t networkId, uint32_t nodeId, double snr)
{
  bool ret;
  sqlite3_stmt *stmt;
  ret = m_dbOutput.WaitPrepare (&stmt, "INSERT INTO " + m_snrTableName + " VALUES (?,?,?,?,?);");
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 1, networkId);
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 2, nodeId);
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 3, 10 * log (snr) / log (10));
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 4, m_seed);
  NS_ABORT_IF (ret == false);
  ret = m_dbOutput.Bind (stmt, 5, m_run);
  NS_ABORT_IF (ret == false);

  ret = m_dbOutput.WaitExec (stmt);
  NS_ABORT_IF (ret == false);
}

/**
 * \brief Interface for constructing everything for the L2 devices (NR, wifi, whatever..)
 */
class L2Setup
{
public:
  /**
   * \brief L2Setup constructor
   * \param manager Output manager
   */
  L2Setup (Scenario *scenario, OutputManager *manager);
  /**
   * \brief ~L2Setup
   */
  virtual ~L2Setup ();
  /**
   * \brief Get a container of user devices
   * \return A user device container
   */
  virtual const NetDeviceContainer & GetUeDev () const;
  /**
   * \brief Get a container of gnb devices
   * \return A gnb device container
   */
  virtual const NetDeviceContainer & GetGnbDev () const;

protected:
  /**
   * \brief Retrieve GNB node list from the scenario
   * \return the GNB node container
   */
  const NodeContainer & GetGnbNodes () const;
  /**
   * \brief Retrieve UE node list from the scenario
   * \return the UE node container
   */
  const NodeContainer & GetUeNodes () const;

  virtual void Init () = 0;

protected:
  NetDeviceContainer m_ueDev; //!< user net devices
  NetDeviceContainer m_gnbDev;//!< gnb net devices
  OutputManager *m_manager; //!< Output manager

private:
  Scenario *m_scenario;
};

L2Setup::L2Setup (Scenario *scenario, OutputManager *manager)
  : m_manager (manager),
    m_scenario (scenario)
{
}

L2Setup::~L2Setup ()
{
}

const NetDeviceContainer &
L2Setup::GetUeDev() const
{
  return m_ueDev;
}

const NetDeviceContainer &
L2Setup::GetGnbDev() const
{
  return m_gnbDev;
}

const NodeContainer &
L2Setup::GetGnbNodes () const
{
  return m_scenario->GetGnbs ();
}

const NodeContainer &
L2Setup::GetUeNodes () const
{
  return m_scenario->GetUes ();
}

/**
 * \brief Setup NR as L2 technology with one bandwith part
 */
class NrSingleBwpSetup : public L2Setup
{
public:
  NrSingleBwpSetup (Scenario *scenario, OutputManager *manager, double freq,
                    double bw, uint32_t num, double txPower,
                    const std::unordered_map<uint32_t, uint32_t> &GnbUeMap);
  virtual ~NrSingleBwpSetup () override;

  virtual Ptr<NrPointToPointEpcHelper> GetEpcHelper () const { return m_epcHelper; }
  virtual Ptr<NrHelper> GetHelper () const { return m_helper; }

  virtual void Init () override;

private:
  void UeReception (RxPacketTraceParams params);

private:
  Ptr<NrHelper> m_helper;
  Ptr<NrPointToPointEpcHelper> m_epcHelper;
  std::unordered_map<uint32_t, uint32_t> m_ueGnbMap;
  uint32_t m_ueNum {0};
};

NrSingleBwpSetup::NrSingleBwpSetup (Scenario *scenario, OutputManager *manager,
                                    double freq, double bw, uint32_t num, double txPower,
                                    const std::unordered_map<uint32_t, uint32_t> &ueGnbMap)
  : L2Setup (scenario, manager),
    m_ueGnbMap (ueGnbMap)
{
  // setup the NR simulation
  m_helper = CreateObject<NrHelper> ();
  m_helper->SetAttribute ("PathlossModel", StringValue ("ns3::Nr3gppPropagationLossModel"));
  m_helper->SetAttribute ("ChannelModel", StringValue ("ns3::Nr3gppChannel"));

  Ptr<NrPhyMacCommon> phyMacCommonBwp1 = CreateObject<NrPhyMacCommon>();
  phyMacCommonBwp1->SetBandwidth (bw);
  phyMacCommonBwp1->SetNumerology(num);
  phyMacCommonBwp1->SetAttribute ("MacSchedulerType", TypeIdValue (NrMacSchedulerTdmaRR::GetTypeId ()));
  phyMacCommonBwp1->SetCcId(0);

  BandwidthPartRepresentation repr1 (0, phyMacCommonBwp1, nullptr, nullptr, nullptr);
  m_helper->AddBandwidthPart(0, repr1);

  m_epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  m_helper->SetEpcHelper (m_epcHelper);
  m_helper->Initialize();

  // install nr net devices
  m_gnbDev = m_helper->InstallEnbDevice (GetGnbNodes());
  m_ueDev = m_helper->InstallUeDevice (GetUeNodes());

  double x = pow (10, txPower/10);

  double totalBandwidth = bw;

  for (uint32_t j = 0; j < m_gnbDev.GetN(); ++j)
    {
      ObjectMapValue objectMapValue;
      Ptr<NrGnbNetDevice> netDevice = DynamicCast<NrGnbNetDevice>(m_gnbDev.Get(j));
      netDevice->GetAttribute("BandwidthPartMap", objectMapValue);
      for (uint32_t i = 0; i < objectMapValue.GetN(); i++)
        {
          Ptr<BandwidthPartGnb> bandwidthPart = DynamicCast<BandwidthPartGnb>(objectMapValue.Get(i));
          if (i==0)
            {
              bandwidthPart->GetPhy()->SetTxPower(10*log10((bw/totalBandwidth)*x));
            }
          else
            {
              NS_FATAL_ERROR ("\n Please extend power assignment for additional bandwidht parts...");
            }
        }
    }

  m_ueNum = m_ueDev.GetN ();
  for (uint32_t i = 0 ; i < m_ueDev.GetN(); ++i)
    {
      Ptr<NrSpectrumPhy > ue1SpectrumPhy = DynamicCast<NrUeNetDevice>
      (m_ueDev.Get(i))->GetPhy(0)->GetSpectrumPhy();
      ue1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe",
                                                 MakeCallback (&NrSingleBwpSetup::UeReception, this));
      Ptr<nrInterference> ue1SpectrumPhyInterference = ue1SpectrumPhy->GetNrInterference();
      NS_ABORT_IF(!ue1SpectrumPhyInterference);
      //ue1SpectrumPhyInterference->TraceConnectWithoutContext("SnrPerProcessedChunk", MakeBoundCallback (&UeSnrPerProcessedChunkTrace, this));
      //ue1SpectrumPhyInterference->TraceConnectWithoutContext("RssiPerProcessedChunk", MakeBoundCallback (&UeRssiPerProcessedChunkTrace, this));
    }

  // enable the traces provided by the nr module
  // m_helper->EnableTraces();
}

void
NrSingleBwpSetup::Init ()
{
  // attach UEs to the closest eNB
  for (const auto & v : m_ueGnbMap)
    {
      std::cout << GetUeDev().GetN() << " " << GetGnbDev().GetN() << std::endl;
      if (v.first < GetUeDev().GetN() && v.second < GetGnbDev().GetN())
        {
          std::cout << " attaching " << v.first << " to " << v.second << std::endl;
          m_helper->AttachToEnb (GetUeDev().Get(v.first), GetGnbDev().Get(v.second));
        }
    }
}

NrSingleBwpSetup::~NrSingleBwpSetup ()
{

}

void
NrSingleBwpSetup::UeReception (RxPacketTraceParams params)
{
  static std::set<uint32_t> ueRecv;
  static uint32_t n = 0;
  // RNTI to NodeId conversion??
  m_manager->SinrStore (params.m_cellId, params.m_rnti, params.m_sinr);

  if (ueRecv.find(params.m_cellId) == ueRecv.end ())
    {
      ueRecv.insert(params.m_cellId);
      ++n;
      if (n == m_ueNum)
        {
          // Exit after the first measurement for each UE
          Simulator::Stop ();
        }

    }
}

static void
ConfigureDefaultValues (bool cellScan = true, double beamSearchAngleStep = 10.0,
                        uint32_t eesmTable = 1,
                        const std::string &errorModel = "ns3::NrEesmErrorModel")
{
  Config::SetDefault ("ns3::Nr3gppPropagationLossModel::ChannelCondition",
                      StringValue("l"));
  Config::SetDefault ("ns3::Nr3gppPropagationLossModel::Scenario",
                      StringValue("InH-OfficeMixed")); // with antenna height of 10 m
  Config::SetDefault ("ns3::Nr3gppPropagationLossModel::Shadowing",
                      BooleanValue(false));

  Config::SetDefault ("ns3::Nr3gppChannel::CellScan",
                      BooleanValue(cellScan));
  Config::SetDefault ("ns3::Nr3gppChannel::UpdatePeriod",
                      TimeValue(MilliSeconds(0)));
  Config::SetDefault ("ns3::Nr3gppChannel::BeamSearchAngleStep",
                      DoubleValue(beamSearchAngleStep));

  Config::SetDefault ("ns3::NrGnbPhy::AntennaNumDim1", UintegerValue (4));
  Config::SetDefault ("ns3::NrGnbPhy::AntennaNumDim2", UintegerValue (8));

  Config::SetDefault ("ns3::NrUePhy::AntennaNumDim1", UintegerValue (2));
  Config::SetDefault ("ns3::NrUePhy::AntennaNumDim2", UintegerValue (4));

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize",
                      UintegerValue(999999999));

  Config::SetDefault("ns3::PointToPointEpcHelper::S1uLinkDelay", TimeValue (MilliSeconds(0)));
  Config::SetDefault("ns3::PointToPointEpcHelper::X2LinkDelay", TimeValue (MilliSeconds(0)));

  Config::SetDefault("ns3::NrMacSchedulerNs3::FixedMcsDl", BooleanValue(false));
  Config::SetDefault("ns3::NrMacSchedulerNs3::FixedMcsUl", BooleanValue(false));
  //Config::SetDefault("ns3::NrMacSchedulerNs3::StartingMcsDl", UintegerValue (mcs));
  //Config::SetDefault("ns3::NrMacSchedulerNs3::StartingMcsUl", UintegerValue (mcs));

  if (eesmTable == 1)
    {
      Config::SetDefault("ns3::NrEesmErrorModel::McsTable", EnumValue (NrEesmErrorModel::McsTable1));
    }
  else if (eesmTable == 2)
    {
      Config::SetDefault("ns3::NrEesmErrorModel::McsTable", EnumValue (NrEesmErrorModel::McsTable2));
    }
  else
    {
      NS_FATAL_ERROR ("Valid tables are 1 or 2, you set " << eesmTable);
    }

  Config::SetDefault("ns3::NrAmc::ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));
  Config::SetDefault("ns3::NrAmc::AmcModel", EnumValue (NrAmc::ShannonModel));
  Config::SetDefault("ns3::NrSpectrumPhy::ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));
}

int
main (int argc, char *argv[])
{
  bool cellScan = false;
  double beamSearchAngleStep = 30.0;
  double totalTxPower = 4;
  uint16_t numerologyBwp1 = 0;
  double frequencyBwp1 = 28e9;
  double bandwidthBwp1 = 100e6;
  double ueX = 300.0;

  double simTime = 2; // seconds
  double udpAppStartTime = 1.0; //seconds
  uint32_t scenarioId = 0;
  uint32_t runId = 0;
  uint32_t seed = 1;

  std::string errorModel = "ns3::NrLteMiErrorModel";
  uint32_t eesmTable = 1;

  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("cellScan",
                "Use beam search method to determine beamforming vector,"
                " the default is long-term covariance matrix method"
                " true to use cell scanning method, false to use the default"
                " power method.",
                cellScan);
  cmd.AddValue ("beamSearchAngleStep",
                "Beam search angle step for beam search method",
                beamSearchAngleStep);
  cmd.AddValue ("totalTxPower",
                "total tx power that will be proportionally assigned to"
                " bandwidth parts depending on each BWP bandwidth ",
                totalTxPower);
  cmd.AddValue("errorModelType",
               "Error model type: ns3::NrEesmErrorModel , ns3::NrLteErrorModel",
               errorModel);
  cmd.AddValue("eesmTable",
               "Table to use when error model is Eesm (1 for McsTable1 or 2 for McsTable2)",
               eesmTable);
  cmd.AddValue("ueX",
               "X position of any UE",
               ueX);
  cmd.AddValue("scenario",
               "Scenario (0 = simple interference, 1 = no interf.",
               scenarioId);
  cmd.AddValue ("seed", "Simulation seed", seed);
  cmd.AddValue ("runId", "Simulation Run ID", runId);

  cmd.Parse (argc, argv);

  RngSeedManager::SetSeed (seed);
  RngSeedManager::SetRun (runId);
  ConfigureDefaultValues (cellScan, beamSearchAngleStep, eesmTable, errorModel);

  Scenario *scenario;
  if (scenarioId == 0)
    {
      scenario = new SimpleInterferenceScenario (1, Vector (0, 0, 10), ueX);
    }
  else if (scenarioId == 1)
    {
      scenario = new SimpleInterferenceScenario (2, Vector (0, 0, 10), ueX);
    }
  else if (scenarioId == 2)
    {
      scenario = new RayingInterferenceScenario (Vector (0,0, 10), ueX);

    }
  else if (scenarioId == 3)
    {
      scenario = new NoInterferenceScenario (Vector (0,0, 10), ueX);
    }
  else
    {
      NS_FATAL_ERROR ("Scenario no recognized");
    }

  std::stringstream ss;
  ss << "cttc-simple-interference-scenario-example-" << scenarioId;
  if (cellScan)
    {
      ss << "-with-cellscan-" << beamSearchAngleStep;
    }
  else
    {
      ss << "-without-cellscan";
    }
  ss << ".db";

  SqliteOutputManager manager (ss.str(), ss.str (), ueX, seed, runId);

  NrSingleBwpSetup setup (scenario, &manager, frequencyBwp1, bandwidthBwp1,
                          numerologyBwp1, 4.0, { {0, 0}, {1, 1}});

  // create the internet and install the IP stack on the UEs
  // get SGW/PGW and create a single RemoteHost
  Ptr<Node> pgw = setup.GetEpcHelper()->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // connect a remoteHost to pgw. Setup routing too
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (scenario->GetUes());
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = setup.GetEpcHelper()->AssignUeIpv4Address (setup.GetUeDev());

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < scenario->GetUes().GetN(); ++j)
    {
      auto ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (scenario->GetUes().Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (setup.GetEpcHelper()->GetUeDefaultGatewayAddress (), 1);
    }

  setup.Init ();

  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps, serverApps;

  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverApps.Add (dlPacketSinkHelper.Install (scenario->GetUes()));

  // configure here UDP traffic
  for (uint32_t j = 0; j < scenario->GetUes().GetN(); ++j)
    {
      UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
      dlClient.SetAttribute ("MaxPackets", UintegerValue(1));
      dlClient.SetAttribute("PacketSize", UintegerValue(500));
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(10)));
      clientApps.Add (dlClient.Install (remoteHost));
    }

  // start UDP server and client apps
  serverApps.Start(Seconds(udpAppStartTime));
  clientApps.Start(Seconds(udpAppStartTime));
  serverApps.Stop(Seconds(simTime));
  clientApps.Stop(Seconds(simTime));

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

