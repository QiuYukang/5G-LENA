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
#ifndef V2X_KPI
#define V2X_KPI

#include <inttypes.h>
#include <vector>
#include <sqlite3.h>

namespace ns3 {

/**
 * \brief Class which reads the specific tables of a given DB to
 *        compute V2X KPIs. It could compute following KPIs:
 *        - Average PIR
 *        - Throughput
 */
class V2xKpi
{
public:
  /**
   * \brief V2xKpi constructor
   */
  V2xKpi ();

  /**
   * \brief V2xKpi destructor
   */
  ~V2xKpi ();

  /**
   * \brief Set the path to the DB to read the tables from.
   * \param dbPath
   */
  void SetDbPath (std::string dbPath);
  /**
   * \brief Set the duration of the transmitting application.
   *
   * Note: Without setting this throughput KPI can not be computed.
   *
   * \param duration The duration of the transmitting application in seconds.
   */
  void SetTxAppDuration (double duration);
  /**
   * \brief Write the KPIs in their respective tables in the DB.
   */
  void WriteKpis ();

private:
  struct PktTxRxData
  {
    PktTxRxData (double time, std::string txRx, uint32_t nodeId, uint64_t imsi, uint32_t pktSize, std::string ipAddrs)
      : time (time), txRx (txRx), nodeId (nodeId), imsi (imsi), pktSize (pktSize), ipAddrs (ipAddrs)
    {}

    double time; //!< time
    std::string txRx {""}; //!< tx/rx indicator
    uint32_t nodeId {std::numeric_limits <uint32_t>::max ()}; //!< node id of tx or rx node
    uint64_t imsi {std::numeric_limits <uint64_t>::max ()}; //!< IMSI of the tx and rx node
    uint32_t pktSize; //!< packet size
    std::string ipAddrs; //!< The ip address of the node.
  };
  /**
   * \brief Delete the table if it already exists with same seed and run number
   * \param seed The seed index
   * \param run The run index
   * \param table The name of the table
   */
  void DeleteWhere (uint32_t seed, uint32_t run, const std::string &table);
  /**
   * \brief Save the RX packet data from pktTxRx table.
   *
   * This method reads and save the entries of pktTxRx table by filtering
   * txRx column using rx key.
   */
  void SavePktRxData ();
  /**
   * \brief Save the TX packet data from pktTxRx table.
   *
   * This method reads and save the entries of pktTxRx table by filtering
   * txRx column using tx key.
   */
  void SavePktTxData ();
  /**
   * \brief Save average PIR
   *
   * This method compute the average PIR of each receiver node with
   * respect to each transmitter it has received packets from. This
   * average PIR is then written in to a new table "avrgPirSec" of
   * the DB.
   */
  void SaveAvrgPir ();
  /**
   * \brief Compute the average PIR
   * \param data The data to be used to compute the average PIR
   * \return The average PIR
   */
  double ComputeAvrgPir (std::vector <PktTxRxData> data);
  /**
   * \brief Save throughput
   *
   * This method compute the throughput of each receiver node with
   * respect to each transmitter it has received packets from. This
   * throughput is then written in to a new table "thputKbps" of
   * the DB.
   */
  void SaveThput ();
  /**
   * \brief Compute the throughput
   *
   * This method would fail if TX application duration is not set.
   *
   * \param data The data to be used to compute the throughput
   * \return The throughput
   *
   * \see SetTxAppDuration
   */
  double ComputeThput (std::vector <PktTxRxData> data);
  /**
   * \brief Get the total transmitted packets by a transmitter
   * \param srcIpAddrs The IP of the transmitter
   * \return The total transmitted packets
   */
  uint64_t GetTotalTxPkts (std::string srcIpAddrs);

  /*
   * Key 1 = Rx node id
   * Key 2 = IP address of the transmitter this RX node received pkts from
   * value of second map = data to compute KPIs or other stats, e.g., PIR, thput.
   */
  std::map <uint32_t, std::map <std::string, std::vector <PktTxRxData> > > m_rxDataMap; //!< map to store the rx data of each node w.r.t its transmitters
  /*
   * Key = Tx node id
   * value = data to compute KPI or other stats, e.g., total txed packets by a tx node
   */
  std::map <uint32_t, std::vector <PktTxRxData> > m_txDataMap; //!< map to store the tx data per transmitting node
  sqlite3* m_db {nullptr}; //!< DB pointer
  std::string m_dbPath {""}; //!< path to the DB to read
  double m_txAppDuration {0.0}; //!< The TX application duration to compute the throughput
};

} // namespace ns3

#endif // V2X_KPI
