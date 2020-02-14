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

#ifndef MMWAVE_CHUNK_PROCESSOR_H
#define MMWAVE_CHUNK_PROCESSOR_H

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/object.h>

namespace ns3 {

class SpectrumValue;

typedef Callback< void, const SpectrumValue& > mmWaveChunkProcessorCallback;

class mmWaveChunkProcessor : public SimpleRefCount<mmWaveChunkProcessor>
{
public:
  mmWaveChunkProcessor ();
  virtual ~mmWaveChunkProcessor ();

  virtual void AddCallback (mmWaveChunkProcessorCallback c);

  virtual void Start ();

  virtual void EvaluateChunk (const SpectrumValue& sinr, Time duration);

  virtual void End ();

private:
  Ptr<SpectrumValue> m_sumValues;
  Time m_totDuration;

  std::vector<mmWaveChunkProcessorCallback> m_mmWaveChunkProcessorCallbacks;
};

} // namespace ns3



#endif /* MMWAVE_CHUNK_PROCESSOR_H */
