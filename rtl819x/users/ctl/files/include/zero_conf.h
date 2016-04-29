/*
 *      Include file of zero_conf.c
 *
 *      Authors: Jerome Zhao 	<jzhao@actiontec.com>
 *
 *      This file is for zero config
 *
 */


#ifndef __ZERO_CONF_H__
#define __ZERO_CONF_H__

/*Interval seconds between retry*/
#ifdef AEI_TELUS_ZERO_CONF
#define INTERVAL    30
#define MAX_TRY_COUNTER 3
#else
#define INTERVAL    300
#endif

/*Define for Auto Sync Wireless settings*/
#define XML_FILE    "/tmp/get_parameters.xml"

#ifdef AEI_TELUS_ZERO_CONF
#define XML_INPUT_FILE "wirelesssettings.xml"
#define SUCCESS_FILE "wirelesssync_successful.xml"
#define FAIL_FILE "wirelesssync_Failure.xml"

/* Define for upgrade*/
#define UPGRADE_INPUT_XML "firmwarelinks.xml"
#define UPGRADE_LOCAL_XML "/tmp/upgrade_url.xml"
#define UPGRADE_DOWNLOAD "/tmp/upgrade.bin"

/* Product type*/
#define PRODUCT_TYPE "WCB3000"
/*Vendor ID, WP used this to identify that is a actiontec peripheral*/
#define VENDOR_ID "ACTIONTEC_WP"
/*Protocal version*/
#define PROTOCOL_VERSION "1.0"
#endif

#endif
