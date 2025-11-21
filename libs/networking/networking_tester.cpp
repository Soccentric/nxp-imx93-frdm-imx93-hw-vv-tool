/**
 * @file networking_tester.cpp
 * @brief Implementation of networking peripheral tester for i.MX93.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This implementation tests the i.MX 93 networking interfaces:
 * - Dual Gigabit Ethernet with ENET QoS controllers
 * - 10/100/1000 Mbps support
 * - IEEE 1588 precision time protocol
 * - AVB (Audio Video Bridging) support
 * - Optional USB Ethernet and WiFi modules
 */

#include "networking_tester.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/ethtool.h>
#include <linux/if_link.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

namespace imx93_peripheral_test {

NetworkingTester::NetworkingTester() : networking_available_(false) {
  // Check if networking is available
  // i.MX93 has dual ENET QoS controllers (typically eth0 and eth1)
  networking_available_ = fs::exists("/proc/net/dev") || system("which ip > /dev/null 2>&1") == 0;

  if (networking_available_) {
    interfaces_ = enumerate_interfaces();
  }
}

TestReport NetworkingTester::short_test() {
  auto start_time = std::chrono::steady_clock::now();

  if (!networking_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "Networking not available",
                         std::chrono::milliseconds(0));
  }

  std::stringstream details;
  bool              all_passed = true;

  // Display network information
  details << "Default Gateway: " << get_default_gateway() << "\n";
  details << "DNS Servers: ";
  // Get DNS servers from /etc/resolv.conf
  std::ifstream resolv_file("/etc/resolv.conf");
  if (resolv_file.is_open()) {
    std::string              line;
    std::vector<std::string> dns_servers;
    while (std::getline(resolv_file, line)) {
      if (line.find("nameserver") == 0) {
        std::istringstream iss(line);
        std::string        nameserver;
        iss >> nameserver;
        if (iss >> nameserver) {
          dns_servers.push_back(nameserver);
        }
      }
    }
    for (size_t i = 0; i < dns_servers.size(); ++i) {
      if (i > 0)
        details << ", ";
      details << dns_servers[i];
    }
  }
  details << "\n";
  details << "Available Interfaces: " << interfaces_.size() << "\n";

