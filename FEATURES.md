<!--
Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)

SPDX-License-Identifier: GPL-2.0-only
-->

# 5G-LENA Features Summary
*(Centre Tecnològic de Telecomunicacions de Catalunya – CTTC)*

_Last verified with ns-3.47 / CTTC-LENA master (October 2025)_

---

## 📘 Overview

This document summarizes the main features currently **supported**, **partially supported**, or **planned** in the **5G-LENA (ns-3 nr)** simulator.
It complements the online documentation at <https://5g-lena.cttc.es/features> and https://cttc-lena.gitlab.io/nr/nrmodule.pdf.

---

## 🔗 Table of Contents
- [PHY Layer](#phy-layer)
- [MAC Layer](#mac-layer)
- [RLC / PDCP / Core Integration](#rlc--pdcp--core-integration)
- [Sidelink (NR-V2X)](#sidelink-nr-v2x)
- [NR-U (Unlicensed NR)](#nr-u-unlicensed-nr)
- [Antenna & Beamforming](#antenna--beamforming)
- [Channel Models & Calibration](#channel-models--calibration)
- [Traffic Models](#traffic-models)
- [Fronthaul (O-RAN split 7.2x)](#fronthaul-o-ran-split-72x)
- [Tools, CI & Framework](#tools-ci--framework)
- [Planned Roadmap](#planned-roadmap)
- [Legend](#legend)
- [Version Information](#version-information)

---

## PHY Layer

| Feature | Status | Notes |
|---|---|---|
| OFDMA (DL/UL) | ✅ Supported | 3GPP-compliant slot/symbol allocation; OFDMA with single analog beam capability |
| TDMA (variable TTIs) | ✅ Supported | Configurable symbol granularity |
| Numerologies (μ = 0–4) | ✅ Supported | Per-BWP SCS |
| TDD and FDD | ✅ Supported | Per-cell configurable TDD string pattern |
| Time-multiplex of shared/control | ✅ Supported | PDCCH/PDSCH in same slot; PUCCH/PUSCH in same slot |
| LDPC coding | ✅ Supported | 3GPP TS 38.212 base graphs 1 & 2 |
| Code block segmentation | ✅ Supported | 3GPP TS 38.212 |
| MCS tables | ✅ Supported | Per BWP; 3GPP TS 38.214 MCS Tables 1 & 2 |
| SRS | ✅ Supported | periodic; scheduled-based SRS (offset and periodicity are reconfigurable) |
| CSI-RS & CSI-IM | ✅ Supported | periodic CSI-RS; used for channel estimation and inter-cell interference measurement (**NR-v4.0**) |
| Analog beamforming | ✅ Supported | Beam search, Direct path, Kronecker and Quasi-omni BF methods; Ideal BF and realistic BF (SRS, imperfect CSI) |
| Digital Precoding | ✅ Supported | Type-I; codebook-based; closed-loop MIMO based on CSI feedback (PMI, RI, CQI) (**NR-v3.0**) |
| SU-MIMO | ✅ Supported | Up to rank-4 and 32 antenna ports (**NR-v3.0**) |
| Sub-band CSI feedback | ✅ Supported | Sub-band CQI and PMI (**NR-v4.0**) |
| Uplink power control | ✅ Supported | Fractional; PUSCH/PUCCH/SRS |
| Channel model (TR 38.901) | ✅ Supported | LOS/NLOS; O2I penetration loss (buildings, car); autocorrelation of shadowing; fast fading |
| Spatial consistency | ⚙️ Experimental | TR 38.901 Procedure A; temporal consistency |

---

## MAC Layer

| Feature | Status | Notes |
|---|---|---|
| Dynamic scheduled-based access | ✅ Supported | UE-specific grants |
| Schedulers: PF / RR / MR | ✅ Supported | Temporal fairness |
| QoS-aware scheduler | ✅ Supported | 5QI-aware; LC byte assignment (**NR-v2.5**) |
| Random TDMA / Random OFDMA schedulers | ✅ Supported | Stress interference testing (**NR-v4.1**) |
| Sub-band-aware scheduler | ✅ Supported | (**NR-v4.0**) |
| Reinforcement-Learning scheduler | ⚙️ Experimental | Via ns3-gym (**NR-v4.0**) |
| HARQ | ✅ Supported | IR and CC methods; configurable number of maximum retransmissions |
| Multiple HARQ processes per UE | ✅ Supported | Configurable; defaults to 16 |
| 3GPP-compliant processing times | ✅ Supported | N0/N1/N2; K0/K1/K2 |
| Adaptive Modulation and Coding (AMC) | ✅ Supported | CQI-driven; Shannon-bound or Error-based models |
| BSR / SR / CQI / RI / PMI processing | ✅ Supported | BSR multiplexed with PUSCH; SR/CQI/RI/PMI in PUCCH |
| Notching mask | ✅ Supported | Per BWP |
| CC / BWP managers | ✅ Supported | Multi-carrier; FDM of numerologies |
| Carrier Aggregation (CA) | ✅ Supported | Inter-CC/BWP scheduling |
| RACH | ⚙️ Partial | contention-based for initial access; RA preamble, RAR, Msg3 and connection setup |
| Handover | ❌ Not yet supported | Planned |

---

## RLC / PDCP / Core Integration

| Feature | Status | Notes |
|---|---|---|
| RLC AM / UM / TM | ✅ Supported | LTE-based |
| PDCP + ROHC | ✅ Supported | Basic header compression |
| SDAP & 5QI handling | ⚙️ Partial | SDAP not implemented, QoS per flow |
| Multi-flow per UE | ✅ Supported | Independent bearers |
| EPC/5GC integration | ⚙️ Partial | Via LTE-EPC model |
| RRC | ⚙️ Partial | Ideal RRC; Real RRC under Experimentation (Planned) |

---

## Sidelink (NR-V2X)

| Feature | Status | Notes |
|---|---|---|
| Broadcast | ✅ Supported | |
| Out-of-coverage | ✅ Supported | direct V2V communication|
| PSCCH/PSSCH | ✅ Supported | Time multiplexing of PSCCH and PSSCH |
| Resource allocation | ✅ Supported | Mode 2: UE-selected, sensing-based |
| Semi-persistent scheduling (SPS) | ✅ Supported | Sensing-based and random SPS |
| Blind retransmissions | ✅ Supported | No feedback |
| HARQ feedback | ❌ Not supported | Being developed |

---

## NR-U (Unlicensed NR)

| Feature | Status | Notes |
|---|---|---|
| LBT Cat2/3/4 | ✅ Supported | LBT after MAC; ED omnidirectional |
| Channel Access Managers | ✅ Supported | AlwaysOn, OnOff, LBT CAMs |
| Wi-Fi coexistence | ⚙️ Partial | Integration with ns-3 Wi-Fi |
| Directional LBT | ⚙️ Partial | Developed; null-space projected LBT and precoding |

---

## Antenna & Beamforming

| Feature | Status | Notes |
|---|---|---|
| Antenna modeling (ULA/UPA) | ✅ Supported | TR 38.901; dual polarization; multi-port; multi-panel UE (**NR-v4.0**) |
| Ideal/Realistic analog BF | ✅ Supported | BF methods: Beam search, Direct LOS path, Kronecker, Quasi-omni |
| Digital precoding | ✅ Supported | Type-I (SU-MIMO); PMI/RI search methods: Ideal, Exhaustive, Maleki, Sasaoka, Fast |
| Beam management (sweep/track) | ⚙️ Partial | Simplified model; no SSB blocks |
| Attach to max-RSRP gNB | ✅ Supported | Handoff margin (**NR-v4.0**) |

---

## Channel Models & Calibration

| Feature | Status | Notes |
|---|---|---|
| 3GPP TR 38.901 | ✅ Supported | Spatial channel model; fast fading, pathloss, and shadowing for UMa, UMi, RMa, InH, V2V (**NR-v2.1**) |
| NYUSIM | ✅ Supported | (**NR-v4.0**) |
| Fluctuating Two-Ray (FTR) | ✅ Supported | (**NR-v4.0**) |
| Legacy/Friis channels | ✅ Supported | Channel matrix approximation (**NR-v4.0**) |
| Calibration examples | ✅ Supported | Indoor/Outdoor based on R1-1709828/RP-180524, and updated to R1-1707360 (MIMO) (**NR-v4.1**) |
| Hexagonal wrap-around | ✅ Supported | Supports rings 0 (1 site), 1 (7 sites) and 3 rings (19 sites) |
| FastFadingConstantPositionMobilityModel | ✅ Supported | Static position with non-zero velocity (fading testing) (**NR-v4.1**) |
| REM maps (DL/UL) | ✅ Supported | SIR, SNR, SINR plots; coverage area and beam-shape maps (**NR-v2.1**) |

---

## Traffic Models

| Feature | Status | Notes |
|---|---|---|
| NGMN apps (FTP, HTTP, video, gaming, VoIP) | ✅ Supported | Traffic-generator framework; mixed traffics (**NR-v2.4**) |
| 3GPP XR (VR/AR/CG, DL/UL streams) | ✅ Supported | TR 38.838; multi-stream XR models (**NR-v2.4**) |
| 3GPP FTP | ✅ Supported | Model 1; TR 36.814 (**NR-v1.2**) |

---

## Fronthaul (O-RAN split 7.2x)

| Feature | Status | Notes |
|---|---|---|
| Fronthaul Control | ✅ Supported | Limited-capacity FH link; FH control methods: Dropping, Postponing, Optimize MCS/RBs); MAC/PHY SAPs (**NR-v3.3**) |
| near-RT RIC | ⚙️ Planned | Under integration and testing |
| xApps | ⚙️ Planned | Under integration and testing |
| E2 | ⚙️ Planned | Under integration and testing |

---

## Tools, CI & Framework

| Feature | Status | Notes |
|---|---|---|
| JSON stats & traces | ✅ Supported | PHY/RLC/PDCP (**NR-v2.6**) |
| Reproducibility tests | ✅ Supported | Examples with KPI checks (**NR-v2.4**) |
| Platform/CI upgrades | ✅ Supported | Clang-format/tidy updates, libc++ checks, Mac CI, style/format jobs (**NR-v3.0->v4.1.1**) |

---

## 🧭 Planned Roadmap (2025 → 2028)

| Area | Possible Enhancements |
|---|---|
| **Channel/Antenna** | Sionna-Ray Tracing channel; hybrid spatial channel model; UE antenna and near-field blocking models for handheld devices |
| **PHY** | Precoding Type-II and MU-MIMO; NTN maturation; TN-NTN integration; SSB blocks transmission and scheduling |
| **MAC** | Handover; Dual Connectivity; SL HARQ; LTM; MU-MIMO schedulers; O-RAN xApp hooks; network slicing |
| **NR-U** | Directional LBT |
| **V2X** | HARQ feedback; multicast/unicast |
| **Framework** | SDAP layer; 5GC; Network Digital Twin; Open RAN |
| **6G-LENA** | NTN; ISAC; FFS; RIS; AI |

---

## Legend

✅ = Supported  ⚙️ = Partial / Experimental  ❌ = Not yet supported

---

## Version Information

- **Latest tagged releases in notes:**
  **NR-v4.1.1** (Oct 16 2025), **NR-v4.1** (Jul 7 2025), **NR-v4.0** (May 15 2025), **NR-v3.3** (Oct 15 2024), **NR-v3.2** (Sep 25 2024), **NR-v3.1** (Jul 19 2024), **NR-v3.0** (Feb 16 2024), **NR-v2.6** (Nov 30 2023), **NR-v2.5** (Jul 26 2023), **NR-v2.4** (Apr 5 2023), **NR-v2.3** (Nov 23 2022), **NR-v2.2** (Jun 3 2022), **NR-v2.1** (May 6 2022), **NR-v2.0** (Apr 21 2022), **NR-v1.3** (Apr 7 2022), **NR-v1.2** (Jun 4 2021), **NR-v1.1** (Mar 2 2021), **NR-v1.0** (Sep 16 2020).
- **ns-3 core compatibility (from notes):** 3.41 → 3.47 (varies by tag; e.g., NR-v4.1.1 with ns-3.46; NR-v4.1 with ns-3.45; NR-v4.0 with ns-3.44; NR-v3.3/3.2/3.1 with ns-3.42; NR-v2.6 with ns-3.40; earlier as listed in each release block).
- **Licensing:** GPLv2; REUSE-compliant since NR-v2.4.
- **Citations (examples):**
  NR-v4.1.1 DOI 10.5281/zenodo.17366429 · NR-v4.1 DOI 10.5281/zenodo.15830121 · NR-v4.0 DOI 10.5281/zenodo.15422217 · NR-v3.3 DOI 10.5281/zenodo.13929095 · NR-v3.2 DOI 10.5281/zenodo.13837253 · NR-v3.1 DOI 10.5281/zenodo.12773723 · NR-v3.0 DOI 10.5281/zenodo.10670856 · NR-v2.6 DOI 10.5281/zenodo.10246105 · NR-v2.5 DOI 10.5281/zenodo.8188631 · NR-v2.4 DOI 10.5281/zenodo.7807983 · NR-v2.3 DOI 10.5281/zenodo.7780747.

---

© 2025 Centre Tecnològic de Telecomunicacions de Catalunya (CTTC) – OpenSim Research Unit
