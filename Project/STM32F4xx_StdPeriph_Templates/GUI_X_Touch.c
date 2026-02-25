/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUI_TOUCH_X.C
Purpose     : Config / System dependent externals for GUI
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "touch.h"

void GUI_TOUCH_X_ActivateX(void) 
{
}

void GUI_TOUCH_X_ActivateY(void) 
{
}


#include "main.h"
uint16_t tcx=0;
uint16_t tcy=0;

int  GUI_TOUCH_X_MeasureX(void) 
{
   tcx=ADS_Read_XY(CMD_RDX);
  //tcx=ADS_Read_XY(CMD_RDY);
  return tcx;
}

int  GUI_TOUCH_X_MeasureY(void) 
{
  tcy=ADS_Read_XY(CMD_RDY);
 //  tcy=4095-ADS_Read_XY(CMD_RDX);
  return tcy;
}


