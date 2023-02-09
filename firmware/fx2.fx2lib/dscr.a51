;; SPDX-License-Identifier: GPL-3.0-or-later
;;
;; This file is part of the fx2adf435xfw project.
;;
;; Copyright (C) 2012 Uwe Hermann <uwe@hermann-uwe.de>
;; Copyright (C) 2016 Stefan Brüns <stefan.bruens@rwth-aachen.de>
;; Copyright (C) 2012-2017 Joel Holdsworth <joel@airwebreathe.org.uk>
;; Copyright (C) 2022, 2023 Martin Homuth-Rosemann
;;
;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, see <http://www.gnu.org/licenses/>.
;;

VID = 0x0456	; Manufacturer ID Analog Devices (0x0456)
PID = 0xb40d	; Product ID EVAL-ADF4351 (0xb40d)
VER = 0x0032	; FW version 00.32

;; store 16 bit word in little endian format
.macro .swapw 	word
	.dw	(<word * 0x100) | >word
.endm

;; store a string as a sequence of char, 0x00, char, 0x00, ...
.macro string_descriptor_a n,str
_string'n:
	.nchr	len,"'str"
	.db	len * 2 + 2
	.db	3
	.irpc	i,^"'str"
		.db	''i, 0
	.endm
.endm

.macro string_descriptor_lang n,lang
_string'n:
	.db	4
	.db	3
	.swapw	lang
.endm

.module DEV_DSCR

; Descriptor types
DSCR_DEVICE_TYPE	= 1
DSCR_CONFIG_TYPE	= 2
DSCR_STRING_TYPE	= 3
DSCR_INTERFACE_TYPE	= 4
DSCR_ENDPOINT_TYPE	= 5
DSCR_DEVQUAL_TYPE	= 6

; Descriptor lengths
DSCR_INTERFACE_LEN	= 9

; Endpoint types
ENDPOINT_TYPE_CONTROL	= 0
ENDPOINT_TYPE_ISO	= 1
ENDPOINT_TYPE_BULK	= 2
ENDPOINT_TYPE_INT	= 3

.globl _dev_dscr, _dev_qual_dscr, _highspd_dscr, _fullspd_dscr
.globl _dev_strings, _serial_num, _dev_strings_end

.area DSCR_AREA (CODE)

; -----------------------------------------------------------------------------
; Device descriptor
; -----------------------------------------------------------------------------
_dev_dscr:
	.db	dev_dscr_end - _dev_dscr
	.db	DSCR_DEVICE_TYPE
	.swapw	0x0200			; USB 2.0
	.db	0 			; Class (defined at interface level)
	.db	0			; Subclass (defined at interface level)
	.db	0			; Protocol (defined at interface level)
	.db	64			; Max. EP0 packet size
	.swapw	VID			; Manufacturer ID
	.swapw 	PID			; Product ID
	.swapw	VER			; Product version
	.db	1			; Manufacturer string index
	.db	2			; Product string index
	.db	3			; Serial number string index (none)
	.db	1			; Number of configurations
dev_dscr_end:

; -----------------------------------------------------------------------------
; Device qualifier (for "other device speed")
; -----------------------------------------------------------------------------
_dev_qual_dscr:
	.db	dev_qualdscr_end - _dev_qual_dscr
	.db	DSCR_DEVQUAL_TYPE
	.swapw	0x0200			; USB 2.0
	.db	0			; Class (vendor specific)
	.db	0			; Subclass (vendor specific)
	.db	0			; Protocol (vendor specific)
	.db	64			; Max. EP0 packet size
	.db	1			; Number of configurations
	.db	0			; Extra reserved byte
dev_qualdscr_end:

; -----------------------------------------------------------------------------
; High-Speed configuration descriptor
; -----------------------------------------------------------------------------
_highspd_dscr:
	.db	highspd_dscr_end - _highspd_dscr
	.db	DSCR_CONFIG_TYPE
	; Total length of the configuration (1st line LSB, 2nd line MSB)
	.db	(highspd_dscr_realend - _highspd_dscr) % 256
	.db	(highspd_dscr_realend - _highspd_dscr) / 256
	.db	1			; Number of interfaces
	.db	1			; Configuration number
	.db	0			; Configuration string (none)
	.db	0xa0			; Attributes (bus powered, remote wakeup)
	.db	100			; Max. power (200mA/2)
highspd_dscr_end:

	; Interfaces (only one in our case)
	.db	DSCR_INTERFACE_LEN
	.db	DSCR_INTERFACE_TYPE
	.db	0			; Interface index
	.db	0			; Alternate setting index
	.db	0			; Number of endpoints
	.db	0xff			; Class (vendor specific)
	.db	0			; Subclass
	.db	0			; Protocol
	.db	0			; String index (none)

highspd_dscr_realend:

	.even

; -----------------------------------------------------------------------------
; Full-Speed configuration descriptor
; -----------------------------------------------------------------------------
_fullspd_dscr:
	.db	fullspd_dscr_end - _fullspd_dscr
	.db	DSCR_CONFIG_TYPE
	; Total length of the configuration (1st line LSB, 2nd line MSB)
	.db	(fullspd_dscr_realend - _fullspd_dscr) % 256
	.db	(fullspd_dscr_realend - _fullspd_dscr) / 256
	.db	1			; Number of interfaces
	.db	1			; Configuration number
	.db	0			; Configuration string (none)
	.db	0xa0			; Attributes (bus powered, remote wakeup)
	.db	100			; Max. power (200mA/2)
fullspd_dscr_end:

	; Interfaces (only one in our case)
	.db	DSCR_INTERFACE_LEN
	.db	DSCR_INTERFACE_TYPE
	.db	0			; Interface index
	.db	0			; Alternate setting index
	.db	0			; Number of endpoints
	.db	0xff			; Class (vendor specific)
	.db	0			; Subclass
	.db	0			; Protocol
	.db	0			; String index (none)

fullspd_dscr_realend:

	.even

; -----------------------------------------------------------------------------
; Strings
; -----------------------------------------------------------------------------

_dev_strings:

; See http://www.usb.org/developers/docs/USB_LANGIDs.pdf for the full list.
string_descriptor_lang 0 0x0409 ; Language code 0x0409 (English, US)

; Vendor string
string_descriptor_a 1,^"ANALOG DEVICES"

; Product string
string_descriptor_a 2,^"EVAL-ADF4351"

; Serial number string
; The template for unique FX2LP id ...
; ... must be 12 byte long, see Cypress KBA212789
_serial_num:
string_descriptor_a 3,^"000000000000"

_dev_strings_end:
	.dw	0x0000
