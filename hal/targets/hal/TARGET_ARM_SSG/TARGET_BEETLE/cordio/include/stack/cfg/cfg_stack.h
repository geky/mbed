/*************************************************************************************************/
/*!
 *  \file   cfg_stack.h
 *
 *  \brief  Stack configuration.
 *
 *          $Date: 2015-06-12 07:19:18 -0400 (Fri, 12 Jun 2015) $
 *          $Revision: 3061 $
 *
 * Copyright (c) 2009-2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: LicenseRef-PBL
 *
 * This file and the related binary are licensed under the
 * Permissive Binary License, Version 1.0 (the "License");
 * you may not use these files except in compliance with the License.
 *
 * You may obtain a copy of the License here:
 * LICENSE-permissive-binary-license-1.0.txt and at
 * https://www.mbed.com/licenses/PBL-1.0
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*************************************************************************************************/
#ifndef CFG_STACK_H
#define CFG_STACK_H


#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  HCI
**************************************************************************************************/

/*! Vendor specific targets */
#define HCI_VS_GENERIC        0
#define HCI_VS_EMM            1

/*! Vendor specific target configuration */
#ifndef HCI_VS_TARGET
#define HCI_VS_TARGET HCI_VS_GENERIC
#endif

/**************************************************************************************************
  DM
**************************************************************************************************/

/*! Maximum number of connections */
#ifndef DM_CONN_MAX
#define DM_CONN_MAX 3
#endif

/**************************************************************************************************
  L2C
**************************************************************************************************/

/*! Maximum number of connection oriented channels */
#ifndef L2C_COC_CHAN_MAX
#define L2C_COC_CHAN_MAX 8
#endif

/*! Maximum number of connection oriented channel registered clients */
#ifndef L2C_COC_REG_MAX
#define L2C_COC_REG_MAX 4
#endif

/**************************************************************************************************
  ATT
**************************************************************************************************/

/**************************************************************************************************
  SMP
**************************************************************************************************/



#ifdef __cplusplus
};
#endif

#endif /* CFG_STACK_H */
