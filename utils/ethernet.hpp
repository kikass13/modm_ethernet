#pragma once

#include <modm/board.hpp>
#include <modm/driver/ethernet/lan8720a.hpp>
#include <modm/processing/rtos.hpp>

#include <FreeRTOS_IP.h>
#include <FreeRTOS_Sockets.h>

#include <utils/singleton.hpp>

#include <functional>

#define MAC_ADDRESS     0x02, 0x00, 0x00, 0x00, 0x00, 0x00
#define IP_ADDRESS      192, 168, 1, 1
#define SUBNETMASK      255, 255, 255, 0
#define GATEWAY_ADDRESS 0, 0, 0, 0
#define DNS_ADDRESS     0, 0, 0, 0


namespace EthernetInterface
{
	using RMII_Ref_Clk = GpioInputA1;
	using RMII_Mdio = GpioA2;
	using RMII_Crs_Dv = GpioInputA7;
	using RMII_Tx_En = GpioOutputG11;
	using RMII_Tx_D0 = GpioOutputG13;
	using RMII_Tx_D1 = GpioOutputB13;
	using RMII_Mdc = GpioOutputC1;
	using RMII_Rx_D0 = GpioInputC4;
	using RMII_Rx_D1 = GpioInputC5;
	using Port = Eth<modm::Lan8720a>;

#define MII_PHY_PORT_MAPPING    EthernetInterface::RMII_Ref_Clk::Refclk, EthernetInterface::RMII_Mdc::Mdc, \
                                EthernetInterface::RMII_Mdio::Mdio,      EthernetInterface::RMII_Crs_Dv::Rcccrsdv, \
                                EthernetInterface::RMII_Tx_En::Txen,     EthernetInterface::RMII_Tx_D0::Txd0, \
                                EthernetInterface::RMII_Tx_D1::Txd1,     EthernetInterface::RMII_Rx_D0::Rxd0, \
                                EthernetInterface::RMII_Rx_D1::Rxd1

}



namespace EthernetUtils{


class NetworkInitTask : modm::rtos::Thread
{
public:
	NetworkInitTask()
	: Thread(5, 2048, "NetworkInitalization")
	{}

	void run()
	{
        MODM_LOG_DEBUG << "  -- Network initialization task run()" << modm::endl;
   
		uint8_t ipAddress[4] { IP_ADDRESS };
		uint8_t netmask[4] { SUBNETMASK };
		uint8_t gatewayAddress[4] { GATEWAY_ADDRESS };
		uint8_t dnsAddress[4] { DNS_ADDRESS };

		// local MAC address
		uint8_t macAddress[6] { MAC_ADDRESS };

		// A real MAC address can be retrieved from the Microchip 24AA02E48
		// I2C EEPROM, which is located at address 0xFA.

	    FreeRTOS_IPInit(ipAddress, netmask, gatewayAddress, dnsAddress, macAddress);
	    vTaskDelete(0);
	}
};

class EthernetManager :  public singleton_type<EthernetManager>{
public:
    EthernetManager()
    {
        MODM_LOG_INFO << "Ethernet manager initializing ... " << modm::endl;
    }

    bool initialize(){
        bool success = true;
        EthernetInterface::Port::connect<MII_PHY_PORT_MAPPING>();
        return success;
    }
    void registerOnNetworkUpCallback(std::function<void(void)> f){
        EthernetManager::userOnNetworkUp_ = f;
    }
    void registerOnNetworkDownCallback(std::function<void(void)> f){
        EthernetManager::userOnNetworkDown_ = f;
    }
    void onNetworkUp(){
        if(userOnNetworkUp_ != nullptr)
            userOnNetworkUp_();
    }
    void onNetworkDown(){
        if(userOnNetworkDown_ != nullptr)
            userOnNetworkDown_();
    }

public:
private:
    static NetworkInitTask networkInit_; 
    static std::function<void(void)> userOnNetworkUp_;
    static std::function<void(void)> userOnNetworkDown_;
};
NetworkInitTask EthernetManager::networkInit_{};
std::function<void(void)> EthernetManager::userOnNetworkUp_{};
std::function<void(void)> EthernetManager::userOnNetworkDown_{};

} // end namespace EtherUtils


// global hook for free rtos ip regarding interface event handling
void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent);
void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
    if(eNetworkEvent == eNetworkUp){
        EthernetUtils::EthernetManager::get().onNetworkUp();
        
        // this has to be moved at some point, relevant, but should be somewhere else
        uint32_t ipAddress;
        uint32_t netmask;
        uint32_t gateway;
        uint32_t dns;
        char buffer[16];
        FreeRTOS_GetAddressConfiguration(&ipAddress, &netmask, &gateway, &dns);
        FreeRTOS_inet_ntoa(ipAddress, buffer);
        MODM_LOG_DEBUG << "   > IP address: " << buffer << modm::endl;
        FreeRTOS_inet_ntoa(netmask, buffer);
        MODM_LOG_DEBUG << "   > Netmask   : " << buffer << modm::endl;
        FreeRTOS_inet_ntoa(gateway, buffer);
        MODM_LOG_DEBUG << "   > Gateway   : " << buffer << modm::endl;
        FreeRTOS_inet_ntoa(dns, buffer);
        MODM_LOG_DEBUG << "   > DNS       : " << buffer << modm::endl;
    }
    else if(eNetworkEvent == eNetworkDown){
        EthernetUtils::EthernetManager::get().onNetworkDown();
    }
    else{
        // impossible
    }
}
