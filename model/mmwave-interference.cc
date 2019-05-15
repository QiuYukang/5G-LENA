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



#include "mmwave-interference.h"
#include <ns3/simulator.h>
#include <ns3/log.h>
#include "mmwave-chunk-processor.h"
#include <stdio.h>
#include <algorithm>



NS_LOG_COMPONENT_DEFINE ("mmWaveInterference");

namespace ns3 {


mmWaveInterference::mmWaveInterference ()
  : m_receiving (false),
  m_lastSignalId (0),
  m_lastSignalIdBeforeReset (0),
  m_firstPower (0.0)
{
  NS_LOG_FUNCTION (this);
}

mmWaveInterference::~mmWaveInterference ()
{
  NS_LOG_FUNCTION (this);
}

void
mmWaveInterference::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_PowerChunkProcessorList.clear ();
  m_sinrChunkProcessorList.clear ();
  m_rxSignal = 0;
  m_allSignals = 0;
  m_noise = 0;
  Object::DoDispose ();
}


TypeId
mmWaveInterference::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::mmWaveInterference")
    .SetParent<Object> ()
    .AddTraceSource ("SnrPerProcessedChunk",
                     "Snr per processed chunk.",
                     MakeTraceSourceAccessor (&mmWaveInterference::m_snrPerProcessedChunk),
                     "ns3::SnrPerProcessedChunk::TracedCallback")
    .AddTraceSource ("RssiPerProcessedChunk",
                     "Rssi per processed chunk.",
                     MakeTraceSourceAccessor (&mmWaveInterference::m_rssiPerProcessedChunk),
                     "ns3::RssiPerProcessedChunk::TracedCallback")
  ;
  return tid;
}


