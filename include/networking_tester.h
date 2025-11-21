/**
 * @file networking_tester.h
 * @brief Networking peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the Networking tester class that implements comprehensive
 * testing and monitoring of networking functionality on the NXP FRDM-IMX93.
 */

#ifndef NETWORKING_TESTER_H
#define NETWORKING_TESTER_H

#include <cstdint>
#include <string>
#include <vector>

#include "peripheral_tester.h"

namespace imx93_peripheral_test {

/**
 * @enum NetworkInterfaceType
 * @brief Types of network interfaces.
 */
enum class NetworkInterfaceType { ETHERNET, WIFI, BLUETOOTH };

/**
 * @enum NetworkProtocol
 * @brief Network protocols to test.
 */
enum class NetworkProtocol { IPv4, IPv6, TCP, UDP, ICMP };

/**
 * @struct NetworkInterfaceInfo
 * @brief Structure containing network interface information.
 */
struct NetworkInterfaceInfo {
  std::string          interface_name;
  NetworkInterfaceType type;
  std::string          mac_address;
  std::string          ip_address;
  std::string          subnet_mask;
  std::string          gateway;
  uint32_t             mtu;
  bool                 is_up;
  bool                 has_carrier;
  uint64_t             rx_bytes;
  uint64_t             tx_bytes;
  uint32_t             rx_packets;
  uint32_t             tx_packets;
};

/**
 * @struct NetworkTestResult
 * @brief Structure containing network test results.
 */
struct NetworkTestResult {
  NetworkProtocol protocol;
  bool            test_passed;
  uint32_t        latency_ms;
  uint32_t        bandwidth_mbps;
  std::string     error_message;
};

/**
 * @class NetworkingTester
 * @brief Tester implementation for networking peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of networking functionality including interface
 * enumeration, connectivity testing, and performance validation.
 */
class NetworkingTester : public PeripheralTester {
public:
  /**
   * @brief Constructs a Networking tester instance.
   */
  NetworkingTester();

  /**
   * @brief Performs short verification test of networking functionality.
   *
   * Tests basic networking operations including:
   * - Interface enumeration
   * - IP connectivity
   * - DNS resolution
   *
   * @return TestReport with detailed results.
   */
  TestReport short_test() override;

  /**
   * @brief Performs extended monitoring of networking performance.
   *
   * Monitors networking over time for:
   * - Connection stability
   * - Bandwidth consistency
   * - Packet loss monitoring
   *
   * @param duration Monitoring duration in seconds.
   * @return TestReport with monitoring results.
   */
  TestReport monitor_test(std::chrono::seconds duration) override;

  /**
   * @brief Returns the peripheral name.
   * @return "Networking" as the peripheral identifier.
   */
  std::string get_peripheral_name() const override {
    return "Networking";
  }

  /**
   * @brief Checks if networking is available on the system.
   * @return true if network interfaces are accessible.
   */
  bool is_available() const override;

private:
  /**
   * @brief Enumerates network interfaces.
   * @return Vector of NetworkInterfaceInfo structures.
   */
  std::vector<NetworkInterfaceInfo> enumerate_interfaces();

  /**
   * @brief Tests basic IP connectivity.
   * @return TestResult indicating success or failure.
   */
  TestResult test_connectivity();

  /**
   * @brief Tests DNS resolution.
   * @return TestResult indicating success or failure.
   */
  TestResult test_dns_resolution();

  /**
   * @brief Tests network bandwidth.
   * @return TestResult indicating success or failure.
   */
  TestResult test_bandwidth();

  /**
   * @brief Tests network latency.
   * @return TestResult indicating success or failure.
   */
  TestResult test_latency();

  /**
   * @brief Monitors network connectivity over time.
   * @param duration Monitoring duration.
   * @return TestResult indicating success or failure.
   */
  TestResult monitor_connectivity(std::chrono::seconds duration);

  /**
   * @brief Parses network interface information.
   * @param interface_name Name of the network interface.
   * @return NetworkInterfaceInfo structure.
   */
  NetworkInterfaceInfo parse_interface_info(const std::string& interface_name);

  /**
   * @brief Gets the default gateway.
   * @return Default gateway IP address.
   */
  std::string get_default_gateway();

  /**
   * @brief Pings a host to test connectivity.
   * @param host Host to ping.
   * @return TestResult indicating success or failure.
   */
  TestResult ping_host(const std::string& host);

  std::vector<NetworkInterfaceInfo> interfaces_;
  bool                              networking_available_;
};

}  // namespace imx93_peripheral_test

#endif  // NETWORKING_TESTER_H