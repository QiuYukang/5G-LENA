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

#ifndef TEST_NR_SL_SCI_HEADERS
#define TEST_NR_SL_SCI_HEADERS


#include <ns3/test.h>
#include <ns3/nr-sl-sci-f01-header.h>
#include <ns3/nr-sl-sci-f02-header.h>

using namespace ns3;

/**
 * \brief Test suite for
 *
 * \sa ns3::NrSlSciF01TestCase
 * \sa ns3::NrSlSciF02TestCase
 */
class NrSlSciHeadersTestSuite : public TestSuite
{
public:
  NrSlSciHeadersTestSuite ();
};

/**
 * \ingroup nr
 *
 * \brief Testing NR Sidelink SCI format 01 header for it correct serialization
 *        and deserialzation
 */
class NrSlSciF01TestCase : public TestCase
{
public:

  /**
   * \brief Creates an instance of the NR Sidelink SCI Format 01 test case.

   * \param sciF01 SCI format 01 header
   * \param expectedHeaderSize The expected size of the header
   */
  NrSlSciF01TestCase (NrSlSciF01Header sciF01, uint16_t expectedHeaderSize);

  virtual ~NrSlSciF01TestCase ();

private:
  /**
   * \brief Builds the test name string based on provided parameter values
   * \param sciF01 the SCI format 01 header
   * \param expectedHeaderSize The expected header size
   * \returns the name string
   */
  std::string BuildNameString (const NrSlSciF01Header &sciF01, uint16_t expectedHeaderSize);
  /**
   * \brief Setup the simulation according to the configuration set by the
   *        class constructor, run it, and verify the result.
   */
  virtual void DoRun ();

  NrSlSciF01Header m_sciF01; //!< SCI format 01 header
  uint16_t m_expectedHeaderSize; //!< The expected header size


}; // end of class NrSlSciF01TestCase

/**
 * \ingroup nr
 *
 * \brief Testing NR Sidelink SCI format 02 header for it correct serialization
 *        and deserialzation
 */
class NrSlSciF02TestCase : public TestCase
{
public:

  /**
   * \brief Creates an instance of the NR Sidelink SCI Format 02 test case.

   * \param sciF02 SCI format 02 header
   * \param expectedHeaderSize The expected size of the header
   */
  NrSlSciF02TestCase (NrSlSciF02Header sciF02, uint16_t expectedHeaderSize);

  virtual ~NrSlSciF02TestCase ();

private:
  /**
   * \brief Builds the test name string based on provided parameter values
   * \param sciF01 the SCI format 02 header
   * \param expectedHeaderSize The expected header size
   * \returns the name string
   */
  std::string BuildNameString (const NrSlSciF02Header &sciF02, uint16_t expectedHeaderSize);
  /**
   * \brief Setup the simulation according to the configuration set by the
   *        class constructor, run it, and verify the result.
   */
  virtual void DoRun ();

  NrSlSciF02Header m_sciF02; //!< SCI format 02 header
  uint16_t m_expectedHeaderSize; //!< The expected header size

}; // end of class NrSlSciF02TestCase

#endif /* TEST_NR_SL_SCI_HEADERS */