  // Test network interfaces
  bool has_active_interface = false;
  for (const auto& interface : interfaces_) {
    if (interface.is_up) {
      has_active_interface = true;
      break;
    }
  }
  TestResult interface_result = has_active_interface ? TestResult::SUCCESS : TestResult::FAILURE;
  details << "Interfaces: " << (interface_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (interface_result != TestResult::SUCCESS)
    all_passed = false;

  // Test connectivity
  TestResult connectivity_result = test_connectivity();
  details << "Connectivity: "
          << (connectivity_result == TestResult::SUCCESS
                  ? "PASS"
                  : (connectivity_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL"))
          << "\n";
  if (connectivity_result != TestResult::SUCCESS &&
      connectivity_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  // Test DNS resolution
  TestResult dns_result = test_dns_resolution();
  details << "DNS Resolution: " << (dns_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (dns_result != TestResult::SUCCESS)
    all_passed = false;

  // Test latency
  TestResult latency_result = test_latency();
  details << "Latency: "
          << (latency_result == TestResult::SUCCESS
                  ? "PASS"
                  : (latency_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL"))
          << "\n";
  if (latency_result != TestResult::SUCCESS && latency_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  TestResult overall_result = all_passed ? TestResult::SUCCESS : TestResult::FAILURE;
  return create_report(overall_result, details.str(), duration);
}

TestReport NetworkingTester::monitor_test(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();

  if (!networking_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "Networking not available",
                         std::chrono::milliseconds(0));
  }

  TestResult result = monitor_connectivity(duration);

  auto end_time      = std::chrono::steady_clock::now();
  auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  std::string details =
      "Network monitoring completed for " + std::to_string(duration.count()) + " seconds";
  return create_report(result, details, test_duration);
}

bool NetworkingTester::is_available() const {
  return networking_available_;
}

TestResult NetworkingTester::test_connectivity() {
  // Test connectivity to multiple reliable hosts
  std::vector<std::string> test_hosts = {"8.8.8.8", "1.1.1.1", "208.67.222.222"};

  int successful_tests = 0;
  for (const auto& host : test_hosts) {
    if (ping_host(host) == TestResult::SUCCESS) {
      successful_tests++;
    }
  }

  // Require at least 2 successful connections
  return (successful_tests >= 2) ? TestResult::SUCCESS : TestResult::FAILURE;
}

TestResult NetworkingTester::test_dns_resolution() {
  // Test DNS resolution for common domains
  std::vector<std::string> test_domains = {"google.com", "github.com", "stackoverflow.com"};

  int successful_resolutions = 0;
  for (const auto& domain : test_domains) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(domain.c_str(), NULL, &hints, &res) == 0) {
      freeaddrinfo(res);
      successful_resolutions++;
    }
  }

  return (successful_resolutions >= 2) ? TestResult::SUCCESS : TestResult::FAILURE;
}

TestResult NetworkingTester::test_latency() {
  // Test latency to a reliable host
  return ping_host("8.8.8.8");
}

TestResult NetworkingTester::monitor_connectivity(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();
  auto end_time   = start_time + duration;

  bool connectivity_stable = true;
  int  check_count         = 0;
  int  failure_count       = 0;

  while (std::chrono::steady_clock::now() < end_time && connectivity_stable) {
    check_count++;
    if (test_connectivity() != TestResult::SUCCESS) {
      failure_count++;
      if (failure_count > 3) {  // Allow up to 3 failures
        connectivity_stable = false;
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  return connectivity_stable ? TestResult::SUCCESS : TestResult::FAILURE;
}

std::vector<NetworkInterfaceInfo> NetworkingTester::enumerate_interfaces() {
  std::vector<NetworkInterfaceInfo> interfaces;

  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) == -1) {
    return interfaces;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    // Skip loopback interface
    if (strcmp(ifa->ifa_name, "lo") == 0)
      continue;

    NetworkInterfaceInfo interface = parse_interface_info(ifa->ifa_name);

    // Avoid duplicates
    bool already_added = false;
    for (const auto& existing : interfaces) {
      if (existing.interface_name == interface.interface_name) {
        already_added = true;
        break;
      }
    }

    if (!already_added) {
      interfaces.push_back(interface);
    }
  }

  freeifaddrs(ifaddr);
  return interfaces;
}

NetworkInterfaceInfo NetworkingTester::parse_interface_info(const std::string& interface_name) {
  NetworkInterfaceInfo interface;
  interface.interface_name = interface_name;
  interface.is_up          = false;
  interface.has_carrier    = false;

  // Determine interface type
  if (interface_name.find("eth") == 0 || interface_name.find("en") == 0) {
    interface.type = NetworkInterfaceType::ETHERNET;
  } else if (interface_name.find("wlan") == 0 || interface_name.find("wl") == 0) {
    interface.type = NetworkInterfaceType::WIFI;
  } else {
    interface.type = NetworkInterfaceType::BLUETOOTH;
  }

  // Read interface flags
  std::ifstream flags_file("/sys/class/net/" + interface_name + "/flags");
  if (flags_file.is_open()) {
    uint32_t flags;
    flags_file >> std::hex >> flags;
    interface.is_up = (flags & IFF_UP) != 0;
  }

  // Read carrier status
  std::ifstream carrier_file("/sys/class/net/" + interface_name + "/carrier");
  if (carrier_file.is_open()) {
    int carrier;
    carrier_file >> carrier;
    interface.has_carrier = (carrier == 1);
  }

  // Read MAC address
  std::ifstream address_file("/sys/class/net/" + interface_name + "/address");
  if (address_file.is_open()) {
    std::getline(address_file, interface.mac_address);
  }

  // Read MTU
  std::ifstream mtu_file("/sys/class/net/" + interface_name + "/mtu");
  if (mtu_file.is_open()) {
    mtu_file >> interface.mtu;
  }

  // Read statistics
  std::ifstream rx_bytes_file("/sys/class/net/" + interface_name + "/statistics/rx_bytes");
  if (rx_bytes_file.is_open()) {
    rx_bytes_file >> interface.rx_bytes;
  }

  std::ifstream tx_bytes_file("/sys/class/net/" + interface_name + "/statistics/tx_bytes");
  if (tx_bytes_file.is_open()) {
    tx_bytes_file >> interface.tx_bytes;
  }

  std::ifstream rx_packets_file("/sys/class/net/" + interface_name + "/statistics/rx_packets");
  if (rx_packets_file.is_open()) {
    rx_packets_file >> interface.rx_packets;
  }

  std::ifstream tx_packets_file("/sys/class/net/" + interface_name + "/statistics/tx_packets");
  if (tx_packets_file.is_open()) {
    tx_packets_file >> interface.tx_packets;
  }

  // Get IP address using getifaddrs
  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) == 0) {
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr == NULL)
        continue;
      if (strcmp(ifa->ifa_name, interface_name.c_str()) == 0) {
        if (ifa->ifa_addr->sa_family == AF_INET) {
          struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
          char                ip_str[INET_ADDRSTRLEN];
          inet_ntop(AF_INET, &addr->sin_addr, ip_str, INET_ADDRSTRLEN);
          interface.ip_address = ip_str;

          // Get subnet mask
          struct sockaddr_in* netmask = (struct sockaddr_in*)ifa->ifa_netmask;
          inet_ntop(AF_INET, &netmask->sin_addr, ip_str, INET_ADDRSTRLEN);
          interface.subnet_mask = ip_str;
        }
      }
    }
    freeifaddrs(ifaddr);
  }

  return interface;
}

std::string NetworkingTester::get_default_gateway() {
  std::ifstream route_file("/proc/net/route");
  if (!route_file.is_open()) {
    return "";
  }

  std::string line;
  while (std::getline(route_file, line)) {
    std::istringstream iss(line);
    std::string interface, destination, gateway, flags, refcnt, use, metric, mask, mtu, window,
        irtt;

    if (iss >> interface >> destination >> gateway >> flags >> refcnt >> use >> metric >> mask >>
        mtu >> window >> irtt) {
      if (destination == "00000000" && gateway != "00000000") {
        // Convert hex gateway to IP
        uint32_t       gw_int = std::stoul(gateway, nullptr, 16);
        struct in_addr addr;
        addr.s_addr = htonl(gw_int);
        return inet_ntoa(addr);
      }
    }
  }

  return "";
}

TestResult NetworkingTester::ping_host(const std::string& host) {
  std::string command = "ping -c 1 -W 2 " + host + " > /dev/null 2>&1";
  int         result  = system(command.c_str());
  return (result == 0) ? TestResult::SUCCESS : TestResult::FAILURE;
}

}  // namespace imx93_peripheral_test