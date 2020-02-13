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

#include "mmwave-chunk-processor.h"

#include <ns3/log.h>
#include <ns3/spectrum-value.h>
#include <ns3/simulator.h>

NS_LOG_COMPONENT_DEFINE ("mmWaveChunkProcessor");

namespace ns3 {

mmWaveChunkProcessor::mmWaveChunkProcessor ()
{
  NS_LOG_FUNCTION (this);
}

mmWaveChunkProcessor::~mmWaveChunkProcessor ()
{
  NS_LOG_FUNCTION (this);
}

void
mmWaveChunkProcessor::AddCallback (mmWaveChunkProcessorCallback c)
{
  NS_LOG_FUNCTION (this);
  m_mmWaveChunkProcessorCallbacks.push_back (c);
}

void
mmWaveChunkProcessor::Start ()
{
  NS_LOG_FUNCTION (this);
  m_sumValues = 0;
  m_totDuration = MicroSeconds (0);
  NS_LOG_INFO ("Starting a chunk processing at time " << Simulator::Now ());
}


void
mmWaveChunkProcessor::EvaluateChunk (const SpectrumValue& sinr, Time duration)
{
  NS_LOG_FUNCTION (this << sinr << duration);
  if (m_sumValues == 0)
    {
      m_sumValues = Create<SpectrumValue> (sinr.GetSpectrumModel ());
    }
  (*m_sumValues) += sinr * duration.GetSeconds ();
  m_totDuration += duration;

  NS_LOG_INFO ("Evaluating chunk sinr " << sinr << " of duration " <<
               duration.GetNanoSeconds() << " ns, sum sinr " << m_sumValues <<
               " tot duration " << m_totDuration);
}

void
mmWaveChunkProcessor::End ()
{
  NS_LOG_FUNCTION (this);
  if (m_totDuration.GetSeconds () > 0)
    {
      std::vector<mmWaveChunkProcessorCallback>::iterator it;
      for (it = m_mmWaveChunkProcessorCallbacks.begin (); it != m_mmWaveChunkProcessorCallbacks.end (); it++)
        {
          (*it)((*m_sumValues) / m_totDuration.GetSeconds ());
        }
    }
  else
    {
      NS_LOG_WARN ("m_numSinr == 0");
    }
}


} // namespace ns3
