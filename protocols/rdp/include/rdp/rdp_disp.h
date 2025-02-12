/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef GUAC_RDP_DISP_H
#define GUAC_RDP_DISP_H

#include <guacamole/config.h>

#include <rdp/dvc.h>
#include <rdp/rdp_settings.h>

#include <freerdp/freerdp.h>

#ifdef HAVE_FREERDP_CLIENT_DISP_H
#include <freerdp/client/disp.h>
#endif

/**
 * The minimum value for width or height, in pixels.
 */
#define GUAC_RDP_DISP_MIN_SIZE 200

/**
 * The maximum value for width or height, in pixels.
 */
#define GUAC_RDP_DISP_MAX_SIZE 8192

/**
 * The minimum amount of time that must elapse between display size updates,
 * in milliseconds.
 */
#define GUAC_RDP_DISP_UPDATE_INTERVAL 500

/**
 * Display size update module.
 */
typedef struct guac_rdp_disp {

#ifdef HAVE_FREERDP_DISPLAY_UPDATE_SUPPORT
    /**
     * Display control interface.
     */
    DispClientContext* disp;
#endif

    /**
     * The timestamp of the last display update request, or 0 if no request
     * has been sent yet.
     */
    guac_timestamp last_request;

    /**
     * The last requested screen width, in pixels.
     */
    int requested_width;

    /**
     * The last requested screen height, in pixels.
     */
    int requested_height;

    /**
     * Whether the size has changed and the RDP connection must be closed and
     * reestablished.
     */
    int reconnect_needed;

} guac_rdp_disp;

/**
 * Allocates a new display update module, which will ultimately control the
 * display update channel once conected.
 *
 * @return A new display update module.
 */
guac_rdp_disp* guac_rdp_disp_alloc();

/**
 * Frees the given display update module.
 *
 * @param disp The display update module to free.
 */
void guac_rdp_disp_free(guac_rdp_disp* disp);

/**
 * @param context The rdpContext associated with the active RDP session.
 */
/**
 * Adds FreeRDP's "disp" plugin to the list of dynamic virtual channel plugins
 * to be loaded by FreeRDP's "drdynvc" plugin. The plugin will only be loaded
 * once guac_rdp_load_drdynvc() is invoked with the guac_rdp_dvc_list passed to
 * this function. The "disp" plugin ultimately adds support for the Display
 * Update channel. NOTE: It is still up to external code to detect when the
 * "disp" channel is connected, and update the guac_rdp_disp with a call to
 * guac_rdp_disp_connect().
 *
 * @param context
 *     The rdpContext associated with the active RDP session.
 *
 * @param list
 *     The guac_rdp_dvc_list to which the "disp" plugin should be added, such
 *     that it may later be loaded by guac_rdp_load_drdynvc().
 */
void guac_rdp_disp_load_plugin(rdpContext* context, guac_rdp_dvc_list* list);

#ifdef HAVE_FREERDP_DISPLAY_UPDATE_SUPPORT
/**
 * Stores the given DispClientContext within the given guac_rdp_disp, such that
 * display updates can be properly sent. Until this is called, changes to the
 * display size will be deferred.
 *
 * @param guac_disp The display update module to associate with the connected
 *                  display update channel.
 * @param disp The DispClientContext associated by FreeRDP with the connected
 *             display update channel.
 */
void guac_rdp_disp_connect(guac_rdp_disp* guac_disp, DispClientContext* disp);
#endif

/**
 * Requests a display size update, which may then be sent immediately to the
 * RDP server. If an update was recently sent, this update may be delayed until
 * the RDP server has had time to settle. The width/height values provided may
 * be automatically altered to comply with the restrictions imposed by the
 * display update channel.
 *
 * @param disp
 *     The display update module which should maintain the requested size,
 *     sending the corresponding display update request when appropriate.
 *
 * @param settings
 *     The RDP client settings associated with the current or pending RDP
 *     session. These settings will be automatically adjusted to match the new
 *     screen size.
 *
 * @param rdp_inst
 *     The FreeRDP instance associated with the current or pending RDP session,
 *     if any. If no RDP session is active, this should be NULL.
 *
 * @param width
 *     The desired display width, in pixels. Due to the restrictions of the RDP
 *     display update channel, this will be contrained to the range of 200
 *     through 8192 inclusive, and rounded down to the nearest even number.
 *
 * @param height
 *     The desired display height, in pixels. Due to the restrictions of the
 *     RDP display update channel, this will be contrained to the range of 200
 *     through 8192 inclusive.
 */
void guac_rdp_disp_set_size(guac_rdp_disp* disp, guac_rdp_settings* settings,
        freerdp* rdp_inst, int width, int height);

/**
 * Sends an actual display update request to the RDP server based on previous
 * calls to guac_rdp_disp_set_size(). If an update was recently sent, the
 * update may be delayed until a future call to this function. If the RDP
 * session has not yet been established, the request will be delayed until the
 * session exists.
 *
 * @param disp
 *     The display update module which should track the update request.
 *
 * @param settings
 *     The RDP client settings associated with the current or pending RDP
 *     session. These settings will be automatically adjusted to match the new
 *     screen size.
 *
 * @param rdp_inst
 *     The FreeRDP instance associated with the current or pending RDP session,
 *     if any. If no RDP session is active, this should be NULL.
 */
void guac_rdp_disp_update_size(guac_rdp_disp* disp,
        guac_rdp_settings* settings, freerdp* rdp_inst);

/**
 * Signals the given display update module that the requested reconnect has
 * been performed.
 *
 * @param disp
 *     The display update module that should be signaled regarding the state
 *     of reconnection.
 */
void guac_rdp_disp_reconnect_complete(guac_rdp_disp* disp);

/**
 * Returns whether a full RDP reconnect is required for display update changes
 * to take effect.
 *
 * @return
 *     Non-zero if a reconnect is needed, zero otherwise.
 */
int guac_rdp_disp_reconnect_needed(guac_rdp_disp* disp);

#endif

