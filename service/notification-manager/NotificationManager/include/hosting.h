//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef _HOSTING_H_
#define _HOSTING_H_

// Standard API
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Iotivity Base CAPI
#include "ocstack.h"
#include "ocsocket.h"
#include "logger.h"


#define HOSTING_TAG  PCF("Hosting")

/**
 *
 * Start resource coordinator
 *
 * @brief create mirrorResourceList and discover coordinatee candidate.
 *
 * @return
 *     OC_STACK_OK               - no errors
 *     OC_STACK_INVALID_CALLBACK - invalid callback function pointer
 *     OC_STACK_INVALID_METHOD   - invalid resource method
 *     OC_STACK_INVALID_URI      - invalid required or reference URI
 *     OC_STACK_INVALID_QUERY    - number of resource types specified for filtering presence
 *                                 notifications exceeds @ref MAX_PRESENCE_FILTERS.
 *     OC_STACK_ERROR            - otherwise error(initialized value)
 */
OCStackResult OICStartCoordinate();

/**
 *
 * stop resource coordinator
 *
 * @brief delete mirrorResourceList used while coordinating
 *
 * @return
 *     OC_STACK_ERROR            - otherwise error(initialized value)
 */
OCStackResult OICStopCoordinate();



#endif
