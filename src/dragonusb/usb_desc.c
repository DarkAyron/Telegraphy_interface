/*____________________________________________________,8_________________
 ___________________________________________________,d8*_________________
 _________________________________________________,d88**_________________
 _______________________________________________,d888`**_________________
 ____________________________________________,d888`****__________________
 ___________________________________________,d88`******__________________
 _________________________________________,d88`*********_________________
 ________________________________________,d8`************________________
 ______________________________________,d8****************_______________
 ____________________________________,d88**************..d**`____________
 __________________________________,d88`*********..d8*`****______________
 ________________________________,d888`****..d8P*`********_______________
 ________________________._____,d8888*8888*`*************________________
 ______________________,*_____,88888*8P*****************_________________
 ____________________,*______d888888*8b.****************_________________
 __________________,P_______dP__*888.*888b.**************________________
 ________________,8*________8____*888*8`**88888b.*********_______________
 ______________,dP________________*88_8b.*******88b.******_______________
 _____________d8`__________________*8b_8b.***********8b.***______________
 ___________,d8`____________________*8._8b.**************88b.____________
 __________d8P_______________________88.*8b.***************______________
 ________,88P________________________*88**8b.************________________
 _______d888*_______.d88P____________888***8b.*********__________________
 ______d8888b..d888888*______________888****8b.*******________*__________
 ____,888888888888888b.______________888*****8b.*****_________8__________
 ___,8*;88888P*****788888888ba.______888******8b.****______*_8'_*________
 __,8;,8888*_________`88888*_________d88*******8b.***_______*8*'_________
 __)8e888*__________,88888be._______888*********8b.**_______8'___________
 _,d888`___________,8888888***_____d888**********88b.*____d8'____________
 ,d88P`___________,8888888Pb._____d888`***********888b.__d8'_____________
 888*____________,88888888**___.d8888*************______d8'______________
 `88____________,888888888____.d88888b*********________d88'______________
 ______________,8888888888bd888888********_____________d88'______________
 ______________d888888888888d888********________________d88'_____________
 ______________8888888888888888b.****____________________d88'____________
 ______________88*._*88888888888b.*______.oo._____________d888'__________
 ______________`888b.`8888888888888b._.d8888P_______________d888'________
 _______________**88b.`*8888888888888888888888b...____________d888'______
 ________________*888b.`*8888888888P***7888888888888e.__________d888'____
 _________________88888b.`********.d8888b**__`88888P*____________d888'___
 _________________`888888b_____.d88888888888**__`8888.____________d888'__
 __________________)888888.___d888888888888P______`8888888b.______d88888'
 _________________,88888*____d88888888888**`________`8888b__________d888'
 ________________,8888*____.8888888888P`______________`888b.________d888'
 _______________,888*______888888888b...________________`88P88b.__d8888'_
 ______.db.___,d88*________88888888888888b________________`88888888888___
 __,d888888b.8888`_________`*888888888888888888P`______________******____
 _/*****8888b**`______________`***8888P*``8888`__________________________
 __/**88`_______________________________/**88`___________________________
 __`|'_____________________________`|*8888888'___________________________

 ************************************************************************
 usb_descr.c
 
 Created on: 2 Apr 2017
 
 Copyright 2017 ayron
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ***********************************************************************/

#include "../cpu.h"
#include "usb_core.h"

ROM const uint8_t USB_dev_desc[] =
{
        0x12,       // bLength
        USB_DESC_DEV,  // bDescriptorType
        0x00,
        0x02,       // bcdUSB: 0200 (2.0)
        0x00,       // bDeviceClass: Get from cfg descr
        0x00,       // bDeviceSubClass: Get from cfg descr
        0x00,       // bDeviceProtocol: Get from cfg descr
        MAX_USB_CTRL_EP_SIZE, // bMaxPacketSize
        0x66,
        0xa6,       // idVendor
        0x34,
        0x12,       // idProduct
        0x00,
        0x01,       // bcdDevice: 0100 (v1.00)
        1,          // Index of string descriptor describing manufacturer
        2,          // Index of string descriptor describing product
        3,          // Index of string descriptor describing the device's serial number
        0x01        // bNumConfigurations
};

