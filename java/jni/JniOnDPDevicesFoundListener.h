/*
* ******************************************************************
*
*  Copyright 2016 Samsung Electronics All Rights Reserved.
*
* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
#include <jni.h>
#include "OCDirectPairing.h"
#include "OCApi.h"
#include "JniOcStack.h"

#ifndef _Included_org_iotivity_base_OcPlatform_JniOnDPDevicesFoundListener
#define _Included_org_iotivity_base_OcPlatform_JniOnDPDevicesFoundListener

enum class DPFunc
{
    FIND_DIRECT_PAIRED_DEV_LIST = 1,
    GET_PAIRED_DEV_LIST,
};

class JniOnDPDevicesFoundListener
{
public:
    JniOnDPDevicesFoundListener(JNIEnv *env, jobject jListener,
            RemoveListenerCallback removeListenerCallback);
    ~JniOnDPDevicesFoundListener();

   jobject convertdpDevVectorToJavaList(JNIEnv *env, OC::PairedDevices DPdevList);
   void directPairingDevicesCallback(OC::PairedDevices paringDevicesList, DPFunc);

private:
    RemoveListenerCallback m_removeListenerCallback;
    jweak m_jwListener;
    void checkExAndRemoveListener(JNIEnv* env);
};
#endif
