/*
 * config.hpp
 *
 *  Created on: 16 wrz 2023
 *      Author: kwarc
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <cstdint>

namespace config
{

/* Network configuration */
constexpr inline uint8_t mac_addr[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };
constexpr inline uint8_t ip_addr[4] = { 192, 168, 1, 30 };
constexpr inline uint8_t net_mask[4] = { 255, 255, 255, 0 };
constexpr inline uint8_t gateway_addr[4] = { 192, 168, 1, 1 };
constexpr inline uint8_t dns_addr[4] = { 208, 67, 222, 222 };

}

#endif /* CONFIG_HPP_ */
