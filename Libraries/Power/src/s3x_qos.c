/*==========================================================
 * Copyright 2020 QuickLogic Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *==========================================================*/

/*==========================================================
 *                                                          
 *-  Copyright Notice  -------------------------------------
 *                                                          
 *    Licensed Materials - Property of QuickLogic Corp.     
 *    Copyright (C) 2019 QuickLogic Corporation             
 *    All rights reserved                                   
 *    Use, duplication, or disclosure restricted            
 *                                                          
 *    File   : s3x_qos.c
 *    Purpose: 
 *                                                          
 *=========================================================*/

#include "Fw_global_config.h"
#include "eoss3_dev.h"
#include "s3x_clock.h"
#include "s3x_dfs.h"
#include "s3x_err_base.h"

Qos_node *qos_hnd = NULL;

void S3x_set_hsosc_qos_req(UINT32_t val)
{
    s3x_clkd_set_HSOSC_qos_rate(val);
}

void S3x_clear_hsosc_qos_req(void)
{
   s3x_clkd_set_HSOSC_qos_rate(0);
}

UINT32_t S3x_get_qos_rate(QOS_REQ_TYPE req)
{
    Qos_node *temp = qos_hnd;
    UINT32_t qos_mrate = 0, rate;
	if (req > OP_REQ_END)
		return qos_mrate;
    while (temp)
    {
         if (temp->active_req & req)
         {
             rate = temp->rate[GET_QOS_INDEX(req)];
           if (rate > qos_mrate)
                qos_mrate = rate;
         }
         temp = temp->next;
    }
    return qos_mrate;
}

void  S3x_add_qos_node (Qos_node *node)
{
    if (!qos_hnd)
    {
        qos_hnd = node;
    }
    else
    {
        Qos_node *temp = qos_hnd;
        while (temp->next){
            temp =  temp->next;
        }
        temp->next = node;
    }

}

int  S3x_register_clkd_qnode(S3x_ClkD *clkd)
{

    int ret = 0;
    // Define this macro to disable QoS, dfs and lpm.
#if (CONST_FREQ == 0)
    Qos_node *qnode;
    /*only one QoS node for each clock domain*/
    if (!clkd->qos)
    {
      qnode = pvPortMalloc(sizeof(Qos_node));
      if (qnode != NULL)
      {
        qnode->next = NULL;
          S3x_add_qos_node(qnode);
          clkd->qos = qnode;
          qnode->clkd = clkd;
      }
      else
          ret = -ENO_MEM;
    }
#endif
    return ret;

}

int S3x_set_clkd_qos_req(S3x_ClkD *clkd, QOS_REQ_TYPE req, UINT32_t val)
{
    UINT8_t index;
    Qos_node *qnode;

    if (!clkd->qos)
        return -EINVALID_PTR;
    else if (req > OP_REQ_END)
        return -EINVALID_VAL;

    else
        qnode = (Qos_node *)clkd->qos;
    index =  GET_QOS_INDEX(req);
    if (val)
    {
      if (!(qnode->active_req & req))
      {
            qnode->active_req |= req;
            qnode->req_cnt++;
      }
      qnode->rate[index] = val;

      if (req == MIN_HSOSC_FREQ)
          S3x_set_hsosc_qos_req(val);
      else if ((req == MIN_OP_FREQ) &&
                clkd->curr_rate < val)
           _s3x_clkd_srate(clkd, val);
      else if (req == MIN_CPU_FREQ)
          _S3x_clkd_set_cpu_qos_rate(clkd, val);

    }
    else
    {
      if (qnode->active_req & req)
      {
        qnode->active_req &= (~req);
        qnode->rate[index] = val;
        qnode->req_cnt--;
        if (req == MIN_HSOSC_FREQ)
          S3x_clear_hsosc_qos_req();
        //else if (req == MIN_CPU_FREQ)
          //_S3x_clkd_set_cpu_qos_rate(clkd, val);
      }
    }

    return STATUS_OK;
}

int S3x_get_clkd_qos_req(S3x_ClkD *clkd, QOS_REQ_TYPE req)
{
    UINT8_t index;
    int ret = 0;
    Qos_node *qnode;

    if (!clkd->qos)
        goto exit;
    else if (req > OP_REQ_END)
        goto exit;

    else
        qnode = (Qos_node *)clkd->qos;
    index =  GET_QOS_INDEX(req);

    if (qnode->active_req & req)
        ret = qnode->rate[index];
exit:
    return ret;
}
