/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_SL_COMM_PRE_CONFIG_RESOURCE_POOL_FACTORY_H
#define NR_SL_COMM_PRE_CONFIG_RESOURCE_POOL_FACTORY_H

#include "nr-sl-comm-resource-pool-factory.h"

#include <ns3/lte-rrc-sap.h>
#include <ns3/simple-ref-count.h>

namespace ns3
{

/** Class to configure and generate resource pools */
class NrSlCommPreconfigResourcePoolFactory : public NrSlCommResourcePoolFactory
{
  public:
    NrSlCommPreconfigResourcePoolFactory();
    ~NrSlCommPreconfigResourcePoolFactory() override;

    /**
     * \brief Create pool
     *
     * Inherited from NrSlCommResourcePoolFactory class
     *
     * \return The struct of type LteRrcSap::SlResourcePoolNr defining the SL pool
     */
    const LteRrcSap::SlResourcePoolNr CreatePool() override;

  private:
    LteRrcSap::SlResourcePoolNr m_pool; //!< Sidelink communication pool
};

} // namespace ns3

#endif /* NR_SL_COMM_PRE_CONFIG_RESOURCE_POOL_FACTORY_H */