void
mmWaveInterference::StartRx (Ptr<const SpectrumValue> rxPsd)
{
  NS_LOG_FUNCTION (this << *rxPsd);
  if (m_receiving == false)
    {
      NS_LOG_INFO ("first signal: " << *rxPsd);
      m_rxSignal = rxPsd->Copy ();
      m_lastChangeTime = Now ();
      m_receiving = true;
      for (std::list<Ptr<mmWaveChunkProcessor> >::const_iterator it = m_PowerChunkProcessorList.begin (); it != m_PowerChunkProcessorList.end (); ++it)
        {
          (*it)->Start ();
        }
      for (std::list<Ptr<mmWaveChunkProcessor> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
        {
          (*it)->Start ();
        }
    }
  else
    {
      NS_LOG_INFO ("additional signal " << *m_rxSignal);
      // receiving multiple simultaneous signals, make sure they are synchronized
      NS_ASSERT (m_lastChangeTime == Now ());
      // make sure they use orthogonal resource blocks
      NS_ASSERT (Sum ((*rxPsd) * (*m_rxSignal)) == 0.0);
      (*m_rxSignal) += (*rxPsd);
    }
}


void
mmWaveInterference::EndRx ()
{
  NS_LOG_FUNCTION (this);
  if (m_receiving != true)
    {
      NS_LOG_INFO ("EndRx was already evaluated or RX was aborted");
    }
  else
    {
      ConditionallyEvaluateChunk ();
      m_receiving = false;
      for (std::list<Ptr<mmWaveChunkProcessor> >::const_iterator it = m_PowerChunkProcessorList.begin (); it != m_PowerChunkProcessorList.end (); ++it)
        {
          (*it)->End ();
        }
      for (std::list<Ptr<mmWaveChunkProcessor> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
        {
          (*it)->End ();
        }
    }
}


void
mmWaveInterference::AddSignal (Ptr<const SpectrumValue> spd, const Time duration)
{
  NS_LOG_FUNCTION (this << *spd << duration);

  // Integrate over our receive bandwidth.
  // Note that differently from wifi, we do not need to pass the
  // signal through the filter. This is because
  // before receiving the signal already passed through the
  // spectrum converter, thus we will consider only the power over the
  // spectrum that corresponds to the spectrum of the receiver.
  // Also, differently from wifi we do not account here for the antenna gain,
  // since this is already taken into account by the spectrum channel.
  double rxPowerW = Integral (*spd);
  // We are creating two events, one that adds the rxPowerW, and
  // another that substracts the rxPowerW at the endTime.
  // These events will be used to determine if the channel is busy and
  // for how long.
  AppendEvent (Simulator::Now(), Simulator::Now() + duration, rxPowerW);

  DoAddSignal (spd);
  uint32_t signalId = ++m_lastSignalId;
  if (signalId == m_lastSignalIdBeforeReset)
    {
      // This happens when m_lastSignalId eventually wraps around. Given that so
      // many signals have elapsed since the last reset, we hope that by now there is
      // no stale pending signal (i.e., a signal that was scheduled
      // for subtraction before the reset). So we just move the
      // boundary further.
      m_lastSignalIdBeforeReset += 0x10000000;
    }
  Simulator::Schedule (duration, &mmWaveInterference::DoSubtractSignal, this, spd, signalId);
}


void
mmWaveInterference::DoAddSignal  (Ptr<const SpectrumValue> spd)
{
  NS_LOG_FUNCTION (this << *spd);
  ConditionallyEvaluateChunk ();
  (*m_allSignals) += (*spd);
}

void
mmWaveInterference::DoSubtractSignal  (Ptr<const SpectrumValue> spd, uint32_t signalId)
{
  NS_LOG_FUNCTION (this << *spd);
  ConditionallyEvaluateChunk ();
  int32_t deltaSignalId = signalId - m_lastSignalIdBeforeReset;
  if (deltaSignalId > 0)
    {
      (*m_allSignals) -= (*spd);
    }
  else
    {
      NS_LOG_INFO ("ignoring signal scheduled for subtraction before last reset");
    }
}


void
mmWaveInterference::ConditionallyEvaluateChunk ()
{
  NS_LOG_FUNCTION (this);
  if (m_receiving)
    {
      NS_LOG_DEBUG (this << " Receiving");
    }
  NS_LOG_DEBUG (this << " now "  << Now () << " last " << m_lastChangeTime);
  if (m_receiving && (Now () > m_lastChangeTime))
    {
      NS_LOG_LOGIC (this << " signal = " << *m_rxSignal << " allSignals = " << *m_allSignals << " noise = " << *m_noise);
      SpectrumValue interf =  (*m_allSignals) - (*m_rxSignal) + (*m_noise);
      SpectrumValue sinr = (*m_rxSignal) / interf;
      SpectrumValue snr = (*m_rxSignal) / (*m_noise);
      double avgSnr = Sum (snr) /(snr.GetSpectrumModel ()->GetNumBands ());
      m_snrPerProcessedChunk (avgSnr);
      NS_LOG_DEBUG ("All signals: "<<(*m_allSignals)[0]<<", rxSingal:"<<(*m_rxSignal)[0]<<" , noise:"<< (*m_noise)[0]);

      double rbWidth = snr.GetSpectrumModel ()->Begin()->fh - snr.GetSpectrumModel ()->Begin()->fl;
      double rssidBm = 10 * log10(Sum((*m_noise + *m_allSignals)* rbWidth)*1000);
      m_rssiPerProcessedChunk(rssidBm);
      
      Time duration = Now () - m_lastChangeTime;
      for (std::list<Ptr<mmWaveChunkProcessor> >::const_iterator it = m_PowerChunkProcessorList.begin (); it != m_PowerChunkProcessorList.end (); ++it)
        {
          (*it)->EvaluateChunk (*m_rxSignal, duration);
        }
      for (std::list<Ptr<mmWaveChunkProcessor> >::const_iterator it = m_sinrChunkProcessorList.begin (); it != m_sinrChunkProcessorList.end (); ++it)
        {
          (*it)->EvaluateChunk (sinr, duration);
        }
      m_lastChangeTime = Now ();
    }
}

void
mmWaveInterference::SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd)
{
  NS_LOG_FUNCTION (this << *noisePsd);
  ConditionallyEvaluateChunk ();
  m_noise = noisePsd;
  m_allSignals = Create<SpectrumValue> (noisePsd->GetSpectrumModel ());
  if (m_receiving == true)
    {
      // abort rx
      m_receiving = false;
    }
  m_lastSignalIdBeforeReset = m_lastSignalId;
}

void
mmWaveInterference::AddPowerChunkProcessor (Ptr<mmWaveChunkProcessor> p)
{
  NS_LOG_FUNCTION (this << p);
  m_PowerChunkProcessorList.push_back (p);
}

void
mmWaveInterference::AddSinrChunkProcessor (Ptr<mmWaveChunkProcessor> p)
{
  NS_LOG_FUNCTION (this << p);
  m_sinrChunkProcessorList.push_back (p);
}

/****************************************************************
 *       Class which records SNIR change events for a
 *       short period of time.
 ****************************************************************/

mmWaveInterference::NiChange::NiChange (Time time, double delta)
  : m_time (time),
    m_delta (delta)
{
}

Time
mmWaveInterference::NiChange::GetTime (void) const
{
  return m_time;
}

double
mmWaveInterference::NiChange::GetDelta (void) const
{
  return m_delta;
}

bool
mmWaveInterference::NiChange::operator < (const mmWaveInterference::NiChange& o) const
{
  return (m_time < o.m_time);
}

bool
mmWaveInterference::IsChannelBusyNow (double energyW)
{
  double detectedPowerW = Integral (*m_allSignals);
  double powerDbm = 10 * log10 (detectedPowerW * 1000);

  NS_LOG_INFO("IsChannelBusyNow detected power is: "<<powerDbm <<
              "  detectedPowerW: "<< detectedPowerW << " length spectrum: "<< 
              (*m_allSignals).GetValuesN() <<" thresholdW:"<< energyW);

  if (detectedPowerW > energyW)
    {
      NS_LOG_INFO ("Channel is BUSY.");
      return true;
    }
  else
    {
      NS_LOG_INFO ("Channel is IDLE.");
      return false;
    }
}

Time
mmWaveInterference::GetEnergyDuration (double energyW)
{
  if (!IsChannelBusyNow (energyW))
    {
      return Seconds (0);
    }

  Time now = Simulator::Now ();
  double noiseInterferenceW = 0.0;
  Time end = now;
  noiseInterferenceW = m_firstPower;

  NS_LOG_INFO("First power: " << m_firstPower);

  for (NiChanges::const_iterator i = m_niChanges.begin (); i != m_niChanges.end (); i++)
    {
      noiseInterferenceW += i->GetDelta ();
      end = i->GetTime ();
      NS_LOG_INFO ("Delta: " << i->GetDelta () << "time: " << i->GetTime ());
      if (end < now)
        {
          continue;
        }
      if (noiseInterferenceW < energyW)
        {
          break;
        }
    }
    
  NS_LOG_INFO("Future power dBm:"<<10 * log10 (noiseInterferenceW*1000)<<" W:"<<noiseInterferenceW <<
  " and energy threshold in W is: "<< energyW);

  if (end > now)
    {
      NS_LOG_INFO ("Channel BUSY until."<<end);

    }
  else
    {
      NS_LOG_INFO ("Channel IDLE.");
    }

  return end > now ? end - now : MicroSeconds (0);
}

void
mmWaveInterference::EraseEvents (void)
{
  m_niChanges.clear ();
  m_firstPower = 0.0;
}

mmWaveInterference::NiChanges::iterator
mmWaveInterference::GetPosition (Time moment)
{
  return std::upper_bound (m_niChanges.begin (), m_niChanges.end (), NiChange (moment, 0));
}

void
mmWaveInterference::AddNiChangeEvent (NiChange change)
{
  m_niChanges.insert (GetPosition (change.GetTime ()), change);
}

void
mmWaveInterference::AppendEvent (Time startTime, Time endTime, double rxPowerW)
{
  Time now = Simulator::Now ();
  
  if (!m_receiving)
    {
      NiChanges::iterator nowIterator = GetPosition (now);
      // We empty the list until the current moment. To do so we 
      // first we sum all the energies until the current moment 
      // and save it in m_firstPower.
      for (NiChanges::iterator i = m_niChanges.begin (); i != nowIterator; i++)
        {
          m_firstPower += i->GetDelta ();
        }
      // then we remove all the events up to the current moment
      m_niChanges.erase (m_niChanges.begin (), nowIterator);
      // we create an event that represents the new energy
      m_niChanges.insert (m_niChanges.begin (), NiChange (startTime, rxPowerW));
    }
  else
    {
      // for the startTime create the event that adds the energy
      AddNiChangeEvent (NiChange (startTime, rxPowerW));
    }

  // for the endTime create event that will substract energy
  AddNiChangeEvent (NiChange (endTime, - rxPowerW));
}

} // namespace ns3


