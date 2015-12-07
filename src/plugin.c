/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - plugin.c                                        *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2014 Bobby Smiles                                       *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "common.h"
#include "hle.h"
#include "hle_internal.h"
#include "Rsp.h"

#if defined(_WIN32)
#include "./win/win.h"
#include "./win/resource.h"
#else
  #if defined(USE_GTK)
#include <gtk/gtk.h>
  #endif
#endif

#define RSP_HLE_VERSION        0x020500
#define RSP_PLUGIN_API_VERSION 0x020000

bool AudioHle = FALSE, GraphicsHle = TRUE;

/* local variables */
static struct hle_t g_hle;
static void (*l_CheckInterrupts)(void) = NULL;
static void (*l_ProcessDlistList)(void) = NULL;
static void (*l_ProcessAlistList)(void) = NULL;
static void (*l_ProcessRdpList)(void) = NULL;
static void (*l_ShowCFB)(void) = NULL;
static void (*l_DebugCallback)(void *, int, const char *) = NULL;
static void *l_DebugCallContext = NULL;

/* Global functions needed by HLE core */
void HleVerboseMessage(void* UNUSED(user_defined), const char *message, ...)
{
#if defined(_WIN32) && defined(_DEBUG)
	// These can get annoying. 
 #if 0
	MessageBox(NULL, message, "HLE Verbose Message", MB_OK);
 #endif
#endif
}

void HleErrorMessage(void* UNUSED(user_defined), const char *message, ...)
{
#if defined(_WIN32)
	MessageBox(NULL, message, "HLE Error Message", MB_OK);
#endif
}

void HleWarnMessage(void* UNUSED(user_defined), const char *message, ...)
{
#if defined(_WIN32) && defined(_DEBUG)
	MessageBox(NULL, message, "HLE Warning Message", MB_OK);
#endif
}

void HleCheckInterrupts(void* UNUSED(user_defined))
{
    if (l_CheckInterrupts == NULL)
        return;

    (*l_CheckInterrupts)();
}

void HleProcessDlistList(void* UNUSED(user_defined))
{
    if (l_ProcessDlistList == NULL)
        return;

    (*l_ProcessDlistList)();
}

void HleProcessAlistList(void* UNUSED(user_defined))
{
    if (l_ProcessAlistList == NULL)
        return;

    (*l_ProcessAlistList)();
}

void HleProcessRdpList(void* UNUSED(user_defined))
{
    if (l_ProcessRdpList == NULL)
        return;

    (*l_ProcessRdpList)();
}

void HleShowCFB(void* UNUSED(user_defined))
{
    if (l_ShowCFB == NULL)
        return;

    (*l_ShowCFB)();
}


/* DLL-exported functions */
EXPORT unsigned long CALL DoRspCycles(unsigned long Cycles)
{
    hle_execute(&g_hle);
    return Cycles;
}

EXPORT void CALL InitiateRSP(RSP_INFO Rsp_Info, unsigned int* UNUSED(CycleCount))
{
    hle_init(&g_hle,
             Rsp_Info.RDRAM,
             Rsp_Info.DMEM,
             Rsp_Info.IMEM,
             Rsp_Info.MI_INTR_REG,
             Rsp_Info.SP_MEM_ADDR_REG,
             Rsp_Info.SP_DRAM_ADDR_REG,
             Rsp_Info.SP_RD_LEN_REG,
             Rsp_Info.SP_WR_LEN_REG,
             Rsp_Info.SP_STATUS_REG,
             Rsp_Info.SP_DMA_FULL_REG,
             Rsp_Info.SP_DMA_BUSY_REG,
             Rsp_Info.SP_PC_REG,
             Rsp_Info.SP_SEMAPHORE_REG,
             Rsp_Info.DPC_START_REG,
             Rsp_Info.DPC_END_REG,
             Rsp_Info.DPC_CURRENT_REG,
             Rsp_Info.DPC_STATUS_REG,
             Rsp_Info.DPC_CLOCK_REG,
             Rsp_Info.DPC_BUFBUSY_REG,
             Rsp_Info.DPC_PIPEBUSY_REG,
             Rsp_Info.DPC_TMEM_REG,
             NULL);

    l_CheckInterrupts = Rsp_Info.CheckInterrupts;
    l_ProcessDlistList = Rsp_Info.ProcessDList;
    l_ProcessAlistList = Rsp_Info.ProcessAList;
    l_ProcessRdpList = Rsp_Info.ProcessRdpList;
    l_ShowCFB = Rsp_Info.ShowCFB;
}

EXPORT void CALL CloseDLL(void)
{
    /* do nothing */
}

EXPORT void CALL GetDllInfo(PLUGIN_INFO * PluginInfo)
{
    PluginInfo->Version = 0x0101;
	PluginInfo->Type = PLUGIN_TYPE_RSP;
	strcpy(PluginInfo->Name, "Mupen64Plus HLE RSP Plugin");
	PluginInfo->NormalMemory = 1;
	PluginInfo->MemoryBswaped = 1;
}

EXPORT void CALL DllConfig(HWND hParent)
{
#if defined(_WIN32)
		DialogBox(dll_hInstance,
			MAKEINTRESOURCE(IDD_RSPCONFIG), hParent, ConfigDlgProc);
#endif
}

EXPORT void CALL DllAbout(HWND hParent)
{
#if defined(_WIN32)
	MessageBox(NULL, "Mupen64Plus HLE RSP plugin v2.5 for Zilmar Spec Emulators", "M64P RSP HLE", MB_OK);
#else
 #if defined(USE_GTK)
	char tMsg[256];
	GtkWidget *dialog, *label, *okay_button;

	dialog = gtk_dialog_new();
	sprintf(tMsg, "Mupen64Plus HLE RSP plugin v2.5 for Zilmar Spec Emulators");
	label = gtk_label_new(tMsg);
	okay_button = gtk_button_new_with_label("OK");

	gtk_signal_connect_object(GTK_OBJECT(okay_button), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy),
		GTK_OBJECT(dialog));
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
		okay_button);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
		label);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_widget_show_all(dialog);
 #else
	char tMsg[256];
	sprintf(tMsg, "Mupen64Plus HLE RSP plugin v2.5 for Zilmar Spec Emulators");
	fprintf(stderr, "About\n%s\n", tMsg);
 #endif
#endif
}

EXPORT void CALL DllTest(HWND hParent)
{
#if defined(_WIN32)
	MessageBox(NULL, "No Test For You.", "No Test", MB_OK);
#endif
}

EXPORT void CALL RomClosed(void)
{
    /* do nothing */
}
