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

#include "cc-bwp-helper.h"
#include <ns3/log.h>
#include <memory>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BandwidthPartRepresentation");

bool
ComponentCarrierInfo::AddBwp (uint32_t id, BandwidthPartInfoPtr &&bwp)
{
  NS_LOG_FUNCTION (this);

  bool ret = true;

  NS_ASSERT (bwp->m_lowerFrequency >= m_lowerFrequency);
  NS_ASSERT (bwp->m_higherFrequency <= m_higherFrequency);
  NS_ASSERT (id >= m_bwp.size ());

  for (uint32_t i = 0; i < id; ++i)
    {
      // Check that we have id from 0 to id-1
      NS_ASSERT (m_bwp.find (i) != m_bwp.end ());
    }

  m_bwp.emplace (id, std::move (bwp));

  uint32_t i = 0;
  while (i < m_bwp.size () - 1)
    {
      auto & bwp = m_bwp.at (i);
      auto & nextBwp = m_bwp.at (i + 1);
      if (bwp->m_higherFrequency <= nextBwp->m_lowerFrequency)
        {
          ret = false;
        }
      ++i;
    }

  return ret;
}


bool
OperationBandInfo::AddCc (uint32_t id, ComponentCarrierInfoPtr &&cc)
{
  NS_LOG_FUNCTION (this);
  bool ret = true;

  NS_ASSERT (cc->m_lowerFrequency >= m_lowerFrequency);
  NS_ASSERT (cc->m_higherFrequency <= m_higherFrequency);
  NS_ASSERT (id >= m_cc.size ());

  for (uint32_t i = 0; i < id; ++i)
    {
      // Check that we have id from 0 to id-1
      NS_ASSERT (m_cc.find (i) != m_cc.end ());
    }

  m_cc.emplace (id, std::move (cc));

  uint32_t i = 0;
  while (i < m_cc.size () - 1)
    {
      auto & cc = m_cc.at (i);
      auto & nextCc = m_cc.at (i + 1);
      if (cc->m_higherFrequency <= nextCc->m_lowerFrequency)
        {
          ret = false;
        }
      ++i;
    }

  return ret;
}

BandwidthPartInfoPtr &
OperationBandInfo::GetBwpAt (uint32_t ccId, uint32_t bwpId) const
{
  return m_cc.at (ccId)->m_bwp.at (bwpId);
}


// cc is a unique_pointer... I'm allowing myself to use a reference, because
// if I would use a pointer (like I was telling you to do) then we end up
// with a pointer to a pointer, and we have to use (*cc)->m_centralFreq...
// a bit useless here.
void
CcBwpCreator::InitializeCc (std::unique_ptr<ComponentCarrierInfo> &cc,
                            double ccBandwidth, double lowerFreq, uint32_t ccId)
{
  cc->m_centralFrequency = lowerFreq + ccId * ccBandwidth + ccBandwidth / 2;
  cc->m_lowerFrequency = lowerFreq + ccId * ccBandwidth;
  cc->m_higherFrequency = lowerFreq + (ccId + 1) * ccBandwidth - 1;
  cc->m_bandwidth = ccBandwidth;
  cc->m_ccId = ccId;
}

std::unique_ptr<ComponentCarrierInfo>
CcBwpCreator::CreateCc (double ccBandwidth, double lowerFreq, uint32_t ccId)
{
  // Create a CC with a single BWP
  std::unique_ptr<ComponentCarrierInfo> cc (new ComponentCarrierInfo ());
  InitializeCc (cc, ccBandwidth, lowerFreq, ccId);

  std::unique_ptr<BandwidthPartInfo> bwp (new BandwidthPartInfo ());
  bwp->m_bwpId = m_bwpCounter++;
  bwp->m_centralFrequency = cc->m_centralFrequency;
  bwp->m_lowerFrequency = cc->m_lowerFrequency;
  bwp->m_higherFrequency = cc->m_higherFrequency;
  bwp->m_bandwidth = cc->m_bandwidth;
  cc->AddBwp (0, std::move (bwp));
  // bwp is not longer a valid pointer
  return cc;
}

OperationBandInfo
CcBwpCreator::CreateOperationBandContiguousCc (const SimpleOperationBandConf &conf)
{
  OperationBandInfo band;
  band.m_bandId = m_bwpCounter++;
  band.m_centralFrequency = conf.m_centralFrequency;
  band.m_bandwidth = conf.m_bandwidth;
  band.m_lowerFrequency = conf.m_centralFrequency - conf.m_bandwidth / 2.0;
  band.m_higherFrequency = conf.m_centralFrequency + conf.m_bandwidth / 2.0;

  uint32_t maxCcBandwidth = 198e6;

  if (conf.m_centralFrequency > 6e9)
    {
      maxCcBandwidth = 396e6;
    }

  double ccBandwidth = std::min (static_cast<double> (maxCcBandwidth),
                                 static_cast<double> (conf.m_bandwidth) / conf.m_numCc);

  for (uint8_t c = 0; c < conf.m_numCc; ++c)
    {
      bool ret = band.AddCc (c, CreateCc (ccBandwidth, band.m_lowerFrequency, c));
      NS_ASSERT (ret);
    }

  NS_ASSERT (band.m_cc.size () == conf.m_numCc);
  return band;
}

OperationBandInfo
CcBwpCreator::CreateOperationBandNonContiguousCc (const std::vector<SimpleOperationBandConf> &configuration)
{
  OperationBandInfo band;
  band.m_bandId = m_bwpCounter++;

  uint32_t id = 0;
  for (const auto & conf : configuration)
    {
      band.AddCc (id++, CreateCc (conf.m_bandwidth, band.m_lowerFrequency, 0));
    }

  return band;
}

