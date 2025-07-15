/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

typedef struct _TDNF_PLUGIN_
{
    char *pszName;
    int nEnabled;
    void *pModule;
    PTDNF_PLUGIN_HANDLE pHandle;
    TDNF_PLUGIN_EVENT RegisterdEvts;
    TDNF_PLUGIN_INTERFACE stInterface;
    struct _TDNF_PLUGIN_ *pNext;
} TDNF_PLUGIN, *PTDNF_PLUGIN;

typedef struct _TDNF_
{
    PSolvSack pSack;
    PTDNF_CMD_ARGS pArgs;
    PTDNF_CONF pConf;
    PTDNF_REPO_DATA pRepos;
    Repo *pSolvCmdLineRepo;
    PTDNF_PLUGIN pPlugins;
} TDNF;

typedef struct _TDNF_CACHED_RPM_ENTRY
{
    char* pszFilePath;
    struct _TDNF_CACHED_RPM_ENTRY *pNext;
} TDNF_CACHED_RPM_ENTRY, *PTDNF_CACHED_RPM_ENTRY;

typedef struct _TDNF_CACHED_RPM_LIST
{
    int nSize;
    PTDNF_CACHED_RPM_ENTRY pHead;
} TDNF_CACHED_RPM_LIST, *PTDNF_CACHED_RPM_LIST;

typedef struct progress_cb_data {
    time_t cur_time;
    time_t prev_time;
    curl_off_t last_bytes;
    time_t last_time;
    double ema_speed;
    char pszData[64];
    int row;
    curl_off_t *pBytesTotal;
    curl_off_t *pBytesDone;
} pcb_data;

typedef struct _TDNF_RPM_TS_
{
    int                     nQuiet;
    rpmts                   pTS;
    rpmtransFlags           nTransFlags;
    rpmprobFilterFlags      nProbFilterFlags;
    FD_t                    pFD;
    PTDNF_CACHED_RPM_LIST   pCachedRpmsArray;
    int                     nPkgsTotal;
    int                     nPkgsProcessed;
    int                     nFilesTotal;
    int                     nFilesProcessed;
    rpm_loff_t              nBytesTotal;
    rpm_loff_t              nBytesProcessed;
    pcb_data                progress;
} TDNFRPMTS, *PTDNFRPMTS;

typedef struct _TDNF_REPO_METADATA
{
    char *pszRepoCacheDir;
    char *pszRepo;
    char *pszRepoMD;
    char *pszPrimary;
    char *pszFileLists;
    char *pszUpdateInfo;
    char *pszOther;
} TDNF_REPO_METADATA,*PTDNF_REPO_METADATA;

typedef struct _TDNF_EVENT_DATA_
{
    union
    {
        int nInt;
        const char *pcszStr;
        const void *pPtr;
    };
    TDNF_EVENT_ITEM_TYPE nType;
    const char *pcszName;
    struct _TDNF_EVENT_DATA_ *pNext;
} TDNF_EVENT_DATA, *PTDNF_EVENT_DATA;
