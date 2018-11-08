/*
**             Copyright 2017 by Kvaser AB, Molndal, Sweden
**                         http://www.kvaser.com
**
** This software is dual licensed under the following two licenses:
** BSD-new and GPLv2. You may use either one. See the included
** COPYING file for details.
**
** License: BSD-new
** ==============================================================================
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the <organization> nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
** BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
** IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
**
** License: GPLv2
** ==============================================================================
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
**
**
** IMPORTANT NOTICE:
** ==============================================================================
** This source code is made available for free, as an open license, by Kvaser AB,
** for use with its applications. Kvaser AB does not accept any liability
** whatsoever for any third party patent or other immaterial property rights
** violations that may result from any usage of this source code, regardless of
** the combination of source code and various applications that it can be used
** in, or with.
**
** -----------------------------------------------------------------------------
*/

/* Kvaser Linux Canlib VCan layer functions used in Scripts */

#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

#include "VCanScriptFunctions.h"
#include "VCanFuncUtil.h"
#include "kcanio_script.h"
#include "canstat.h"


#if DEBUG
#   define DEBUGPRINT(args) printf args
#else
#   define DEBUGPRINT(args)
#endif

/***************************************************************************/
static canStatus scriptControlStatusToCanStatus(unsigned int script_control_status)
{
  canStatus stat;

  switch (script_control_status) {
    case KCANIO_SCRIPT_CTRL_ERR_SUCCESS:
      stat = canOK;
      break;

    case KCANIO_SCRIPT_CTRL_ERR_NO_MORE_PROCESSES:     /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_OUT_OF_CODE_MEM:       /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_OPEN_FILE_NO_MEM:      /* fall through */
      stat = canERR_NOMEM;
      break;

    case KCANIO_SCRIPT_CTRL_ERR_FILE_NOT_FOUND:        /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_OPEN_FILE_ERR:         /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_FILE_READ_ERR:         /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_LOAD_FILE_ERR:         /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_FILE_REWIND_FAIL:      /* fall through */
      stat = canERR_DEVICE_FILE;
      break;

    case KCANIO_SCRIPT_CTRL_ERR_SPI_BUSY:
      stat = canERR_HARDWARE;
      break;

    case KCANIO_SCRIPT_CTRL_ERR_UNKNOWN_COMMAND:       /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_NOT_IMPLEMENTED:
      stat = canERR_NOT_IMPLEMENTED;
      break;

    case KCANIO_SCRIPT_CTRL_ERR_COMPILER_VERSION:
      stat = canERR_SCRIPT_WRONG_VERSION;
      break;

    case KCANIO_SCRIPT_CTRL_ERR_LOAD_FAIL:             /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_SETUP_FAIL:            /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_SETUP_FUN_TABLE_FAIL:  /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_SETUP_PARAMS_FAIL:     /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_PROCESSES_NOT_FOUND:   /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_START_FAILED:          /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_STOP_FAILED:           /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_PROCESS_NOT_STOPPED:   /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_PROCESS_NOT_RUNNING:   /* fall through */
    case KCANIO_SCRIPT_CTRL_ERR_ENVVAR_NOT_FOUND:      /* fall through */
    default:
      stat = canERR_SCRIPT_FAIL;
      break;
  }
  return stat;
}

//======================================================================
// vCanScript_stop
//======================================================================
canStatus vCanScript_stop(HandleData *hData, int slotNo, int mode)
{
  int ret;
  KCAN_IOCTL_SCRIPT_CONTROL_T script_control;

  memset(&script_control, 0, sizeof(script_control));
  script_control.scriptNo = slotNo;
  script_control.command = CMD_SCRIPT_STOP;
  script_control.stopMode = (signed char) mode;
  script_control.channel = hData->channelNr;
  ret = ioctl(hData->fd, KCAN_IOCTL_SCRIPT_CONTROL, &script_control);
  if (ret != 0) {
    return errnoToCanStatus(errno);
  }
  return scriptControlStatusToCanStatus(script_control.script_control_status);
}

