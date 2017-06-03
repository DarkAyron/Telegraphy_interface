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
 __,8;,8888*_________`88888*_________d88********8b.***______*8*'_________
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
 usb_core.c

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

#include <cpu.h>
#include "usb_core.h"
#include "usb_desc.h"
#include "usb488.h"
#include <SEGGER_RTT.h>
#include <SEGGER_SYSVIEW.h>

#define USB_DEBUG

static int isCtrlStalled = 0;
static volatile uint32_t usbCfg = 0;
static volatile uint32_t usbInterface = 0;

static const uint32_t EndPoints[] = {
	EP_TYPE_CONTROL,
	EP_TYPE_BULK_OUT,
	EP_TYPE_BULK_IN,
	EP_TYPE_INTERRUPT_IN	/* for SRQ */
};

int USB_send_ctrl(const uint8_t *data, unsigned int len)
{
	unsigned int bytesSent = 0;
	do {
		bytesSent += UDD_Send(ENDPOINT_CTRL, data + bytesSent, len - bytesSent);
	} while(bytesSent < len);
	return bytesSent;
}

static void USB_SendZlp(void)
{
	while (UOTGHS_DEVEPTISR_TXINI !=
	       (UOTGHS->UOTGHS_DEVEPTISR[0] & UOTGHS_DEVEPTISR_TXINI)) {
		if ((UOTGHS->UOTGHS_DEVISR & UOTGHS_DEVISR_SUSP) ==
		    UOTGHS_DEVISR_SUSP) {
			return;
		}
	}
	UOTGHS->UOTGHS_DEVEPTICR[0] = UOTGHS_DEVEPTICR_TXINIC;
}

void USB_Init()
{
	UDD_Init();
	UDD_Attach();
}

void USB_Reset()
{
#ifdef USB_DEBUG
	SEGGER_RTT_printf(0, "USB: Reset\n");
#endif
	udd_configure_address(0);
	udd_enable_address();

	/* Configure Control Endpoint */
	UDD_InitEP(ENDPOINT_CTRL, EP_TYPE_CONTROL);
	udd_enable_setup_received_interrupt(ENDPOINT_CTRL);
	udd_enable_endpoint_interrupt(ENDPOINT_CTRL);
}

void USB_stallEndpoint(int ep)
{
	if (ep == 0)
		isCtrlStalled = 1;
	UOTGHS->UOTGHS_DEVEPT = (1 << ep);
	UOTGHS->UOTGHS_DEVEPTIER[ep] = UOTGHS_DEVEPTIER_STALLRQS;
}

void USB_unstallEndpoint(int ep)
{
	if (Is_udd_stall(ep)) {
		udd_ack_stall(ep);
		udd_reset_data_toggle(ep);
	}
	udd_disable_stall_handshake(ep);
	if (ep == 0)
		isCtrlStalled = 0;
}

int USB_SendDescriptor(struct USB_Ctrl_Setup *setup)
{
	enum USB_DESC_Type desc_type;
	uint8_t desc_idx;
	const uint8_t *desc;
	unsigned int len;

	desc_type = setup->valueH;
	desc_idx = setup->valueL;

#ifdef USB_DEBUG
	SEGGER_RTT_printf(0, "Descriptor: %d, %d\n", desc_type, desc_idx);
#endif

	switch (desc_type) {
		case USB_DESC_DEV:
			desc = USB_dev_desc;
			break;
		case USB_DESC_DEV_QUAL:
			desc = USB_dev_qualifier_desc;
			break;
		case USB_DESC_CFG:
			desc = USB_cfg_desc;
			break;
		case USB_DESC_STR:
			switch (desc_idx) {
				case 0:
					desc = USB_lang_str_desc;
					break;
				case 1:
					desc = USB_vendor_str_desc;
					break;
				case 2:
					desc = USB_prod_str_desc;
					break;
				case 3:
					desc = USB_serial_str_desc;
					break;
				default:
					desc = 0;
			}
			break;
		default:
			desc = 0;

	}

	if (desc != 0) {
		if (desc_type == USB_DESC_CFG)
			len = desc[2];
		else
			len = desc[0];
		if (setup->length < len)
			len = setup->length;
		USB_send_ctrl(desc, len);
		return 1;
	}
	return 0;
}

void USB_Handler()
{
	SEGGER_SYSVIEW_RecordEnterISR();

	if (Is_udd_reset()) {
		USB_Reset();
		udd_ack_reset();
	}

	if (Is_udd_endpoint_interrupt(ENDPOINT_CTRL)) {
		if (UDD_ReceivedSetupInt()) {
			struct USB_Ctrl_Setup setup;
			int result = 1;
			uint8_t requestType;

			UDD_Recv(ENDPOINT_CTRL, (uint8_t *) & setup, 8);
			UDD_ClearSetupInt();
			requestType = setup.RequestType;
			if (requestType & USB_CTRL_REQUEST_DIR_DEVICETOHOST) {
#ifdef USB_DEBUG
				SEGGER_RTT_printf(0, "USB CTRL: In\n");
#endif
				/* In Request */
				UDD_WaitIN();
			} else {
#ifdef USB_DEBUG
				SEGGER_RTT_printf(0, "USB CTRL: Out\n");
#endif
				/* Out Request */
				UDD_ClearIN();
			}

			if ((requestType & USB_CTRL_REQUEST_TYPE_MASK)
			    == USB_CTRL_REQUEST_TYPE_STANDARD) {
				/* Standard Requests */
				enum USB_CTRL_STD_Requests req = setup.Request;
#ifdef USB_DEBUG
				SEGGER_RTT_printf(0, "USB CTRL: %d\n", req);
#endif
				if (req == USB_CTRL_STD_Req_GET_DEV_STATUS) {
					if (setup.RequestType == USB_CTRL_REQUEST_STATUS_DEVICE)	// device
					{
#ifdef USB_DEBUG
						SEGGER_RTT_printf(0,
								  "USB CTRL: Device Status\n");
#endif
						/* ToDo: Send power status */
						UDD_Send8(ENDPOINT_CTRL, 1);
						UDD_Send8(ENDPOINT_CTRL, 0);
					} else if (setup.RequestType ==
						   USB_CTRL_REQUEST_STATUS_EP)
					{
#ifdef USB_DEBUG
						SEGGER_RTT_printf(0,
								  "USB CTRL: EP Status\n");
#endif
						UDD_Send8(ENDPOINT_CTRL,
							  isCtrlStalled);
						UDD_Send8(ENDPOINT_CTRL, 0);
					} else {
#ifdef USB_DEBUG
						SEGGER_RTT_printf(0, "USB CTRL: Status %d\n", setup.RequestType);
#endif
						UDD_Send8(ENDPOINT_CTRL, 0);
						UDD_Send8(ENDPOINT_CTRL, 0);
					}

				} else if (req ==
					   USB_CTRL_STD_Req_CLEAR_FEATURE) {
					if (setup.valueL == 1) {
#ifdef USB_DEBUG
						SEGGER_RTT_printf(0,
								  "USB CTRL: Clear Remote WakeUp\n");
#endif
						/* DEVICEREMOTEWAKEUP */
						UDD_Send8(ENDPOINT_CTRL, 0);
						UDD_Send8(ENDPOINT_CTRL, 0);
					} else if (setup.valueL == 0) {
#ifdef USB_DEBUG
						SEGGER_RTT_printf(0,
								  "USB CTRL: Unstall\n");
#endif
						/* ENDPOINTHALT */
						USB_unstallEndpoint(setup.index);

						UDD_Send8(ENDPOINT_CTRL, 0);
						UDD_Send8(ENDPOINT_CTRL, 0);
					} else {
#ifdef USB_DEBUG
						SEGGER_RTT_printf(0,
								  "USB CTRL: Clear: ?Unknown feature\n");
#endif
						result = 0;
					}
				} else if (req == USB_CTRL_STD_Req_SET_FEATURE) {
					/* Check which is the selected feature */
					if (setup.valueL == 1) {
#ifdef USB_DEBUG
						SEGGER_RTT_printf(0,
								  "USB CTRL: Set Remote WakeUp\n");
#endif
						/* DEVICEREMOTEWAKEUP */
						/* isRemoteWakeUpEnabled = 1; */
						UDD_Send8(ENDPOINT_CTRL, 0);
					} else if (setup.valueL == 0) {
#ifdef USB_DEBUG
						SEGGER_RTT_printf(0,
								  "USB CTRL: Stall\n");
#endif
						/* ENDPOINTHALT */
						USB_stallEndpoint(setup.index);
					} else {
						USB_stallEndpoint(ENDPOINT_CTRL);
					}
				} else if (req == USB_CTRL_STD_Req_SET_ADDRESS) {
					UDD_WaitIN();
					UDD_SetAddress(setup.valueL);
#ifdef USB_DEBUG
					SEGGER_RTT_printf(0,
							  "USB CTRL: Set Address: %d\n",
							  setup.valueL);
#endif
				} else if (req ==
					   USB_CTRL_STD_Req_GET_DESCRIPTOR) {
#ifdef USB_DEBUG
					SEGGER_RTT_printf(0,
							  "USB CTRL: Send Descriptor\n");
#endif
					result = USB_SendDescriptor(&setup);
				} else if (req ==
					   USB_CTRL_STD_Req_SET_DESCRIPTOR) {
#ifdef USB_DEBUG
					SEGGER_RTT_printf(0,
							  "USB CTRL: Set Descriptor, stalled!\n");
#endif
					USB_stallEndpoint(ENDPOINT_CTRL);
				} else if (req ==
					   USB_CTRL_STD_Req_GET_CONFIGURATION) {
#ifdef USB_DEBUG
					SEGGER_RTT_printf(0,
							  "USB CTRL: Get configure\n");
#endif
					UDD_Send8(ENDPOINT_CTRL, usbCfg);
				} else if (req ==
					   USB_CTRL_STD_Req_SET_CONFIGURATION) {
#ifdef USB_DEBUG
					SEGGER_RTT_printf(0,
							  "USB CTRL: Set configure\n");
#endif
					if ((requestType &
					     USB_CTRL_REQUEST_RCPT_MASK) ==
					    USB_CTRL_REQUEST_RCPT_DEVICE) {
						UDD_InitEndpoints(EndPoints,
								  (sizeof
								   (EndPoints) /
								   sizeof
								   (EndPoints
								    [0])));
						usbCfg = setup.valueL;

					} else {
#ifdef USB_DEBUG
						SEGGER_RTT_printf(0,
								  "USB CTRL: Config failed, stalled!\n");
#endif
						USB_stallEndpoint(ENDPOINT_CTRL);
					}
				} else if (req ==
					   USB_CTRL_STD_Req_GET_INTERFACE) {
#ifdef USB_DEBUG
					SEGGER_RTT_printf(0,
							  "USB CTRL: Get Interface\n");
#endif
					UDD_Send8(ENDPOINT_CTRL, usbInterface);
				} else if (req ==
					   USB_CTRL_STD_Req_SET_INTERFACE) {
					usbInterface = setup.valueL;
#ifdef USB_DEBUG
					SEGGER_RTT_printf(0,
							  "USB CTRL: Set interface: %d\n",
							  setup.valueL);
#endif
				}
			} else {
#ifdef USB_DEBUG
				SEGGER_RTT_printf(0,
						  "USB CTRL: Class Interface Request\n");
#endif
				UDD_WaitIN();

				result = USB488_processCtrl(&setup);
			}

			if (result) {
#ifdef USB_DEBUG
				SEGGER_RTT_printf(0, "USB CTRL: Sent packet\n");
#endif
				UDD_ClearIN();
			} else {
#ifdef USB_DEBUG
				SEGGER_RTT_printf(0, "USB CTRL: Error, stalled!\n");
#endif
				USB_stallEndpoint(ENDPOINT_CTRL);
			}
		}
	} else if (Is_udd_endpoint_interrupt(ENDPOINT_BOUT)) {
		udd_ack_out_received(ENDPOINT_BOUT);
	}
	SEGGER_SYSVIEW_RecordExitISR();
}
