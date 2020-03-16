# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('nr', ['core','network', 'spectrum', 'virtual-net-device','point-to-point','applications','internet', 'lte', 'propagation'])
    module.source = [
        'helper/nr-helper.cc',
        'helper/nr-phy-rx-trace.cc',
        'helper/nr-mac-rx-trace.cc',
        'helper/nr-point-to-point-epc-helper.cc',
        'helper/nr-bearer-stats-calculator.cc',
        'helper/nr-bearer-stats-connector.cc',
        'helper/ideal-beamforming-helper.cc',
        'helper/node-distribution-scenario-interface.cc',
        'helper/grid-scenario-helper.cc',
        'helper/cc-bwp-helper.cc',
        'model/nr-net-device.cc',
        'model/nr-gnb-net-device.cc',
        'model/nr-ue-net-device.cc',
        'model/nr-phy.cc',
        'model/nr-gnb-phy.cc',
        'model/nr-ue-phy.cc',
        'model/nr-spectrum-phy.cc',
        'model/nr-spectrum-value-helper.cc',
        'model/nr-interference.cc',
        'model/nr-mac-scheduler.cc',
        'model/nr-mac-scheduler-tdma-rr.cc',
        'model/nr-mac-scheduler-tdma-pf.cc',
        'model/nr-mac-scheduler-ofdma-rr.cc',
        'model/nr-mac-scheduler-ofdma-pf.cc',
        'model/nr-control-messages.cc',
        'model/nr-spectrum-signal-parameters.cc',
        'model/nr-radio-bearer-tag.cc',
        'model/nr-amc.cc',
        'model/nr-phy-mac-common.cc',
        'model/nr-mac-sched-sap.cc',
        'model/nr-phy-sap.cc',
        'model/nr-lte-mi-error-model.cc',
        'model/nr-gnb-mac.cc',
        'model/nr-ue-mac.cc',
        'model/nr-rrc-protocol-ideal.cc',
        'model/nr-mac-pdu-header.cc',
        'model/nr-mac-pdu-tag.cc',
        'model/nr-harq-phy.cc',
        'model/bandwidth-part-gnb.cc',
        'model/bandwidth-part-ue.cc',
        'model/bwp-manager-gnb.cc',
        'model/bwp-manager-ue.cc',
        'model/bwp-manager-algorithm.cc',
        'model/nr-mac-harq-vector.cc',
        'model/nr-mac-scheduler-harq-rr.cc',
        'model/nr-mac-scheduler-cqi-management.cc',
        'model/nr-mac-scheduler-lcg.cc',
        'model/nr-mac-scheduler-ns3.cc',
        'model/nr-mac-scheduler-ns3-base.cc',
        'model/nr-mac-scheduler-tdma.cc',
        'model/nr-mac-scheduler-ofdma.cc',
        'model/nr-mac-scheduler-ofdma-mr.cc',
        'model/nr-mac-scheduler-tdma-mr.cc',
        'model/nr-mac-scheduler-ue-info-pf.cc',
        'model/nr-eesm-error-model.cc',
        'model/nr-eesm-t1.cc',
        'model/nr-eesm-t2.cc',
        'model/nr-eesm-ir.cc',
        'model/nr-eesm-cc.cc',
        'model/nr-eesm-ir-t1.cc',
        'model/nr-eesm-ir-t2.cc',
        'model/nr-eesm-cc-t1.cc',
        'model/nr-eesm-cc-t2.cc',
        'model/nr-error-model.cc',
        'model/nr-ch-access-manager.cc',
        'model/beam-id.cc',
        'model/beamforming-vector.cc',
        'model/beam-manager.cc',
        'model/ideal-beamforming-algorithm.cc',
        'model/sfnsf.cc',
        'model/lena-error-model.cc',
        'model/nr-sl-resource-pool-factory.cc',
        'model/nr-sl-preconfig-resource-pool-factory.cc',
        ]

    module_test = bld.create_ns3_module_test_library('nr')
    module_test.source = [
        'test/nr-system-test-configurations.cc',
        'test/nr-test-numerology-delay.cc',
        'test/nr-test-fdm-of-numerologies.cc',
        'test/nr-test-sched.cc',
        'test/nr-system-test-schedulers.cc',
        'test/test-antenna-3gpp-model-conf.cc',
        'test/nr-test-l2sm-eesm.cc',
        'test/nr-lte-pattern-generation.cc',
        'test/nr-phy-patterns.cc',
        'test/test-sfnsf.cc',
        'test/test-timings.cc',
        'test/test-nr-spectrum-phy.cc',
        'test/nr-lte-cc-bwp-configuration.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'nr'
    headers.source = [
        'helper/nr-helper.h',
        'helper/nr-phy-rx-trace.h',
        'helper/nr-mac-rx-trace.h',
        'helper/nr-point-to-point-epc-helper.h',
        'helper/nr-bearer-stats-calculator.h',
        'helper/nr-bearer-stats-connector.h',
        'helper/ideal-beamforming-helper.h',
        'helper/node-distribution-scenario-interface.h',
        'helper/grid-scenario-helper.h',
        'helper/cc-bwp-helper.h',
        'model/nr-net-device.h',
        'model/nr-gnb-net-device.h',
        'model/nr-ue-net-device.h',
        'model/nr-phy.h',
        'model/nr-gnb-phy.h',
        'model/nr-ue-phy.h',
        'model/nr-spectrum-phy.h',
        'model/nr-spectrum-value-helper.h',
        'model/nr-interference.h',
        'model/nr-mac.h',
        'model/nr-phy-mac-common.h',
        'model/nr-mac-scheduler.h',
        'model/nr-mac-scheduler-tdma-rr.h',
        'model/nr-mac-scheduler-tdma-pf.h',
        'model/nr-mac-scheduler-ofdma-rr.h',
        'model/nr-mac-scheduler-ofdma-pf.h',
        'model/nr-control-messages.h',
        'model/nr-spectrum-signal-parameters.h',
        'model/nr-radio-bearer-tag.h',
        'model/nr-amc.h',
        'model/nr-mac-sched-sap.h',
        'model/nr-mac-csched-sap.h',
        'model/nr-phy-sap.h',
        'model/nr-lte-mi-error-model.h',
        'model/nr-gnb-mac.h',
        'model/nr-ue-mac.h',
        'model/nr-rrc-protocol-ideal.h',
        'model/nr-mac-pdu-header.h',
        'model/nr-mac-pdu-tag.h',
        'model/nr-harq-phy.h',
        'model/bandwidth-part-gnb.h',
        'model/bandwidth-part-ue.h',
        'model/bwp-manager-gnb.h',
        'model/bwp-manager-ue.h',
        'model/bwp-manager-algorithm.h',
        'model/nr-mac-harq-process.h',
        'model/nr-mac-harq-vector.h',
        'model/nr-mac-scheduler-harq-rr.h',
        'model/nr-mac-scheduler-cqi-management.h',
        'model/nr-mac-scheduler-lcg.h',
        'model/nr-mac-scheduler-ns3.h',
        'model/nr-mac-scheduler-ns3-base.h',
        'model/nr-mac-scheduler-tdma.h',
        'model/nr-mac-scheduler-ofdma.h',
        'model/nr-mac-scheduler-ofdma-mr.h',
        'model/nr-mac-scheduler-tdma-mr.h',
        'model/nr-mac-scheduler-ue-info-mr.h',
        'model/nr-mac-scheduler-ue-info-rr.h',
        'model/nr-mac-scheduler-ue-info-pf.h',
        'model/nr-mac-scheduler-ue-info.h',
        'model/nr-eesm-error-model.h',
        'model/nr-eesm-t1.h',
        'model/nr-eesm-t2.h',
        'model/nr-eesm-ir.h',
        'model/nr-eesm-cc.h',
        'model/nr-eesm-ir-t1.h',
        'model/nr-eesm-ir-t2.h',
        'model/nr-eesm-cc-t1.h',
        'model/nr-eesm-cc-t2.h',
        'model/nr-error-model.h',
        'model/nr-ch-access-manager.h',
        'model/beam-id.h',
        'model/beamforming-vector.h',
        'model/beam-manager.h',
        'model/ideal-beamforming-algorithm.h',
        'model/sfnsf.h',
        'model/lena-error-model.h',
        'model/nr-sl-resource-pool-factory.h',
        'model/nr-sl-preconfig-resource-pool-factory.h'
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')
        for dirname in os.listdir('src/nr'):
            if dirname.startswith('.') or dirname == 'examples':
                continue
            path = os.path.join('src/nr', dirname)
            if not os.path.isdir(path):
                continue
            if os.path.exists(os.path.join(path, 'wscript')):
                bld.recurse(dirname)

    obj = bld.create_ns3_program('nr-print-introspected-doxygen', ['nr'])
    obj.source = 'utils/print-introspected-doxygen.cc'
    #obj.use = ['nr']



    # bld.ns3_python_bindings()