//======================================================================
// vCanScript_start
//======================================================================
canStatus vCanScript_start(HandleData *hData, int slotNo)
{
  int ret;
  KCAN_IOCTL_SCRIPT_CONTROL_T script_control;

  memset(&script_control, 0, sizeof(script_control));
  script_control.scriptNo = slotNo;
  script_control.command = CMD_SCRIPT_START;
  script_control.channel = hData->channelNr;
  ret = ioctl(hData->fd, KCAN_IOCTL_SCRIPT_CONTROL, &script_control);
  if (ret != 0) {
    return errnoToCanStatus(errno);
  }
  return scriptControlStatusToCanStatus(script_control.script_control_status);
}

//======================================================================
// vCanScript_load_file
// Load a compiled script file directly into the script engine on the device.
//======================================================================
canStatus vCanScript_load_file(HandleData *hData, int slotNo,
                               char *hostFileName)
{
  int ret;
  canStatus status;
  size_t bytes;
  FILE *hFile;
  KCAN_IOCTL_SCRIPT_CONTROL_T script_control;
  const size_t current_block_size = sizeof(script_control.data) - 1;

  hFile = fopen(hostFileName, "rb");
  if (!hFile) {
    DEBUGPRINT((TXT("Could not open script file '%s'\n"), hostFileName));
    return canERR_HOST_FILE;
  }

  // Start transfer of data; setup slot for script
  memset(&script_control, 0, sizeof(script_control));
  script_control.scriptNo = slotNo;
  script_control.channel = hData->channelNr;
  script_control.command = CMD_SCRIPT_LOAD_REMOTE_START;
  ret = ioctl(hData->fd, KCAN_IOCTL_SCRIPT_CONTROL, &script_control);
  if (ret != 0) {
    fclose(hFile);
    return errnoToCanStatus(errno);
  }
  status = scriptControlStatusToCanStatus(script_control.script_control_status);
  if (status != canOK) {
    fclose(hFile);
    return status;
  }

  script_control.command = CMD_SCRIPT_LOAD_REMOTE_DATA;
  do {
    bytes = fread(script_control.script.data, 1, current_block_size, hFile);
    script_control.script.length = bytes;
    ret = ioctl(hData->fd, KCAN_IOCTL_SCRIPT_CONTROL, &script_control);
    if (ret != 0) {
      fclose(hFile);
      return errnoToCanStatus(errno);
    }
    status = scriptControlStatusToCanStatus(script_control.script_control_status);
    if (status != canOK) {
      fclose(hFile);
      return status;
    }
  } while (bytes == current_block_size);
  fclose(hFile);

  // Finish
  script_control.command = CMD_SCRIPT_LOAD_REMOTE_FINISH;
  script_control.script.length = 0;
  ret = ioctl(hData->fd, KCAN_IOCTL_SCRIPT_CONTROL, &script_control);
  if (ret != 0) {
    return errnoToCanStatus(errno);
  }

  status = scriptControlStatusToCanStatus(script_control.script_control_status);
  if (status != canOK) {
    return status;
  }
  return canOK;
}

//======================================================================
// vCanScript_unload
//======================================================================
canStatus vCanScript_unload(HandleData *hData, int slotNo)
{
  int ret;
  KCAN_IOCTL_SCRIPT_CONTROL_T script_control;

  memset(&script_control, 0, sizeof(script_control));
  script_control.scriptNo = slotNo;
  script_control.command = CMD_SCRIPT_UNLOAD;
  script_control.channel = hData->channelNr;
  ret = ioctl(hData->fd, KCAN_IOCTL_SCRIPT_CONTROL, &script_control);
  if (ret != 0) {
    return errnoToCanStatus(errno);
  }
  return scriptControlStatusToCanStatus(script_control.script_control_status);
}
