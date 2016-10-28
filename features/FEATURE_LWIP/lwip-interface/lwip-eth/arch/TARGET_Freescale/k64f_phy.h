/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef K64F_PHY_H
#define K64F_PHY_H

#include "fsl_phy.h"
#include "lwip/err.h"
#include "k64f_enet_config.h"


/** \brief  Low level init of the PHY
 *
 *  \param[in]      netif  Pointer to LWIP netif structure
 *  \return         ERR_OK if low level initialization was successful
 */
err_t k64f_phy_init(uint32_t *sys_clock, phy_speed_t *phy_speed, phy_duplex_t *phy_duplex);

/** \brief  Get the current status of the PHY
 *
 *  \return         true if the link is up
 */
bool k64f_phy_link_status(void);

/** \brief  Periodic PHY task
 *
 *  Must be called periodically by the enet driver at PHY_TASK_PERIOD_MS milliseconds
 *
 *  \param[in]      param  Optional argument to pass to the netif_set_link_up and
 *                  netif_set_link_down functions called on phy updates
 */
void k64f_phy_task(void *param);


#endif