// cfg descriptor
ROM const uint8_t USB_cfg_desc[] =
{
    // cfg descr
    0x09,           // bLength: cfg desc len
    USB_DESC_CFG,   // bDescriptorType: Configuration
    0x27,           // wTotalLength: 0x27 (39 bytes = total size of cfg + sub descriptors)
    0x00,
    0x01,           // bNumInterfaces: 1 interface
    0x01,           // bConfigurationValue: Configuration value
    0x00,           // iConfiguration: Index of string descriptor describing the configuration
    0xc0,           // bmAttributes: self powered
    0x05,           // MaxPower: 10 mA

    // intf descr
    0x09,           // bLength: Interface Descriptor size
    USB_DESC_INTF,  // bDescriptorType: Interface
    0x00,           // bInterfaceNumber: Number of Interface
    0x00,           // bAlternateSetting: Alternate setting
    0x03,           // bNumEndpoints: Three endpoints used
    0xFE,           // bInterfaceClass: Application Class
    0x03,           // bInterfaceSubClass: TMC
    0x01,           // bInterfaceProtocol: USB488
    0x00,           // iInterface:

    // ep 1 descr
    0x07,           // bLength: Endpoint Descriptor size
    USB_DESC_EP,    // bDescriptorType: Endpoint
    0x01,           // bEndpointAddress: (OUT1)
    0x02,           // bmAttributes: Bulk
    MAX_USB_EP_SIZE_L,// wMaxPacketSize: MAX_BUF_SZ
    MAX_USB_EP_SIZE_H,
    0x00,           // bInterval: ignore for Bulk transfer

    // ep 2 descr
    0x07,           // bLength: Endpoint Descriptor size
    USB_DESC_EP,    // bDescriptorType: Endpoint
    0x82,           // bEndpointAddress: (IN2)
    0x02,           // bmAttributes: Bulk
    MAX_USB_EP_SIZE_L,           // wMaxPacketSize:
    MAX_USB_EP_SIZE_H,
    0x00,           // bInterval: ignore for Bulk transfer

    // ep 3 descr
    0x07,           // bLength: Endpoint Descriptor size
    USB_DESC_EP,    // bDescriptorType: Endpoint
    0x83,           // bEndpointAddress: (IN3)
    0x03,           // bmAttributes: Interrupt
    MAX_USB_EP_SIZE_L,// wMaxPacketSize: MAX_USB_EP_SIZE
    MAX_USB_EP_SIZE_H,
    0x08,           // bInterval: 8
};

ROM const uint8_t USB_dev_qualifier_desc[] =
{
    0x0A,           // bLength
    0x06,           // bDescriptorType: Device Qualifier
    0x00,
    0x02,           // bcdUSB Spec Version: 0200 (2.0)
    0x00,           // bDeviceClass: Get from cfg descr
    0x00,           // bDeviceSubClass: Get from cfg descr
    0x00,           // bDeviceProtocol: Get from cfg descr
    MAX_USB_CTRL_EP_SIZE,// bMaxPacketSize: MAX_BUF_SZ
    0x01,           // bNumConfigurations: 1
    0x00            // bReserved: Don't touch this or the device explodes
};

ROM const uint8_t USB_lang_str_desc[] =
{
    0x4,        // bLength
    USB_DESC_STR,  // bDescriptorType: String
                // Language: English
    0x09, 0x04
};

ROM const uint8_t USB_vendor_str_desc[] =
{
    0x44,       // bLength
    USB_DESC_STR,// bDescriptorType: String
                // Manufacturer: "https://e621.net/post/show/274883"
    0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00,
    0x73, 0x00, 0x3a, 0x00, 0x2f, 0x00, 0x2f, 0x00,
    0x65, 0x00, 0x36, 0x00, 0x32, 0x00, 0x31, 0x00,
    0x2e, 0x00, 0x6e, 0x00, 0x65, 0x00, 0x74, 0x00,
    0x2f, 0x00, 0x70, 0x00, 0x6f, 0x00, 0x73, 0x00,
    0x74, 0x00, 0x2f, 0x00, 0x73, 0x00, 0x68, 0x00,
    0x6f, 0x00, 0x77, 0x00, 0x2f, 0x00, 0x32, 0x00,
    0x37, 0x00, 0x34, 0x00, 0x38, 0x00, 0x38, 0x00,
    0x33, 0x00
};

ROM const uint8_t USB_prod_str_desc[] =
{
    0x2a,      // bLength
    USB_DESC_STR, // bDescriptorType: String
               // Product name: "Telgraphy interface"
    0x54, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x65, 0x00,
    0x67, 0x00, 0x72, 0x00, 0x61, 0x00, 0x70, 0x00,
    0x68, 0x00, 0x79, 0x00, 0x20, 0x00, 0x69, 0x00,
    0x6e, 0x00, 0x74, 0x00, 0x65, 0x00, 0x72, 0x00,
    0x66, 0x00, 0x61, 0x00, 0x63, 0x00, 0x65, 0x00
};

ROM const uint8_t USB_serial_str_desc[] =
{
    0x08,       // bLength
    USB_DESC_STR,// bDescriptorType: String
                // Serial: 0.1
    0x30, 0x00, 0x2e, 0x00, 0x31, 0x00
};

// USB488 Capabilities
ROM const uint8_t USB488_capabilities_response[] =
{
		0x01,	// Success
		0x00,	// reserved
		0x00,	// USBTMC version 1.0
		0x01,
		0x04,	// INDICATOR_PULSE accepted
		0x00,	// no TermChar
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00,	// USB488 version 1.0
		0x01,
		0x07,	// 488.2, REN accepted, TRIGGER accepted
		0x04,	// no SCPI, SR1, RL0, DT0
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
};

