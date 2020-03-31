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


#ifndef MMWAVE_INTERFERENCE_H
#define MMWAVE_INTERFERENCE_H

#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include <ns3/spectrum-value.h>
#include <string.h>
#include <ns3/mmwave-chunk-processor.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/traced-callback.h>
#include <ns3/vector.h>


namespace ns3 {

/**
 * \ingroup spectrum
 *
 * \brief The mmWaveInterference class
 */
class mmWaveInterference : public Object
{
public:
  mmWaveInterference ();
  virtual ~mmWaveInterference ();
  static TypeId GetTypeId (void);
  virtual void DoDispose () override;
  void StartRx (Ptr<const SpectrumValue> rxPsd);
  void EndRx ();
  void AddSignal (Ptr<const SpectrumValue> spd, const Time duration);
  void SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd);
  void AddPowerChunkProcessor (Ptr<mmWaveChunkProcessor> p);
  void AddSinrChunkProcessor (Ptr<mmWaveChunkProcessor> p);

  /**
   * \brief Checks if the sum of the energy, including the energies that start
   * at this moment is greater than provided energy detection threshold.
   * If yes it returns true, otherwise false.
   * @param energyW energy detection threshold used to evaluate if the channel
   * is busy
   * @return Returns true if the energy is above provided threshold. Otherwise
   * false.
   */
  bool IsChannelBusyNow (double energyW);

  /**
   * \brief Returns the duration of the energy that is above the energy
   * provided detection threshold
   * @param energyW energy detection threshold used to evaluate if the channel
   * is busy
   * @return Duration of the energy that is above provided energy detection
   * threshold.
   */
  Time GetEnergyDuration (double energyW);


  /**
  * \brief Crates events corresponding to the new energy. One event corresponds
  * to the moment when the energy starts, and another to the moment that energy
  * ends and in that event the energy is negative, or it is being substracted.
  * @param startTime Energy start time
  * @param endTime Energy end time
  * @param rxPowerW Power of the energy in Watts
  */
  void AppendEvent (Time startTime, Time endTime, double rxPowerW);
  /**
   * Erase all events.
   */
  void EraseEvents (void);

private:

  /**
     * Noise and Interference (thus Ni) event.
     */
    class NiChange
    {
      public:
      /**
       * Create a NiChange at the given time and the amount of NI change.
       *
       * \param time time of the event
       * \param delta the power
       */
      NiChange (Time time, double delta);
      /**
       * Return the event time.
       *
       * \return the event time.
       */
      Time GetTime (void) const;
      /**
       * Return the power
       *
       * \return the power
       */
      double GetDelta (void) const;
      /**
       * Compare the event time of two NiChange objects (a < o).
       *
       * \param o
       * \return true if a < o.time, false otherwise
       */
      bool operator < (const NiChange& o) const;


  private:
      Time m_time;
      double m_delta;
    };
   /**
    * typedef for a vector of NiChanges
    */
   typedef std::vector <NiChange> NiChanges;

   mmWaveInterference::NiChanges::iterator GetPosition (Time moment);

  void ConditionallyEvaluateChunk ();
  void DoAddSignal (Ptr<const SpectrumValue> spd);
  void DoSubtractSignal  (Ptr<const SpectrumValue> spd, uint32_t signalId);

  /**
   * Add NiChange to the list at the appropriate position.
   *
   * \param change
   */
  void AddNiChangeEvent (NiChange change);

  std::list<Ptr<mmWaveChunkProcessor> > m_PowerChunkProcessorList;
  std::list<Ptr<mmWaveChunkProcessor> > m_sinrChunkProcessorList;

  TracedCallback<double> m_snrPerProcessedChunk; ///<! Trace for SNR per processed chunk.
  TracedCallback<double> m_rssiPerProcessedChunk;  ///<! Trace for RSSI pre processed chunk.

  bool m_receiving;

  Ptr<SpectrumValue> m_rxSignal;
  Ptr<SpectrumValue> m_allSignals;
  Ptr<const SpectrumValue> m_noise;

  Time m_lastChangeTime;

  uint32_t m_lastSignalId;
  uint32_t m_lastSignalIdBeforeReset;

  /// Used for energy duration calculation, inspired by wifi/model/interference-helper implementation
  NiChanges m_niChanges; //!< List of events in whitch there is some change in the energy
  double m_firstPower; //!< This contains the accumulated sum of the energy events until the certain moment it has been calculated


};

} // namespace ns3

#endif /* MMWAVE_INTERFERENCE_H */