void
CcBwpCreator::PlotNrCaBwpConfiguration (const std::vector<OperationBandInfo> &bands,
                                        const std::string &filename)
{

  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  // FIXME: I think I can do this with calling the gnuclass in ns3 by calling
  //        plot.AppendExtra (whatever gnu line sting) (see gnuplot documantation
  //        in ns3

  // Set the range for the x axis.
  double minFreq = 100e9;
  double maxFreq = 0;
  for (const auto & band : bands)
    {
      if (band.m_lowerFrequency < minFreq)
        {
          minFreq = band.m_lowerFrequency;
        }
      if (band.m_higherFrequency > maxFreq)
        {
          maxFreq = band.m_higherFrequency;
        }
    }

  outFile << "set term eps" << std::endl;
  outFile << "set output \"" << filename << ".eps\"" << std::endl;
  outFile << "set grid" << std::endl;

  outFile << "set xrange [";
  outFile << minFreq * 1e-6 - 1;
  outFile << ":";
  outFile << maxFreq * 1e-6 + 1;
  outFile << "]";
  outFile << std::endl;

  outFile << "set yrange [1:100]" << std::endl;
  outFile << "set xlabel \"f [MHz]\"" << std::endl;

  uint16_t index = 1;   //<! Index must be larger than zero for gnuplot
  for (const auto & band : bands)
    {
      std::string label = "n";
      uint16_t bandId = static_cast<uint16_t> (band.m_bandId);
      label += std::to_string (bandId);
      PlotFrequencyBand (outFile, index, band.m_lowerFrequency * 1e-6, band.m_higherFrequency * 1e-6,
                         70, 90, label);
      index++;
      for (uint32_t i = 0; i < band.m_cc.size (); ++i)
        {
          const auto & cc = band.m_cc.at (i);
          uint16_t ccId = static_cast<uint16_t> (cc->m_ccId);
          label = "CC" + std::to_string (ccId);
          PlotFrequencyBand (outFile, index, cc->m_lowerFrequency * 1e-6, cc->m_higherFrequency * 1e-6,
                             40, 60, label);
          index++;
          for (uint32_t j = 0; j < cc->m_bwp.size (); ++i)
            {
              const auto & bwp = cc->m_bwp.at (j);
              uint16_t bwpId = static_cast<uint16_t> (bwp->m_bwpId);
              label = "BWP" + std::to_string (bwpId);
              PlotFrequencyBand (outFile, index, bwp->m_lowerFrequency * 1e-6, bwp->m_higherFrequency * 1e-6,
                                 10, 30, label);
              index++;
            }
        }
    }

  outFile << "unset key" << std::endl;
  outFile << "plot -x" << std::endl;

}

void
CcBwpCreator::PlotLteCaConfiguration (const std::vector<OperationBandInfo> &bands,
                                      const std::string &filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  // FIXME: I think I can do this with calling the gnuclass in ns3 and use
  //        plot.AppendExtra (whatever sting);

  double minFreq = 100e9;
  double maxFreq = 0;
  for (const auto & band : bands)
    {
      if (band.m_lowerFrequency < minFreq)
        {
          minFreq = band.m_lowerFrequency;
        }
      if (band.m_higherFrequency > maxFreq)
        {
          maxFreq = band.m_higherFrequency;
        }
    }

  outFile << "set term eps" << std::endl;
  outFile << "set output \"" << filename << ".eps\"" << std::endl;
  outFile << "set grid" << std::endl;

  outFile << "set xrange [";
  outFile << minFreq * 1e-6 - 1;
  outFile << ":";
  outFile << maxFreq * 1e-6 + 1;
  outFile << "]";
  outFile << std::endl;

  outFile << "set yrange [1:100]" << std::endl;
  outFile << "set xlabel \"f [MHz]\"" << std::endl;

  uint16_t index = 1;   //<! Index must be larger than zero for gnuplot
  for (const auto & band : bands)
    {
      std::string label = "n";
      uint16_t bandId = static_cast<uint16_t> (band.m_bandId);
      label += std::to_string (bandId);
      PlotFrequencyBand (outFile, index, band.m_lowerFrequency * 1e-6, band.m_higherFrequency * 1e-6,
                         70, 90, label);
      index++;
      for (uint32_t i = 0; i < band.m_cc.size (); ++i)
        {
          const auto & cc = band.m_cc.at (i);
          uint16_t ccId = static_cast<uint16_t> (cc->m_ccId);
          label = "CC" + std::to_string (ccId);
          PlotFrequencyBand (outFile, index, cc->m_lowerFrequency * 1e-6, cc->m_higherFrequency * 1e-6,
                             40, 60, label);
          index++;
        }
    }

  outFile << "unset key" << std::endl;
  outFile << "plot -x" << std::endl;

}

void
CcBwpCreator::PlotFrequencyBand (std::ofstream &outFile,
                                 uint16_t index,
                                 double xmin,
                                 double xmax,
                                 double ymin,
                                 double ymax,
                                 const std::string &label)
{
  outFile << "set object " << index << " rect from " << xmin  << "," << ymin <<
    " to "   << xmax  << "," << ymax << " front fs empty " << std::endl;

  outFile << "LABEL" << index << " = \"" << label << "\"" << std::endl;

  outFile << "set label " << index << " at " << xmin << "," <<
    (ymin + ymax) / 2 << " LABEL" << index << std::endl;

}

}
