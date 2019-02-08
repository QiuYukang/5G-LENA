# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('nr', ['core','network', 'spectrum', 'virtual-net-device','point-to-point','applications','internet', 'lte', 'propagation'])
    module.source = [
        'helper/mmwave-helper.cc',
        'helper/mmwave-phy-rx-trace.cc',
        'helper/mmwave-point-to-point-epc-helper.cc',
        'helper/mmwave-bearer-stats-calculator.cc',        
        'helper/mmwave-bearer-stats-connector.cc',           
        'model/mmwave-net-device.cc',
        'model/mmwave-enb-net-device.cc',
        'model/mmwave-ue-net-device.cc',
        'model/mmwave-phy.cc',
        'model/mmwave-enb-phy.cc',
        'model/mmwave-ue-phy.cc',
        'model/mmwave-spectrum-phy.cc',
        'model/mmwave-spectrum-value-helper.cc',
        'model/mmwave-interference.cc',
        'model/mmwave-chunk-processor.cc',
        'model/mmwave-mac.cc',
        'model/mmwave-mac-scheduler.cc',
        'model/mmwave-mac-scheduler-tdma-rr.cc',
        'model/mmwave-mac-scheduler-tdma-pf.cc',
        'model/mmwave-mac-scheduler-ofdma-rr.cc',
        'model/mmwave-mac-scheduler-ofdma-pf.cc',
        'model/mmwave-control-messages.cc',
        'model/mmwave-spectrum-signal-parameters.cc',
        'model/mmwave-radio-bearer-tag.cc',
        'model/nr-amc.cc',
        'model/mmwave-phy-mac-common.cc',
        'model/mmwave-mac-sched-sap.cc',
        'model/mmwave-phy-sap.cc',
        'model/nr-lte-mi-error-model.cc',
        'model/mmwave-enb-mac.cc',
        'model/mmwave-ue-mac.cc',
        'model/mmwave-rrc-protocol-ideal.cc',
        'model/buildings-obstacle-propagation-loss-model.cc',
        'model/mmwave-mac-pdu-header.cc',
        'model/mmwave-mac-pdu-tag.cc',
        'model/mmwave-harq-phy.cc',
        'model/mmwave-propagation-loss-model.cc',
        'model/antenna-array-model.cc',
        'model/antenna-array-basic-model.cc',
        'model/antenna-array-3gpp-model.cc',
        'model/mmwave-3gpp-propagation-loss-model.cc',
        'model/mmwave-3gpp-channel.cc', 
        'model/mmwave-3gpp-buildings-propagation-loss-model.cc',
        'model/component-carrier-gnb.cc',
        'model/component-carrier-mmwave-ue.cc',
        'model/bwp-manager-gnb.cc',
        'model/bwp-manager-ue.cc',
        'model/bwp-manager-algorithm.cc',
        'model/mmwave-mac-harq-vector.cc',
        'model/mmwave-mac-scheduler-harq-rr.cc',
        'model/mmwave-mac-scheduler-cqi-management.cc', 
        'model/mmwave-mac-scheduler-lcg.cc',
        'model/mmwave-mac-scheduler-ns3.cc',
        'model/mmwave-mac-scheduler-ns3-base.cc',
        'model/mmwave-mac-scheduler-tdma.cc',
        'model/mmwave-mac-scheduler-ofdma.cc',
        'model/mmwave-mac-scheduler-ofdma-mr.cc',
        'model/mmwave-mac-scheduler-tdma-mr.cc',
        'model/mmwave-mac-scheduler-ue-info-pf.cc',
        'model/nr-eesm-error-model.cc',
        'model/nr-error-model.cc',
        ]

    module_test = bld.create_ns3_module_test_library('nr')
    module_test.source = [
        'test/mmwave-system-test-configurations.cc',
        'test/mmwave-test-numerology-delay.cc',
        'test/mmwave-test-fdm-of-numerologies.cc',
        'test/mmwave-test-sched.cc',
        'test/mmwave-system-test-schedulers.cc',
        'test/test-antenna-3gpp-model-conf.cc',
        'test/nr-test-l2sm-eesm.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'nr'
    headers.source = [
        'helper/mmwave-helper.h',
        'helper/mmwave-phy-rx-trace.h',
        'helper/mmwave-point-to-point-epc-helper.h',
        'helper/mmwave-bearer-stats-calculator.h',        
        'helper/mmwave-bearer-stats-connector.h',        
        'model/mmwave-net-device.h',
        'model/mmwave-enb-net-device.h',
        'model/mmwave-ue-net-device.h',
        'model/mmwave-phy.h',
        'model/mmwave-enb-phy.h',
        'model/mmwave-ue-phy.h',
        'model/mmwave-spectrum-phy.h',
        'model/mmwave-spectrum-value-helper.h',
        'model/mmwave-interference.h',
        'model/mmwave-chunk-processor.h',
        'model/mmwave-mac.h',
        'model/mmwave-phy-mac-common.h',
        'model/mmwave-mac-scheduler.h',
        'model/mmwave-mac-scheduler-tdma-rr.h',
        'model/mmwave-mac-scheduler-tdma-pf.h',
        'model/mmwave-mac-scheduler-ofdma-rr.h',
        'model/mmwave-mac-scheduler-ofdma-pf.h',
        'model/mmwave-control-messages.h',
        'model/mmwave-spectrum-signal-parameters.h',
        'model/mmwave-radio-bearer-tag.h',
        'model/nr-amc.h',
        'model/mmwave-mac-sched-sap.h',
        'model/mmwave-mac-csched-sap.h',        
        'model/mmwave-phy-sap.h',
        'model/nr-lte-mi-error-model.h',
        'model/mmwave-enb-mac.h',
        'model/mmwave-ue-mac.h',
        'model/mmwave-rrc-protocol-ideal.h',
        'model/buildings-obstacle-propagation-loss-model.h',
        'model/mmwave-mac-pdu-header.h',
        'model/mmwave-mac-pdu-tag.h',
        'model/mmwave-harq-phy.h',
        'model/mmwave-propagation-loss-model.h',
        'model/antenna-array-model.h',
        'model/antenna-array-basic-model.h',
        'model/antenna-array-3gpp-model.h',
        'model/mmwave-3gpp-propagation-loss-model.h',
        'model/mmwave-3gpp-channel.h',
        'model/mmwave-3gpp-buildings-propagation-loss-model.h',
        'model/component-carrier-gnb.h',
        'model/component-carrier-mmwave-ue.h',
        'model/bwp-manager-gnb.h',
        'model/bwp-manager-ue.h',
        'model/bwp-manager-algorithm.h',
        'model/mmwave-mac-harq-process.h',
        'model/mmwave-mac-harq-vector.h',
        'model/mmwave-mac-scheduler-harq-rr.h',
        'model/mmwave-mac-scheduler-cqi-management.h',
        'model/mmwave-mac-scheduler-lcg.h',
        'model/mmwave-mac-scheduler-ns3.h',
        'model/mmwave-mac-scheduler-ns3-base.h',
        'model/mmwave-mac-scheduler-tdma.h',
        'model/mmwave-mac-scheduler-ofdma.h',
        'model/mmwave-mac-scheduler-ofdma-mr.h',
        'model/mmwave-mac-scheduler-tdma-mr.h',
        'model/mmwave-mac-scheduler-ue-info-mr.h',
        'model/mmwave-mac-scheduler-ue-info-rr.h',
        'model/mmwave-mac-scheduler-ue-info-pf.h',
        'model/mmwave-mac-scheduler-ue-info.h',
        'model/nr-eesm-error-model.h',
        'model/nr-error-model.h',
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


    # bld.ns3_python_bindings()

