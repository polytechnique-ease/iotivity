//******************************************************************
//
// Copyright 2016 Samsung Electronics All Rights Reserved.
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

#include "gtest/gtest.h"
#include "time.h"

// Test function hooks
#define CAcloseSslConnection CAcloseSslConnectionTest
#define CAdecryptSsl CAdecryptSslTest
#define CAdeinitSslAdapter CAdeinitSslAdapterTest
#define CAencryptSsl CAencryptSslTest
#define CAinitSslAdapter CAinitSslAdapterTest
#define CAinitiateSslHandshake CAinitiateSslHandshakeTest
#define CAsetCredentialTypesCallback CAsetCredentialTypesCallbackTest
#define CAsetSslAdapterCallbacks CAsetSslAdapterCallbacksTest
#define CAsetSslHandshakeCallback CAsetSslHandshakeCallbackTest
#define CAsetTlsCipherSuite CAsetTlsCipherSuiteTest
#define CAsslGenerateOwnerPsk CAsslGenerateOwnerPskTest
#define CAcloseSslConnectionAll CAcloseSslConnectionAllTest
#ifdef MULTIPLE_OWNER
#define GetCASecureEndpointData GetCASecureEndpointDataTest
#endif

#include "../src/adapter_util/ca_adapter_net_ssl.c"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#if defined(_WIN32)
#include "../../../../c_common/windows/include/pthread_create.h"
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
/** @todo stop-gap for naming issue. Windows.h does not like us to use ERROR */
#ifdef ERROR
#undef ERROR
#endif //ERROR
#endif //HAVE_WINDOWS_H
#include "platform_features.h"
#include "logger.h"

#define MBED_TLS_DEBUG_LEVEL (4) // Verbose

#define SEED "PREDICTED_SEED"
#define dummyHandler 0xF123

#define SERVER_PORT 4433
#define SERVER_NAME "localhost"
#define GET_REQUEST "GET / HTTP/1.0\r\n\r\n"

/* **************************
 *
 *
 * Common routines
 *
 *
 * *************************/

unsigned char serverCert[] = {
    0x30, 0x82, 0x02, 0x39, 0x30, 0x82, 0x01, 0xdf, 0x02, 0x01, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x7c, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
    0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c,
    0x09, 0x53, 0x6f, 0x6d, 0x65, 0x73, 0x74, 0x61, 0x74, 0x65, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03,
    0x55, 0x04, 0x07, 0x0c, 0x08, 0x53, 0x6f, 0x6d, 0x65, 0x63, 0x69, 0x74, 0x79, 0x31, 0x0b, 0x30,
    0x09, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x02, 0x42, 0x42, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03,
    0x55, 0x04, 0x0b, 0x0c, 0x0d, 0x53, 0x65, 0x71, 0x75, 0x72, 0x69, 0x74, 0x79, 0x20, 0x50, 0x61,
    0x72, 0x74, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x02, 0x6f, 0x62, 0x31,
    0x14, 0x30, 0x12, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01, 0x16, 0x05,
    0x6f, 0x62, 0x40, 0x62, 0x62, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x36, 0x30, 0x38, 0x31, 0x35, 0x31,
    0x33, 0x31, 0x31, 0x31, 0x37, 0x5a, 0x17, 0x0d, 0x31, 0x39, 0x30, 0x35, 0x31, 0x32, 0x31, 0x33,
    0x31, 0x31, 0x31, 0x37, 0x5a, 0x30, 0x81, 0xd4, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
    0x06, 0x13, 0x02, 0x55, 0x41, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x03,
    0x41, 0x73, 0x64, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x06, 0x47, 0x6f,
    0x74, 0x68, 0x61, 0x6d, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x02, 0x5a,
    0x5a, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x08, 0x42, 0x65, 0x61, 0x6d,
    0x54, 0x65, 0x61, 0x6d, 0x31, 0x1c, 0x30, 0x1a, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
    0x01, 0x09, 0x01, 0x16, 0x0d, 0x72, 0x61, 0x69, 0x6c, 0x40, 0x6d, 0x61, 0x69, 0x6c, 0x2e, 0x63,
    0x6f, 0x6d, 0x31, 0x32, 0x30, 0x30, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x29, 0x75, 0x75, 0x69,
    0x64, 0x3a, 0x33, 0x32, 0x33, 0x32, 0x33, 0x32, 0x33, 0x32, 0x2d, 0x33, 0x32, 0x33, 0x32, 0x2d,
    0x33, 0x32, 0x33, 0x32, 0x2d, 0x33, 0x32, 0x33, 0x32, 0x2d, 0x33, 0x32, 0x33, 0x32, 0x33, 0x32,
    0x33, 0x32, 0x33, 0x32, 0x33, 0x32, 0x31, 0x34, 0x30, 0x32, 0x06, 0x03, 0x55, 0x1d, 0x11, 0x0c,
    0x2b, 0x75, 0x73, 0x65, 0x72, 0x69, 0x64, 0x3a, 0x36, 0x37, 0x36, 0x37, 0x36, 0x37, 0x36, 0x37,
    0x2d, 0x36, 0x37, 0x36, 0x37, 0x2d, 0x36, 0x37, 0x36, 0x37, 0x2d, 0x36, 0x37, 0x36, 0x37, 0x2d,
    0x36, 0x37, 0x36, 0x37, 0x36, 0x37, 0x36, 0x37, 0x36, 0x37, 0x36, 0x37, 0x30, 0x59, 0x30, 0x13,
    0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
    0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xf7, 0x13, 0x5c, 0x73, 0x72, 0xce, 0x10, 0xe5, 0x09,
    0x97, 0x9a, 0xf8, 0xf2, 0x70, 0xa6, 0x3d, 0x89, 0xf5, 0xc5, 0xe4, 0x44, 0xe2, 0x4a, 0xb6, 0x61,
    0xa8, 0x12, 0x8d, 0xb4, 0xdc, 0x2b, 0x47, 0x84, 0x60, 0x0c, 0x25, 0x66, 0xe9, 0xe0, 0xe5, 0xac,
    0x22, 0xbf, 0x15, 0xdc, 0x71, 0xb1, 0x88, 0x4f, 0x16, 0xbf, 0xc2, 0x77, 0x37, 0x76, 0x3f, 0xe0,
    0x67, 0xc6, 0x1d, 0x23, 0xfe, 0x7c, 0x8b, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
    0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20, 0x47, 0xcc, 0x41, 0x8a, 0x27, 0xc7,
    0xd0, 0xaa, 0xb4, 0xab, 0x85, 0xbf, 0x09, 0x4d, 0x06, 0xd7, 0x7e, 0x0d, 0x39, 0xf9, 0x36, 0xa1,
    0x3d, 0x96, 0x23, 0xe2, 0x24, 0x64, 0x98, 0x63, 0x21, 0xba, 0x02, 0x21, 0x00, 0xe5, 0x8f, 0x7f,
    0xf1, 0xa6, 0x82, 0x03, 0x6a, 0x18, 0x7a, 0x54, 0xe7, 0x0e, 0x25, 0x77, 0xd8, 0x46, 0xfa, 0x96,
    0x8a, 0x7e, 0x14, 0xc4, 0xcb, 0x21, 0x32, 0x3e, 0x89, 0xd9, 0xba, 0x8c, 0x3f
};
int serverCertLen = sizeof(serverCert);

unsigned char serverPrivateKey[] = {
    0x30, 0x77, 0x02, 0x01, 0x01, 0x04, 0x20, 0x02, 0x51, 0xb5, 0x97, 0xb9, 0xe9, 0xd8, 0x8d, 0x66,
    0x2b, 0x8a, 0xb3, 0x9c, 0x6a, 0xd2, 0xca, 0x18, 0x21, 0xb9, 0x87, 0x3d, 0xf5, 0x8e, 0xa2, 0x8d,
    0x38, 0xf6, 0xb7, 0xd2, 0x76, 0x74, 0x99, 0xa0, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
    0x03, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0xf7, 0x13, 0x5c, 0x73, 0x72, 0xce, 0x10,
    0xe5, 0x09, 0x97, 0x9a, 0xf8, 0xf2, 0x70, 0xa6, 0x3d, 0x89, 0xf5, 0xc5, 0xe4, 0x44, 0xe2, 0x4a,
    0xb6, 0x61, 0xa8, 0x12, 0x8d, 0xb4, 0xdc, 0x2b, 0x47, 0x84, 0x60, 0x0c, 0x25, 0x66, 0xe9, 0xe0,
    0xe5, 0xac, 0x22, 0xbf, 0x15, 0xdc, 0x71, 0xb1, 0x88, 0x4f, 0x16, 0xbf, 0xc2, 0x77, 0x37, 0x76,
    0x3f, 0xe0, 0x67, 0xc6, 0x1d, 0x23, 0xfe, 0x7c, 0x8b
};

int serverPrivateKeyLen = sizeof(serverPrivateKey);

unsigned char caCert[] = {
    0x30, 0x82, 0x02, 0x3e, 0x30, 0x82, 0x01, 0xe5, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x09, 0x00,
    0x87, 0xa7, 0x68, 0x01, 0x7c, 0xe9, 0xf8, 0xf0, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce,
    0x3d, 0x04, 0x03, 0x02, 0x30, 0x7c, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
    0x02, 0x55, 0x53, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x09, 0x53, 0x6f,
    0x6d, 0x65, 0x73, 0x74, 0x61, 0x74, 0x65, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x07,
    0x0c, 0x08, 0x53, 0x6f, 0x6d, 0x65, 0x63, 0x69, 0x74, 0x79, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03,
    0x55, 0x04, 0x0a, 0x0c, 0x02, 0x42, 0x42, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0b,
    0x0c, 0x0d, 0x53, 0x65, 0x71, 0x75, 0x72, 0x69, 0x74, 0x79, 0x20, 0x50, 0x61, 0x72, 0x74, 0x31,
    0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x02, 0x6f, 0x62, 0x31, 0x14, 0x30, 0x12,
    0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01, 0x16, 0x05, 0x6f, 0x62, 0x40,
    0x62, 0x62, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x36, 0x30, 0x37, 0x32, 0x35, 0x31, 0x31, 0x31, 0x36,
    0x31, 0x31, 0x5a, 0x17, 0x0d, 0x31, 0x39, 0x30, 0x35, 0x31, 0x35, 0x31, 0x31, 0x31, 0x36, 0x31,
    0x31, 0x5a, 0x30, 0x7c, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55,
    0x53, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x09, 0x53, 0x6f, 0x6d, 0x65,
    0x73, 0x74, 0x61, 0x74, 0x65, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x08,
    0x53, 0x6f, 0x6d, 0x65, 0x63, 0x69, 0x74, 0x79, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
    0x0a, 0x0c, 0x02, 0x42, 0x42, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x0d,
    0x53, 0x65, 0x71, 0x75, 0x72, 0x69, 0x74, 0x79, 0x20, 0x50, 0x61, 0x72, 0x74, 0x31, 0x0b, 0x30,
    0x09, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x02, 0x6f, 0x62, 0x31, 0x14, 0x30, 0x12, 0x06, 0x09,
    0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01, 0x16, 0x05, 0x6f, 0x62, 0x40, 0x62, 0x62,
    0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x2e, 0xcf, 0xc3, 0xfa, 0x2e,
    0x04, 0x11, 0x62, 0x34, 0x63, 0x6f, 0xdf, 0xb6, 0x67, 0xfb, 0x5a, 0x50, 0x8c, 0x15, 0x73, 0xc9,
    0xc1, 0x57, 0x3a, 0x9e, 0xf8, 0xf4, 0xa8, 0x0c, 0x1a, 0xe9, 0x91, 0x51, 0x9d, 0x03, 0x26, 0x48,
    0xaa, 0x46, 0x84, 0x12, 0x06, 0x2d, 0xfc, 0x66, 0xbe, 0x41, 0xed, 0xfd, 0xcd, 0x32, 0xa3, 0x9b,
    0x34, 0xf2, 0xaa, 0x95, 0x1f, 0x8e, 0x5d, 0x49, 0x77, 0x80, 0xc2, 0xa3, 0x50, 0x30, 0x4e, 0x30,
    0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x1e, 0x81, 0xc3, 0x62, 0xff, 0x8c,
    0x5a, 0x98, 0x90, 0xac, 0x2c, 0xc3, 0x65, 0xb9, 0x3f, 0x8f, 0x04, 0x55, 0xfa, 0x7c, 0x30, 0x1f,
    0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x1e, 0x81, 0xc3, 0x62, 0xff,
    0x8c, 0x5a, 0x98, 0x90, 0xac, 0x2c, 0xc3, 0x65, 0xb9, 0x3f, 0x8f, 0x04, 0x55, 0xfa, 0x7c, 0x30,
    0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x0a, 0x06,
    0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x47, 0x00, 0x30, 0x44, 0x02, 0x20,
    0x1c, 0xa1, 0x55, 0xa8, 0x04, 0xbc, 0x5b, 0x00, 0xa8, 0xac, 0x2f, 0xe6, 0xb7, 0x3c, 0x7c, 0xf3,
    0x7e, 0x93, 0xce, 0xe0, 0xdf, 0x6e, 0x36, 0xe4, 0x36, 0x20, 0xcb, 0x36, 0x9c, 0x13, 0x3b, 0xc4,
    0x02, 0x20, 0x7f, 0x18, 0x13, 0x7d, 0x1b, 0x8c, 0xe3, 0x5b, 0xd9, 0xac, 0x74, 0x8c, 0xc0, 0xe9,
    0xbf, 0x1b, 0x48, 0x6f, 0xb6, 0x6a, 0x45, 0x03, 0xa6, 0x5d, 0x4d, 0x65, 0xf7, 0x96, 0xa0, 0x08,
    0x83, 0x7c
};
int caCertLen = sizeof(caCert);

unsigned char control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM[] = {
    0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x20, 0x32, 0x30, 0x30, 0x20, 0x4f, 0x4b, 0x0d,
    0x0a, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, 0x3a, 0x20, 0x74,
    0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x0d, 0x0a, 0x0d, 0x0a, 0x3c, 0x68, 0x32, 0x3e,
    0x6d, 0x62, 0x65, 0x64, 0x20, 0x54, 0x4c, 0x53, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x65, 0x72, 0x3c, 0x2f, 0x68, 0x32, 0x3e, 0x0d, 0x0a, 0x54, 0x45, 0x53, 0x54, 0x20,
    0x4d, 0x45, 0x53, 0x53, 0x41, 0x47, 0x45, 0x0d, 0x0a, 0x3c, 0x70, 0x3e, 0x53, 0x75, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x66, 0x75, 0x6c, 0x20, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x54, 0x4c, 0x53, 0x2d, 0x45, 0x43, 0x44,
    0x48, 0x45, 0x2d, 0x45, 0x43, 0x44, 0x53, 0x41, 0x2d, 0x57, 0x49, 0x54, 0x48, 0x2d, 0x41, 0x45,
    0x53, 0x2d, 0x31, 0x32, 0x38, 0x2d, 0x43, 0x43, 0x4d, 0x3c, 0x2f, 0x70, 0x3e, 0x0d, 0x0a
};
int control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_len = sizeof(control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM);

unsigned char control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_8[] = {
    0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x20, 0x32, 0x30, 0x30, 0x20, 0x4f, 0x4b, 0x0d,
    0x0a, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, 0x3a, 0x20, 0x74,
    0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x0d, 0x0a, 0x0d, 0x0a, 0x3c, 0x68, 0x32, 0x3e,
    0x6d, 0x62, 0x65, 0x64, 0x20, 0x54, 0x4c, 0x53, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x65, 0x72, 0x3c, 0x2f, 0x68, 0x32, 0x3e, 0x0d, 0x0a, 0x54, 0x45, 0x53, 0x54, 0x20,
    0x4d, 0x45, 0x53, 0x53, 0x41, 0x47, 0x45, 0x0d, 0x0a, 0x3c, 0x70, 0x3e, 0x53, 0x75, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x66, 0x75, 0x6c, 0x20, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x54, 0x4c, 0x53, 0x2d, 0x45, 0x43, 0x44,
    0x48, 0x45, 0x2d, 0x45, 0x43, 0x44, 0x53, 0x41, 0x2d, 0x57, 0x49, 0x54, 0x48, 0x2d, 0x41, 0x45,
    0x53, 0x2d, 0x31, 0x32, 0x38, 0x2d, 0x43, 0x43, 0x4d, 0x2d, 0x38, 0x3c, 0x2f, 0x70, 0x3e, 0x0d,
    0x0a
};
int control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_8_len = sizeof(control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_8);

unsigned char control_server_message_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256[] = {
    0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x20, 0x32, 0x30, 0x30, 0x20, 0x4f, 0x4b, 0x0d,
    0x0a, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, 0x3a, 0x20, 0x74,
    0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x0d, 0x0a, 0x0d, 0x0a, 0x3c, 0x68, 0x32, 0x3e,
    0x6d, 0x62, 0x65, 0x64, 0x20, 0x54, 0x4c, 0x53, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x65, 0x72, 0x3c, 0x2f, 0x68, 0x32, 0x3e, 0x0d, 0x0a, 0x54, 0x45, 0x53, 0x54, 0x20,
    0x4d, 0x45, 0x53, 0x53, 0x41, 0x47, 0x45, 0x0d, 0x0a, 0x3c, 0x70, 0x3e, 0x53, 0x75, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x66, 0x75, 0x6c, 0x20, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x54, 0x4c, 0x53, 0x2d, 0x45, 0x43, 0x44,
    0x48, 0x45, 0x2d, 0x45, 0x43, 0x44, 0x53, 0x41, 0x2d, 0x57, 0x49, 0x54, 0x48, 0x2d, 0x41, 0x45,
    0x53, 0x2d, 0x31, 0x32, 0x38, 0x2d, 0x43, 0x42, 0x43, 0x2d, 0x53, 0x48, 0x41, 0x32, 0x35, 0x36,
    0x3c, 0x2f, 0x70, 0x3e, 0x0d, 0x0a
};
int control_server_message_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256_len = sizeof(control_server_message_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256);

unsigned char control_server_message_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256[] = {
    0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x20, 0x32, 0x30, 0x30, 0x20, 0x4f, 0x4b, 0x0d,
    0x0a, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, 0x3a, 0x20, 0x74,
    0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x0d, 0x0a, 0x0d, 0x0a, 0x3c, 0x68, 0x32, 0x3e,
    0x6d, 0x62, 0x65, 0x64, 0x20, 0x54, 0x4c, 0x53, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x65, 0x72, 0x3c, 0x2f, 0x68, 0x32, 0x3e, 0x0d, 0x0a, 0x54, 0x45, 0x53, 0x54, 0x20,
    0x4d, 0x45, 0x53, 0x53, 0x41, 0x47, 0x45, 0x0d, 0x0a, 0x3c, 0x70, 0x3e, 0x53, 0x75, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x66, 0x75, 0x6c, 0x20, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x54, 0x4c, 0x53, 0x2d, 0x45, 0x43, 0x44,
    0x48, 0x45, 0x2d, 0x45, 0x43, 0x44, 0x53, 0x41, 0x2d, 0x57, 0x49, 0x54, 0x48, 0x2d, 0x41, 0x45,
    0x53, 0x2d, 0x31, 0x32, 0x38, 0x2d, 0x47, 0x43, 0x4D, 0x2d, 0x53, 0x48, 0x41, 0x32, 0x35, 0x36,
    0x3c, 0x2f, 0x70, 0x3e, 0x0d, 0x0a
};
int control_server_message_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256_len = sizeof(control_server_message_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256);

unsigned char control_server_message_ECDHE_ECDSA_WITH_AES_128_CBC_SHA[] = {
    0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x20, 0x32, 0x30, 0x30, 0x20, 0x4f, 0x4b, 0x0d,
    0x0a, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, 0x3a, 0x20, 0x74,
    0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x0d, 0x0a, 0x0d, 0x0a, 0x3c, 0x68, 0x32, 0x3e,
    0x6d, 0x62, 0x65, 0x64, 0x20, 0x54, 0x4c, 0x53, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x65, 0x72, 0x3c, 0x2f, 0x68, 0x32, 0x3e, 0x0d, 0x0a, 0x54, 0x45, 0x53, 0x54, 0x20,
    0x4d, 0x45, 0x53, 0x53, 0x41, 0x47, 0x45, 0x0d, 0x0a, 0x3c, 0x70, 0x3e, 0x53, 0x75, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x66, 0x75, 0x6c, 0x20, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x54, 0x4c, 0x53, 0x2d, 0x45, 0x43, 0x44,
    0x48, 0x45, 0x2d, 0x45, 0x43, 0x44, 0x53, 0x41, 0x2d, 0x57, 0x49, 0x54, 0x48, 0x2d, 0x41, 0x45,
    0x53, 0x2d, 0x31, 0x32, 0x38, 0x2d, 0x43, 0x42, 0x43, 0x2d, 0x53, 0x48, 0x41, 0x3c, 0x2f, 0x70,
    0x3e, 0x0d, 0x0a
};
int control_server_message_ECDHE_ECDSA_WITH_AES_128_CBC_SHA_len = sizeof(control_server_message_ECDHE_ECDSA_WITH_AES_128_CBC_SHA);

unsigned char control_client_message[] = {
    0x47, 0x45, 0x54, 0x20, 0x2f, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x0d, 0x0a,
    0x0d, 0x0a
};
int control_client_message_len = sizeof(control_client_message);

unsigned char control_server_message_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384[] = {
    0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x20, 0x32, 0x30, 0x30, 0x20, 0x4f, 0x4b, 0x0d,
    0x0a, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, 0x3a, 0x20, 0x74,
    0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x0d, 0x0a, 0x0d, 0x0a, 0x3c, 0x68, 0x32, 0x3e,
    0x6d, 0x62, 0x65, 0x64, 0x20, 0x54, 0x4c, 0x53, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x65, 0x72, 0x3c, 0x2f, 0x68, 0x32, 0x3e, 0x0d, 0x0a, 0x54, 0x45, 0x53, 0x54, 0x20,
    0x4d, 0x45, 0x53, 0x53, 0x41, 0x47, 0x45, 0x0d, 0x0a, 0x3c, 0x70, 0x3e, 0x53, 0x75, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x66, 0x75, 0x6c, 0x20, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x54, 0x4c, 0x53, 0x2d, 0x45, 0x43, 0x44,
    0x48, 0x45, 0x2d, 0x45, 0x43, 0x44, 0x53, 0x41, 0x2d, 0x57, 0x49, 0x54, 0x48, 0x2d, 0x41, 0x45,
    0x53, 0x2d, 0x32, 0x35, 0x36, 0x2d, 0x43, 0x42, 0x43, 0x2d, 0x53, 0x48, 0x41, 0x33, 0x38, 0x34,
    0x3c, 0x2f, 0x70, 0x3e, 0x0d, 0x0a
};
int control_server_message_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384_len = sizeof(control_server_message_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384);

unsigned char control_server_message_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384[] = {
    0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x30, 0x20, 0x32, 0x30, 0x30, 0x20, 0x4f, 0x4b, 0x0d,
    0x0a, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, 0x3a, 0x20, 0x74,
    0x65, 0x78, 0x74, 0x2f, 0x68, 0x74, 0x6d, 0x6c, 0x0d, 0x0a, 0x0d, 0x0a, 0x3c, 0x68, 0x32, 0x3e,
    0x6d, 0x62, 0x65, 0x64, 0x20, 0x54, 0x4c, 0x53, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x65, 0x72, 0x3c, 0x2f, 0x68, 0x32, 0x3e, 0x0d, 0x0a, 0x54, 0x45, 0x53, 0x54, 0x20,
    0x4d, 0x45, 0x53, 0x53, 0x41, 0x47, 0x45, 0x0d, 0x0a, 0x3c, 0x70, 0x3e, 0x53, 0x75, 0x63, 0x63,
    0x65, 0x73, 0x73, 0x66, 0x75, 0x6c, 0x20, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x75, 0x73, 0x69, 0x6e, 0x67, 0x3a, 0x20, 0x54, 0x4c, 0x53, 0x2d, 0x45, 0x43, 0x44,
    0x48, 0x45, 0x2d, 0x45, 0x43, 0x44, 0x53, 0x41, 0x2d, 0x57, 0x49, 0x54, 0x48, 0x2d, 0x41, 0x45,
    0x53, 0x2d, 0x32, 0x35, 0x36, 0x2d, 0x47, 0x43, 0x4D, 0x2d, 0x53, 0x48, 0x41, 0x33, 0x38, 0x34,
    0x3c, 0x2f, 0x70, 0x3e, 0x0d, 0x0a
};
int control_server_message_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384_len = sizeof(control_server_message_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384);

static void error(const char *msg)
{
    perror(msg);
    exit(0);
}

static int sockfd, newsockfd;

static void socketConnect()
{
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = SERVER_PORT;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(SERVER_NAME);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    //memset((char *) &serv_addr, sizeof(serv_addr));
    memset((void*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
}

static ssize_t CATCPPacketSendCB(CAEndpoint_t *, const void *buf, size_t buflen)
{
    int n;
    n = write(sockfd, buf, buflen);
    if (n < 0)
         error("ERROR writing to socket");
    return n;
}

char msg[256] = {0}; size_t msglen = 0;
static void CATCPPacketReceivedCB(const CASecureEndpoint_t *, const void *data, size_t dataLength)
{
    memcpy(msg, data, dataLength);
    msglen = dataLength;
}

static void PacketReceive(unsigned char *data, int * datalen)
{
    int n;
    char buffer[2048] = {'\0'};
    n = read(sockfd, buffer, 5);
    if ((buffer[0] == 0x16 || buffer[0] == 0x14 || buffer[0] == 0x17 || buffer[0] == 0x15)
        && buffer[1] == 0x03 && buffer[2] == 0x03)
    {
        int tlslen = (unsigned char)buffer[3] * 0x100 + (unsigned char)buffer[4];
        n = read(sockfd, buffer + 5, tlslen);
    }

    if (n < 0)
         error("ERROR reading from socket");

    *datalen = n + 5;
    memcpy(data, buffer, *datalen);
}

static void socketClose()
{
    close(sockfd);
}

static void infoCallback_that_loads_x509(PkiInfo_t * inf)
{
    inf->crt.data = (uint8_t*)serverCert;
    inf->crt.len = sizeof(serverCert);
    inf->key.data = (uint8_t*)serverPrivateKey;
    inf->key.len = sizeof(serverPrivateKey);
    inf->ca.data = (uint8_t*)caCert;
    inf->ca.len = sizeof(caCert);
    inf->crl.data = NULL;
    inf->crl.len = 0;
}

static void socketOpen_server()
{
    int portno;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    portno = SERVER_PORT;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("\nERROR opening socket");
    //bzero((char *) &serv_addr, sizeof(serv_addr));
    memset((void*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("\nERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
    if (newsockfd < 0)
          error("\nERROR on accept");
}

static ssize_t CATCPPacketSendCB_server(CAEndpoint_t *, const void *buf, size_t buflen)
{
    int n;
    n = write(newsockfd,buf,buflen);
    if (n < 0)
         error("ERROR writing to socket");
    return n;
}

static void CATCPPacketReceivedCB_server(const CASecureEndpoint_t *, const void *data, size_t dataLength)
{
    memcpy(msg, data, dataLength);
    msglen = dataLength;
}
static void PacketReceive_server(unsigned char *data, int * datalen)
{
    int n;
    char buffer[2048] = {'\0'};
    n = read(newsockfd, buffer, 5);

    if (buffer[0] == 0x16 || buffer[0] == 0x14 || buffer[0] == 0x17 || buffer[0] == 0x15)
    {
        int tlslen = (unsigned char)buffer[3] * 0x100 + (unsigned char)buffer[4];
        n = read(newsockfd, buffer + 5, tlslen);
    }

    if (n < 0)
         error("\nERROR reading from socket");

    *datalen = n + 5;
    memcpy(data, buffer, *datalen);
}

static void socketClose_server()
{
    close(newsockfd);
    close(sockfd);
}

static void clutch(bool * list)
{
    list[1] = true;
}

const unsigned char IDENTITY[] = ("6767676767676767");
const unsigned char RS_CLIENT_PSK[] = ("AAAAAAAAAAAAAAAA");
static int32_t GetDtlsPskCredentials( CADtlsPskCredType_t,
              const unsigned char *, size_t,
              unsigned char *result, size_t)
{
    int32_t ret = -1;

    if (NULL == result)
    {
        return ret;
    }
    memcpy(result, IDENTITY, sizeof(IDENTITY));
    ret = sizeof(IDENTITY);

    return ret;
}

/* **************************
 *
 *
 * MbedTLS client routine
 *
 *
 * *************************/

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#endif

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_ENTROPY_C) ||  \
    !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_CLI_C) || \
    !defined(MBEDTLS_NET_C) || !defined(MBEDTLS_RSA_C) ||         \
    !defined(MBEDTLS_PEM_PARSE_C) ||!defined(MBEDTLS_CTR_DRBG_C) || \
    !defined(MBEDTLS_X509_CRT_PARSE_C)
static int client( void )
{
    mbedtls_printf("MBEDTLS_BIGNUM_C and/or MBEDTLS_ENTROPY_C and/or "
           "MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_CLI_C and/or "
           "MBEDTLS_NET_C and/or MBEDTLS_RSA_C and/or "
           "MBEDTLS_CTR_DRBG_C and/or MBEDTLS_X509_CRT_PARSE_C "
           "not defined.\n");
    return( 0 );
}
#else

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"


#include <string.h>

#define DEBUG_LEVEL (0)

static void my_debug_client( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    mbedtls_fprintf( (FILE *) ctx, "%s:%04d: %s", file, line, str );
    fflush(  (FILE *) ctx  );
}

static void * client(void *)
{
    int ret, len;
    mbedtls_net_context server_fd;
    uint32_t flags;
    unsigned char buf[1024];
    const char *pers = "ssl_client1";

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt owncert;
    mbedtls_pk_context pkey;

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_net_init( &server_fd );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    mbedtls_x509_crt_init( &cacert );
    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_pk_init( &pkey );

    mbedtls_printf( "\n  . Seeding the random number generator..." );
    fflush( stdout );

    mbedtls_entropy_init( &entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 0. Initialize certificates
     */
    mbedtls_printf( "  . Loading the CA root certificate ..." );
    fflush( stdout );
    ret = mbedtls_x509_crt_parse( &cacert, (const unsigned char *) caCert, caCertLen );
    if( ret < 0 )
    {
        mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse caCert returned -0x%x\n\n", -ret );
        goto exit;
    }
    ret = mbedtls_x509_crt_parse( &owncert, (const unsigned char *) serverCert, serverCertLen );
    if( ret < 0 )
    {
        mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse serverCert returned -0x%x\n\n", -ret );
        goto exit;
    }
    ret =  mbedtls_pk_parse_key( &pkey, (const unsigned char *) serverPrivateKey,
                                                                serverPrivateKeyLen, NULL, 0 );
    if( ret < 0 )
    {
        mbedtls_printf( " failed\n  !  mbedtls_pk_parse_key returned -0x%x\n\n", -ret );
        goto exit;
    }

    mbedtls_printf( " ok (%d skipped)\n", ret );

    /*
     * 1. Start the connection
     */
    mbedtls_printf( "  . Connecting to tcp/%s/%s...", SERVER_NAME, "SERVER_PORT" );
    fflush( stdout );

    if( ( ret = mbedtls_net_connect( &server_fd, "127.0.0.1",
                                         "4433", MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_net_connect returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 2. Setup stuff
     */
    mbedtls_printf( "  . Setting up the SSL/TLS structure..." );
    fflush( stdout );

    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /* OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
    mbedtls_ssl_conf_own_cert( &conf, &owncert, &pkey );
    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug_client, stdout );

    if( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
        goto exit;
    }

    if( ( ret = mbedtls_ssl_set_hostname( &ssl, "mbed TLS Server 1" ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

    /*
     * 4. Handshake
     */
    mbedtls_printf( "  . Performing the SSL/TLS handshake..." );
    fflush( stdout );

    while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret );
            goto exit;
        }
    }

    mbedtls_printf( " ok\n" );

    /*
     * 5. Verify the server certificate
     */
    mbedtls_printf( "  . Verifying peer X.509 certificate..." );

    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        mbedtls_printf( " failed\n" );

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        mbedtls_printf( "%s\n", vrfy_buf );
    }
    else
        mbedtls_printf( " ok\n" );

    /*
     * 3. Write the GET request
     */
    mbedtls_printf( "  > Write to server:" );
    fflush( stdout );

    len = sprintf( (char *) buf, GET_REQUEST );

    while( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
            goto exit;
        }
    }

    len = ret;
    mbedtls_printf( " %d bytes written\n\n%s", len, (char *) buf );

    /*
     * 7. Read the HTTP response
     */
    mbedtls_printf( "  < Read from server:" );
    fflush( stdout );

    do
    {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            mbedtls_printf( "failed\n  ! mbedtls_ssl_read returned %d\n\n", ret );
            break;
        }

        if( ret == 0 )
        {
            mbedtls_printf( "\n\nEOF\n\n" );
            break;
        }

        len = ret;
        mbedtls_printf( " %d bytes read\n\n%s", len, (char *) buf );
    }
    while( 1 );

    mbedtls_ssl_close_notify( &ssl );

exit:

#ifdef MBEDTLS_ERROR_C
    if( ret != 0 )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        mbedtls_printf("Last error was: %d - %s\n\n", ret, error_buf );
    }
#endif

    mbedtls_net_free( &server_fd );

    mbedtls_x509_crt_free( &cacert );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return NULL;
}
#endif /* MBEDTLS_BIGNUM_C && MBEDTLS_ENTROPY_C && MBEDTLS_SSL_TLS_C &&
          MBEDTLS_SSL_CLI_C && MBEDTLS_NET_C && MBEDTLS_RSA_C &&
          MBEDTLS_PEM_PARSE_C && MBEDTLS_CTR_DRBG_C &&
          MBEDTLS_X509_CRT_PARSE_C */

/* **************************
 *
 *
 * MbedTLS server routine
 *
 *
 * *************************/

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#endif

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_PEM_PARSE_C)|| \
    !defined(MBEDTLS_ENTROPY_C) || !defined(MBEDTLS_SSL_TLS_C) || \
    !defined(MBEDTLS_SSL_SRV_C) || !defined(MBEDTLS_NET_C) ||     \
    !defined(MBEDTLS_RSA_C) || !defined(MBEDTLS_CTR_DRBG_C) ||    \
    !defined(MBEDTLS_X509_CRT_PARSE_C) || !defined(MBEDTLS_FS_IO) 

/* int */void * server( void )
{
    mbedtls_printf("MBEDTLS_BIGNUM_C and/or MBEDTLS_ENTROPY_C "
           "and/or MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_SRV_C and/or "
           "MBEDTLS_NET_C and/or MBEDTLS_RSA_C and/or "
           "MBEDTLS_CTR_DRBG_C and/or MBEDTLS_X509_CRT_PARSE_C "
           "and/or MBEDTLS_PEM_PARSE_C not defined.\n");
    return(/* 0 */);
}
#else

#include <stdlib.h>
#include <string.h>

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif

#define HTTP_RESPONSE \
    "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" \
    "<h2>mbed TLS Test Server</h2>\r\n" \
    "TEST MESSAGE\r\n" \
    "<p>Successful connection using: %s</p>\r\n"

#define DEBUG_LEVEL (0)

static int mbedtls_entropy_func_clutch(void *, unsigned char *output, size_t len)
{
    for (uint32_t i = 0; i < len; i++) output[i] = 0x11;
    return 0;
}

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    mbedtls_fprintf((FILE *) ctx, "%s:%04d: %s", file, line, str);
    fflush(  (FILE *) ctx  );
}

static void * server(void *)
{
    int ret, len;
    mbedtls_net_context listen_fd, client_fd;
    unsigned char buf[1024];
    const char *pers = "ssl_server";

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt srvcert;
    mbedtls_pk_context pkey;
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_context cache;
#endif

    mbedtls_net_init( &listen_fd );
    mbedtls_net_init( &client_fd );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_init( &cache );
#endif
    mbedtls_x509_crt_init( &srvcert );
    mbedtls_pk_init( &pkey );
    mbedtls_entropy_init( &entropy );
    mbedtls_ctr_drbg_init( &ctr_drbg );

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold( DEBUG_LEVEL );
#endif

    /*
     * 1. Load the certificates and private RSA key
     */
    mbedtls_printf( "\n  . Loading the server cert. and key..." );
    fflush( stdout );

    /*
     * This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) serverCert, serverCertLen );
    if( ret != 0 )
    {
        mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret );
        goto exit;
    }

    ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) caCert, caCertLen );
    if( ret != 0 )
    {
        mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret );
        goto exit;
    }

    ret =  mbedtls_pk_parse_key( &pkey, (const unsigned char *) serverPrivateKey,
                                                                serverPrivateKeyLen, NULL, 0 );
    if( ret != 0 )
    {
        mbedtls_printf( " failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 2. Setup the listening TCP socket
     */
    mbedtls_printf( "  . Bind on https://localhost:4433/ ..." );
    fflush( stdout );

    if( ( ret = mbedtls_net_bind( &listen_fd, NULL, "4433", MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_net_bind returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 3. Seed the RNG
     */
    mbedtls_printf( "  . Seeding the random number generator..." );
    fflush( stdout );

    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func_clutch, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
    }

    mbedtls_printf( " ok\n" );

    /*
     * 4. Setup stuff
     */
    mbedtls_printf( "  . Setting up the SSL data...." );
    fflush( stdout );

    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                    MBEDTLS_SSL_IS_SERVER,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache( &conf, &cache,
                                   mbedtls_ssl_cache_get,
                                   mbedtls_ssl_cache_set );
#endif

    mbedtls_ssl_conf_ca_chain( &conf, srvcert.next, NULL );
    if( ( ret = mbedtls_ssl_conf_own_cert( &conf, &srvcert, &pkey ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret );
        goto exit;
    }

    if( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

reset:
#ifdef MBEDTLS_ERROR_C
    if( ret != 0 )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        mbedtls_printf("Last error was: %d - %s\n\n", ret, error_buf );
    }
#endif

    mbedtls_net_free( &client_fd );

    mbedtls_ssl_session_reset( &ssl );

    /*
     * 3. Wait until a client connects
     */
    mbedtls_printf( "  . Waiting for a remote connection ..." );
    fflush( stdout );

    if( ( ret = mbedtls_net_accept( &listen_fd, &client_fd,
                                    NULL, 0, NULL ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_net_accept returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_ssl_set_bio( &ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

    mbedtls_printf( " ok\n" );

    /*
     * 5. Handshake
     */
    mbedtls_printf( "  . Performing the SSL/TLS handshake..." );
    fflush( stdout );

    while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_handshake returned %d\n\n", ret );
            goto exit;
        }
    }

    mbedtls_printf( " ok\n" );

    /*
     * 6. Read the HTTP Request
     */
    mbedtls_printf( "  < Read from client:" );
    fflush( stdout );

    do
    {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret <= 0 )
        {
            switch( ret )
            {
                case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                    mbedtls_printf( " connection was closed gracefully\n" );
                    break;

                case MBEDTLS_ERR_NET_CONN_RESET:
                    mbedtls_printf( " connection was reset by peer\n" );
                    break;

                default:
                    mbedtls_printf( " mbedtls_ssl_read returned -0x%x\n", -ret );
                    break;
            }

            break;
        }

        len = ret;
        mbedtls_printf( " %d bytes read\n\n%s", len, (char *) buf );

        if( ret > 0 )
            break;
    }
    while( 1 );

    /*
     * 7. Write the 200 Response
     */
    mbedtls_printf( "  > Write to client:" );

    fflush( stdout );

    len = sprintf( (char *) buf, HTTP_RESPONSE,
                   mbedtls_ssl_get_ciphersuite( &ssl ) );

    while( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 )
    {
        if( ret == MBEDTLS_ERR_NET_CONN_RESET )
        {
            mbedtls_printf( " failed\n  ! peer closed the connection\n\n" );
            goto reset;
        }

        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
            goto exit;
        }
    }

    len = ret;
    mbedtls_printf( " %d bytes written\n\n%s\n", len, (char *) buf );

    mbedtls_printf( "  . Closing the connection..." );

    while( ( ret = mbedtls_ssl_close_notify( &ssl ) ) < 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_close_notify returned %d\n\n", ret );
            goto exit;
        }
    }

    mbedtls_printf( " ok\n" );

    ret = 0;
    goto exit;

exit:

#ifdef MBEDTLS_ERROR_C
    if( ret != 0 )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        mbedtls_printf("Last error was: %d - %s\n\n", ret, error_buf );
    }
#endif

    mbedtls_net_free( &client_fd );
    mbedtls_net_free( &listen_fd );

    mbedtls_x509_crt_free( &srvcert );
    mbedtls_pk_free( &pkey );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_free( &cache );
#endif
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

#if defined(_WIN32)
    mbedtls_printf( "  Press Enter to exit this program.\n" );
    fflush( stdout );
#endif

    return NULL;
}
#endif /* MBEDTLS_BIGNUM_C && MBEDTLS_ENTROPY_C &&
          MBEDTLS_SSL_TLS_C && MBEDTLS_SSL_SRV_C && MBEDTLS_NET_C &&
          MBEDTLS_RSA_C && MBEDTLS_CTR_DRBG_C && MBEDTLS_X509_CRT_PARSE_C
          && MBEDTLS_FS_IO && MBEDTLS_PEM_PARSE_C */

/* **************************
 *
 *
 * CAinitSslAdapter test
 *
 *
 * *************************/

static int testCAinitSslAdapter()
{
    int ret = 0;

    CAEndpoint_t serverAddr;
    serverAddr.adapter = CA_ADAPTER_IP;
    serverAddr.flags = CA_SECURE;
    serverAddr.port = 4433;
    char addr[] = {0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x00}; // 127.0.0.1
    memcpy(serverAddr.addr, addr, sizeof(addr));
    serverAddr.ifindex = 0;

    ret = CAinitSslAdapter();
    if (ret != 0 ||
        &g_caSslContext == NULL ||
        &g_caSslContext->crt == NULL ||
        &g_caSslContext->pkey == NULL ||
        &g_caSslContext->clientTlsConf == NULL ||
        &g_caSslContext->serverTlsConf == NULL ||
        &g_caSslContext->rnd == NULL ||
        &g_caSslContext->entropy == NULL)
    {
        ret = 1;
    }

    // CAdeinitSslAdapter
    oc_mutex_lock(g_sslContextMutex);
    DeletePeerList();
    mbedtls_x509_crt_free(&g_caSslContext->crt);
    mbedtls_pk_free(&g_caSslContext->pkey);
    mbedtls_ssl_config_free(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_free(&g_caSslContext->serverTlsConf);
    mbedtls_ctr_drbg_free(&g_caSslContext->rnd);
    mbedtls_entropy_free(&g_caSslContext->entropy);
    OICFree(g_caSslContext);
    g_caSslContext = NULL;
    oc_mutex_unlock(g_sslContextMutex);
    oc_mutex_free(g_sslContextMutex);
    g_sslContextMutex = NULL;

    return ret;
}

// CAinitTlsAdapter()
TEST(TLSAdaper, Test_1)
{
    int ret = 0xFF;
    ret = testCAinitSslAdapter();
    EXPECT_EQ(0, ret);
}

/* **************************
 *
 *
 * CAsetSslAdapterCallbacks test
 *
 *
 * *************************/

static int testCAsetSslAdapterCallbacks()
{
    int ret = 0xFF;
    CAEndpoint_t serverAddr;
    serverAddr.adapter = CA_ADAPTER_IP;
    serverAddr.flags = CA_SECURE;
    serverAddr.port = 4433;
    char addr[] = {0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x00}; // 127.0.0.1
    memcpy(serverAddr.addr, addr, sizeof(addr));
    serverAddr.ifindex = 0;

    // CAinitSslAdapter
    g_sslContextMutex = oc_mutex_new();
    oc_mutex_lock(g_sslContextMutex);
    g_caSslContext = (SslContext_t *)OICCalloc(1, sizeof(SslContext_t));
    g_caSslContext->peerList = u_arraylist_create();
    mbedtls_entropy_init(&g_caSslContext->entropy);
    mbedtls_ctr_drbg_init(&g_caSslContext->rnd);
    unsigned char * seed = (unsigned char*) SEED;
    mbedtls_ctr_drbg_seed(&g_caSslContext->rnd, mbedtls_entropy_func_clutch,
                                  &g_caSslContext->entropy, seed, sizeof(SEED));
    mbedtls_ctr_drbg_set_prediction_resistance(&g_caSslContext->rnd, MBEDTLS_CTR_DRBG_PR_OFF);
    mbedtls_ssl_config_init(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_defaults(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_psk_cb(&g_caSslContext->clientTlsConf, GetPskCredentialsCallback, NULL);
    mbedtls_ssl_conf_rng( &g_caSslContext->clientTlsConf, mbedtls_ctr_drbg_random,
                          &g_caSslContext->rnd);
    mbedtls_ssl_conf_curves(&g_caSslContext->clientTlsConf, curve[ADAPTER_CURVE_SECP256R1]);
    mbedtls_ssl_conf_min_version(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                 MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_authmode(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_VERIFY_REQUIRED);
    CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256);
    mbedtls_x509_crt_init(&g_caSslContext->ca);
    mbedtls_x509_crt_init(&g_caSslContext->crt);
    mbedtls_pk_init(&g_caSslContext->pkey);
    mbedtls_x509_crl_init(&g_caSslContext->crl);
    oc_mutex_unlock(g_sslContextMutex);

    CAsetSslAdapterCallbacks(CATCPPacketReceivedCB, CATCPPacketSendCB, (CATransportAdapter_t)0);
    if (g_caSslContext->adapterCallbacks[0].recvCallback == NULL &&
        g_caSslContext->adapterCallbacks[0].sendCallback == NULL &&
        g_caSslContext->adapterCallbacks[1].recvCallback == NULL &&
        g_caSslContext->adapterCallbacks[1].sendCallback == NULL)
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }
    CAsetSslAdapterCallbacks(CATCPPacketReceivedCB, CATCPPacketSendCB, CA_ADAPTER_IP);
    CAsetSslAdapterCallbacks(CATCPPacketReceivedCB, CATCPPacketSendCB, CA_ADAPTER_TCP);
    if (g_caSslContext->adapterCallbacks[0].recvCallback == CATCPPacketReceivedCB &&
        g_caSslContext->adapterCallbacks[0].sendCallback == CATCPPacketSendCB &&
        g_caSslContext->adapterCallbacks[1].recvCallback == CATCPPacketReceivedCB &&
        g_caSslContext->adapterCallbacks[1].sendCallback == CATCPPacketSendCB)
    {
        ret += 0;
    }
    else
    {
        ret += 1;
    }

    // CAdeinitSslAdapter
    oc_mutex_lock(g_sslContextMutex);
    DeletePeerList();
    mbedtls_x509_crt_free(&g_caSslContext->crt);
    mbedtls_pk_free(&g_caSslContext->pkey);
    mbedtls_ssl_config_free(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_free(&g_caSslContext->serverTlsConf);
    mbedtls_ctr_drbg_free(&g_caSslContext->rnd);
    mbedtls_entropy_free(&g_caSslContext->entropy);
    OICFree(g_caSslContext);
    g_caSslContext = NULL;
    oc_mutex_unlock(g_sslContextMutex);
    oc_mutex_free(g_sslContextMutex);
    g_sslContextMutex = NULL;

    return ret;
}

// CAsetSslAdapterCallbacks()
TEST(TLSAdaper, Test_2)
{
    int ret = 0xFF;
    ret = testCAsetSslAdapterCallbacks();
    EXPECT_EQ(0, ret);
}

/* **************************
 *
 *
 * CAinitiateSslHandshake test
 *
 *
 * *************************/

unsigned char predictedClientHello[] = {
    0x16, 0x03, 0x01, 0x00, 0x63, 0x01, 0x00, 0x00, 0x5f, 0x03, 0x03, 0x57, 0xf2, 0x5f, 0x21, 0x04,
    0xb1, 0x3b, 0xda, 0x55, 0xa4, 0x8e, 0xcc, 0x3f, 0xe9, 0x45, 0x5c, 0xaf, 0xcb, 0x19, 0x2e, 0x1f,
    0x4b, 0xd5, 0x84, 0x5c, 0x4b, 0xd7, 0x7d, 0x38, 0xa2, 0xfa, 0x3d, 0x00, 0x00, 0x06, 0xc0, 0xac,
    0xc0, 0xae, 0x00, 0xff, 0x01, 0x00, 0x00, 0x30, 0x00, 0x0d, 0x00, 0x16, 0x00, 0x14, 0x06, 0x03,
    0x06, 0x01, 0x05, 0x03, 0x05, 0x01, 0x04, 0x03, 0x04, 0x01, 0x03, 0x03, 0x03, 0x01, 0x02, 0x03,
    0x02, 0x01, 0x00, 0x0a, 0x00, 0x04, 0x00, 0x02, 0x00, 0x17, 0x00, 0x0b, 0x00, 0x02, 0x01, 0x00,
    0x00, 0x16, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00
};
static unsigned char controlBuf[sizeof(predictedClientHello)];
static char controlBufLen = 0;
static ssize_t CATCPPacketSendCB_forInitHsTest(CAEndpoint_t *, const void * buf, size_t buflen)
{
    int n;
    n = write(sockfd, buf, buflen);
    if (n < 0)
         error("ERROR writing to socket");

    memset(controlBuf, 0, sizeof(predictedClientHello));
    memcpy(controlBuf, buf, buflen);
    controlBufLen = buflen;
    return buflen;
}

static void * test0CAinitiateSslHandshake(void * arg)
{
    CAEndpoint_t serverAddr;
    serverAddr.adapter = CA_ADAPTER_TCP;
    serverAddr.flags = CA_SECURE;
    serverAddr.port = 4433;
    char addr[] = {0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x00}; // 127.0.0.1
    memcpy(serverAddr.addr, addr, sizeof(addr));
    serverAddr.ifindex = 0;

    // CAinitSslAdapter
    g_sslContextMutex = oc_mutex_new();
    oc_mutex_lock(g_sslContextMutex);
    g_caSslContext = (SslContext_t *)OICCalloc(1, sizeof(SslContext_t));
    g_caSslContext->peerList = u_arraylist_create();
    mbedtls_entropy_init(&g_caSslContext->entropy);
    mbedtls_ctr_drbg_init(&g_caSslContext->rnd);
    unsigned char * seed = (unsigned char*) SEED;
    mbedtls_ctr_drbg_seed(&g_caSslContext->rnd, mbedtls_entropy_func_clutch,
                                  &g_caSslContext->entropy, seed, sizeof(SEED));
    mbedtls_ctr_drbg_set_prediction_resistance(&g_caSslContext->rnd, MBEDTLS_CTR_DRBG_PR_OFF);
    mbedtls_ssl_config_init(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_defaults(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_psk_cb(&g_caSslContext->clientTlsConf, GetPskCredentialsCallback, NULL);
    mbedtls_ssl_conf_rng( &g_caSslContext->clientTlsConf, mbedtls_ctr_drbg_random,
                          &g_caSslContext->rnd);
    mbedtls_ssl_conf_curves(&g_caSslContext->clientTlsConf, curve[ADAPTER_CURVE_SECP256R1]);
    mbedtls_ssl_conf_min_version(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                 MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_authmode(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_x509_crt_init(&g_caSslContext->ca);
    mbedtls_x509_crt_init(&g_caSslContext->crt);
    mbedtls_pk_init(&g_caSslContext->pkey);
    mbedtls_x509_crl_init(&g_caSslContext->crl);
    oc_mutex_unlock(g_sslContextMutex);

    // CAsetSslAdapterCallbacks
    g_caSslContext->adapterCallbacks[1].recvCallback = CATCPPacketReceivedCB;
    g_caSslContext->adapterCallbacks[1].sendCallback = CATCPPacketSendCB_forInitHsTest;

    // CAsetPkixInfoCallback
    g_getPkixInfoCallback = infoCallback_that_loads_x509;

    // CAsetCredentialTypesCallback
    g_getCredentialTypesCallback = clutch;

    // CAsetTlsCipherSuite
    mbedtls_ssl_conf_ciphersuites(&g_caSslContext->clientTlsConf,
                                         tlsCipher[SSL_ECDHE_ECDSA_WITH_AES_128_CCM]);
    mbedtls_ssl_conf_ciphersuites(&g_caSslContext->serverTlsConf,
                                         tlsCipher[SSL_ECDHE_ECDSA_WITH_AES_128_CCM]);
    g_caSslContext->cipher = SSL_ECDHE_ECDSA_WITH_AES_128_CCM;

    CAsetPskCredentialsCallback(GetDtlsPskCredentials);

    socketConnect();

    unsigned int unixTime = (unsigned)time(NULL);
    CAinitiateSslHandshake(&serverAddr);
    predictedClientHello[11] = unixTime >> 24;
    predictedClientHello[12] = (unixTime << 8) >> 24;
    predictedClientHello[13] = (unixTime << 16) >> 24;
    predictedClientHello[14] = (unixTime << 24) >> 24;

    // CAcloseTlsConnection
    oc_mutex_lock(g_sslContextMutex);
    SslEndPoint_t * tep = GetSslPeer(&serverAddr);
    mbedtls_ssl_close_notify(&tep->ssl);
    RemovePeerFromList(&tep->sep.endpoint);
    oc_mutex_unlock(g_sslContextMutex);

    // CAdeinitTlsAdapter
    oc_mutex_lock(g_sslContextMutex);
    DeletePeerList();
    mbedtls_x509_crt_free(&g_caSslContext->crt);
    mbedtls_pk_free(&g_caSslContext->pkey);
    mbedtls_ssl_config_free(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_free(&g_caSslContext->serverTlsConf);
    mbedtls_ctr_drbg_free(&g_caSslContext->rnd);
    mbedtls_entropy_free(&g_caSslContext->entropy);
    OICFree(g_caSslContext);
    g_caSslContext = NULL;
    oc_mutex_unlock(g_sslContextMutex);
    oc_mutex_free(g_sslContextMutex);
    g_sslContextMutex = NULL;

    socketClose();

    if (controlBufLen == sizeof(predictedClientHello) &&
        memcmp(predictedClientHello, controlBuf, sizeof(predictedClientHello)) == 0)
    {
        *((int*)arg) = 0;
        return NULL;
    }
    else
    {
        *((int*)arg) = 0xFF;
        return (void *) 0xFF;
    }
}

static int test1CAinitiateSslHandshake()
{
    int ret = 0xff;
    ret = CAinitiateSslHandshake(NULL);
    if (CA_STATUS_INVALID_PARAM == ret)
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }
    return ret;
}

// CAinitiateSslHandshake()
TEST(TLSAdaper, Test_3_0)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 1;

    ret = pthread_create( &thread1, NULL, server, (void*) NULL);
    if(ret)
    {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, test0CAinitiateSslHandshake, &arg);
    if(ret)
    {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(0, arg);
}

TEST(TLSAdaper, Test_3_1)
{
    int ret = 0xFF;
    ret = test1CAinitiateSslHandshake();
    EXPECT_EQ(0, ret);
}

/* **************************
 *
 *
 * CAencryptSsl test
 *
 *
 * *************************/

static void * testCAencryptSsl(void * arg)
{
    int ret = 0;
    CAEndpoint_t serverAddr;
    serverAddr.adapter = CA_ADAPTER_TCP;
    serverAddr.flags = CA_SECURE;
    serverAddr.port = 4433;
    char addr[] = {0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x00}; // 127.0.0.1
    memcpy(serverAddr.addr, addr, sizeof(addr));
    serverAddr.ifindex = 0;

    // CAinitTlsAdapter
    g_sslContextMutex = oc_mutex_new();
    oc_mutex_lock(g_sslContextMutex);
    g_caSslContext = (SslContext_t *)OICCalloc(1, sizeof(SslContext_t));
    g_caSslContext->peerList = u_arraylist_create();
    mbedtls_entropy_init(&g_caSslContext->entropy);
    mbedtls_ctr_drbg_init(&g_caSslContext->rnd);
    unsigned char * seed = (unsigned char*) SEED;
    mbedtls_ctr_drbg_seed(&g_caSslContext->rnd, mbedtls_entropy_func_clutch,
                                  &g_caSslContext->entropy, seed, sizeof(SEED));
    mbedtls_ctr_drbg_set_prediction_resistance(&g_caSslContext->rnd, MBEDTLS_CTR_DRBG_PR_OFF);
    mbedtls_ssl_config_init(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_defaults(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_psk_cb(&g_caSslContext->clientTlsConf, GetPskCredentialsCallback, NULL);
    mbedtls_ssl_conf_rng( &g_caSslContext->clientTlsConf, mbedtls_ctr_drbg_random,
                          &g_caSslContext->rnd);
    mbedtls_ssl_conf_curves(&g_caSslContext->clientTlsConf, curve[ADAPTER_CURVE_SECP256R1]);
    mbedtls_ssl_conf_min_version(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                 MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_authmode(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_VERIFY_REQUIRED);
    CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256);
    mbedtls_x509_crt_init(&g_caSslContext->ca);
    mbedtls_x509_crt_init(&g_caSslContext->crt);
    mbedtls_pk_init(&g_caSslContext->pkey);
    mbedtls_x509_crl_init(&g_caSslContext->crl);
    oc_mutex_unlock(g_sslContextMutex);

    CAsetSslAdapterCallbacks(CATCPPacketReceivedCB, CATCPPacketSendCB, CA_ADAPTER_TCP);

    CAsetPkixInfoCallback(infoCallback_that_loads_x509);

    // CAsetCredentialTypesCallback
    g_getCredentialTypesCallback = clutch;

    if (*((int*)arg) == 0)
    {
        CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM);
    }
    else if (*((int*)arg) == 1)
    {
        CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8);
    }
    else if (*((int*)arg) == 2)
    {
        CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256);
    }
    else if (*((int*)arg) == 3)
    {
        CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256);
    }
    else if (*((int*)arg) == 4)
    {
        CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384);
    }
    else if (*((int*)arg) == 5)
    {
        CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384);
    }

    CAsetPskCredentialsCallback(GetDtlsPskCredentials);

    socketConnect();

    // CAinitiateSslHandshake
    oc_mutex_lock(g_sslContextMutex);
    InitiateTlsHandshake(&serverAddr);
    oc_mutex_unlock(g_sslContextMutex);

    unsigned char buffer[2048] = {'\0'};
    int buflen = 0;
    CASecureEndpoint_t * sep = (CASecureEndpoint_t *) malloc (sizeof(CASecureEndpoint_t));
    sep->endpoint = serverAddr;

    for (int i = 0; i < 6; i++)
    {
        PacketReceive(buffer, &buflen);
        CAdecryptSsl(sep, (uint8_t *)buffer, buflen);
    }

    ret = sprintf( (char*)buffer, GET_REQUEST );

    CAencryptSsl(&serverAddr, buffer, ret);

    PacketReceive(buffer, &buflen);
    CAdecryptSsl(sep, (uint8_t *)buffer, buflen);

    CAcloseSslConnection(&serverAddr);

    // CAdeinitSslAdapter
    oc_mutex_lock(g_sslContextMutex);
    DeletePeerList();
    mbedtls_x509_crt_free(&g_caSslContext->crt);
    mbedtls_pk_free(&g_caSslContext->pkey);
    mbedtls_ssl_config_free(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_free(&g_caSslContext->serverTlsConf);
    mbedtls_ctr_drbg_free(&g_caSslContext->rnd);
    mbedtls_entropy_free(&g_caSslContext->entropy);
    OICFree(g_caSslContext);
    g_caSslContext = NULL;
    oc_mutex_unlock(g_sslContextMutex);
    oc_mutex_free(g_sslContextMutex);
    g_sslContextMutex = NULL;

    socketClose();

    if (*((int*)arg) == 0)
    {
        if (control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_len == msglen &&
            memcmp(msg, control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM,
            control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_len) == 0)
        {
            ret = 0;
        }
        else
        {
            ret = 1;
        }
    }
    if (*((int*)arg) == 1)
    {
        if (control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_8_len == msglen &&
            memcmp(msg, control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_8,
            control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_8_len) == 0)
        {
            ret = 0;
        }
        else
        {
            ret = 1;
        }
    }
    else if (*((int*)arg) == 2)
    {
        if (control_server_message_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256_len == msglen &&
            memcmp(msg, control_server_message_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
            control_server_message_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256_len) == 0)
        {
            ret = 0;
        }
        else
        {
            ret = 1;
        }
    }
    else if (*((int*)arg) == 3)
    {
        if (control_server_message_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256_len == msglen &&
            memcmp(msg, control_server_message_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
            control_server_message_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256_len) == 0)
        {
            ret = 0;
        }
        else
        {
            ret = 1;
        }
    }
    else if (*((int*)arg) == 4)
    {
        if (control_server_message_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384_len == msglen &&
            memcmp(msg, control_server_message_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
                   control_server_message_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384_len) == 0)
        {
            ret = 0;
        }
        else
        {
            ret = 1;
        }
    }
    else if (*((int*)arg) == 5)
    {
        if (control_server_message_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384_len == msglen &&
            memcmp(msg, control_server_message_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
                   control_server_message_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384_len) == 0)
        {
            ret = 0;
        }
        else
        {
            ret = 1;
        }
    }

    if (ret == 0)
    {
        *((int*)arg) = 0;
        return NULL;
    }
    else
    {
        *((int*)arg) = 0xFF;
        return (void *) 0xFF;
    }
}

// CAencryptSsl()
TEST(TLSAdaper, Test_4_0)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 0;

    ret = pthread_create( &thread1, NULL, server, (void*) NULL);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, testCAencryptSsl, &arg);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(0, arg);
}

// CAencryptSsl()
TEST(TLSAdaper, Test_4_1)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 1;

    ret = pthread_create( &thread1, NULL, server, (void*) NULL);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, testCAencryptSsl, &arg);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(0, arg);
}

// CAencryptSsl()
TEST(TLSAdaper, Test_4_2)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 2;

    ret = pthread_create( &thread1, NULL, server, (void*) NULL);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, testCAencryptSsl, &arg);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(0, arg);
}

// CAencryptSsl()
TEST(TLSAdaper, Test_4_3)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 3;

    ret = pthread_create( &thread1, NULL, server, (void*) NULL);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, testCAencryptSsl, &arg);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(0, arg);
}

// CAencryptSsl()
TEST(TLSAdaper, Test_4_4)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 4;

    ret = pthread_create( &thread1, NULL, server, (void*) NULL);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, testCAencryptSsl, &arg);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(0, arg);
}

TEST(TLSAdaper, Test_4_5)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 5;

    ret = pthread_create( &thread1, NULL, server, (void*) NULL);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, testCAencryptSsl, &arg);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(0, arg);
}

/* **************************
 *
 *
 * CAdecryptSsl test
 *
 *
 * *************************/

static void * testCAdecryptSsl(void * arg)
{
    int ret = 0;
    unsigned char buffer[2048] = {'\0'};
    int buflen = 0;

    CAEndpoint_t serverAddr;
    serverAddr.adapter = CA_ADAPTER_TCP;
    serverAddr.flags = CA_SECURE;
    serverAddr.port = 4433;
    char addr[] = {0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x00}; // 127.0.0.1
    memcpy(serverAddr.addr, addr, sizeof(addr));
    serverAddr.ifindex = 0;

    // CAinitTlsAdapter
    g_sslContextMutex = oc_mutex_new();
    oc_mutex_lock(g_sslContextMutex);
    g_caSslContext = (SslContext_t *)OICCalloc(1, sizeof(SslContext_t));
    g_caSslContext->peerList = u_arraylist_create();
    mbedtls_entropy_init(&g_caSslContext->entropy);
    mbedtls_ctr_drbg_init(&g_caSslContext->rnd);
    unsigned char * seed = (unsigned char*) SEED;
    mbedtls_ctr_drbg_seed(&g_caSslContext->rnd, mbedtls_entropy_func_clutch,
                                  &g_caSslContext->entropy, seed, sizeof(SEED));
    mbedtls_ctr_drbg_set_prediction_resistance(&g_caSslContext->rnd, MBEDTLS_CTR_DRBG_PR_OFF);
    mbedtls_ssl_config_init(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_defaults(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_psk_cb(&g_caSslContext->clientTlsConf, GetPskCredentialsCallback, NULL);
    mbedtls_ssl_conf_rng( &g_caSslContext->clientTlsConf, mbedtls_ctr_drbg_random,
                          &g_caSslContext->rnd);
    mbedtls_ssl_conf_curves(&g_caSslContext->clientTlsConf, curve[ADAPTER_CURVE_SECP256R1]);
    mbedtls_ssl_conf_min_version(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                 MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_authmode(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_VERIFY_REQUIRED);
    CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256);
    mbedtls_x509_crt_init(&g_caSslContext->ca);
    mbedtls_x509_crt_init(&g_caSslContext->crt);
    mbedtls_pk_init(&g_caSslContext->pkey);
    mbedtls_x509_crl_init(&g_caSslContext->crl);
    oc_mutex_unlock(g_sslContextMutex);

    // CAsetTlsAdapterCallbacks
    CAsetSslAdapterCallbacks(CATCPPacketReceivedCB, CATCPPacketSendCB, CA_ADAPTER_TCP);

    // CAsetPkixInfoCallback
    CAsetPkixInfoCallback(infoCallback_that_loads_x509);

    // CAsetCredentialTypesCallback
    g_getCredentialTypesCallback = clutch;

    CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM);

    CAsetPskCredentialsCallback(GetDtlsPskCredentials);

    socketConnect();

    // CAinitiateSslHandshake
    oc_mutex_lock(g_sslContextMutex);
    InitiateTlsHandshake(&serverAddr);
    oc_mutex_unlock(g_sslContextMutex);

    CASecureEndpoint_t * sep = (CASecureEndpoint_t *) malloc (sizeof(CASecureEndpoint_t));
    sep->endpoint = serverAddr;

    for (int i = 0; i < 6; i++)
    {
        PacketReceive(buffer, &buflen);
        CAdecryptSsl(sep, (uint8_t *)buffer, buflen);
    }

    ret = sprintf((char*)buffer, GET_REQUEST);

    CAencryptSsl(&serverAddr, buffer, ret);

    PacketReceive(buffer, &buflen);
    CAdecryptSsl(sep, (uint8_t *)buffer, buflen);

    CAcloseSslConnection(&serverAddr);

    // CAdeinitSslAdapter
    oc_mutex_lock(g_sslContextMutex);
    DeletePeerList();
    mbedtls_x509_crt_free(&g_caSslContext->crt);
    mbedtls_pk_free(&g_caSslContext->pkey);
    mbedtls_ssl_config_free(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_free(&g_caSslContext->serverTlsConf);
    mbedtls_ctr_drbg_free(&g_caSslContext->rnd);
    mbedtls_entropy_free(&g_caSslContext->entropy);
    OICFree(g_caSslContext);
    g_caSslContext = NULL;
    oc_mutex_unlock(g_sslContextMutex);
    oc_mutex_free(g_sslContextMutex);
    g_sslContextMutex = NULL;

    socketClose();

    if (control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_len == msglen &&
            memcmp(msg, control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM,
            control_server_message_ECDHE_ECDSA_WITH_AES_128_CCM_len) == 0)
    {
        *((int*)arg) = 0;
        return NULL;
    }
    else
    {
        *((int*)arg) = 0xFF;
        return (void *) 0xFF;
    }
}

// CAdecryptTls()
TEST(TLSAdaper, Test_5)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 1;

    ret = pthread_create( &thread1, NULL, server, (void*) NULL);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, testCAdecryptSsl, &arg);
    if(ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(0, arg);
}

/* **************************
 *
 *
 * CAdeinitSslAdapter test
 *
 *
 * *************************/

static int testCAdeinitSslAdapter()
{
    int ret = 0;
    CAEndpoint_t serverAddr;
    serverAddr.adapter = CA_ADAPTER_IP;
    serverAddr.flags = CA_SECURE;
    serverAddr.port = 4433;
    char addr[] = {0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x00}; // 127.0.0.1
    memcpy(serverAddr.addr, addr, sizeof(addr));
    serverAddr.ifindex = 0;

    // CAinitTlsAdapter
    g_sslContextMutex = oc_mutex_new();
    oc_mutex_lock(g_sslContextMutex);
    g_caSslContext = (SslContext_t *)OICCalloc(1, sizeof(SslContext_t));
    g_caSslContext->peerList = u_arraylist_create();
    mbedtls_entropy_init(&g_caSslContext->entropy);
    mbedtls_ctr_drbg_init(&g_caSslContext->rnd);
    unsigned char * seed = (unsigned char*) SEED;
    mbedtls_ctr_drbg_seed(&g_caSslContext->rnd, mbedtls_entropy_func,
                                  &g_caSslContext->entropy, seed, sizeof(SEED));
    mbedtls_ctr_drbg_set_prediction_resistance(&g_caSslContext->rnd, MBEDTLS_CTR_DRBG_PR_OFF);
    mbedtls_ssl_config_init(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_defaults(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_psk_cb(&g_caSslContext->clientTlsConf, GetPskCredentialsCallback, NULL);
    mbedtls_ssl_conf_rng( &g_caSslContext->clientTlsConf, mbedtls_ctr_drbg_random,
                          &g_caSslContext->rnd);
    mbedtls_ssl_conf_curves(&g_caSslContext->clientTlsConf, curve[ADAPTER_CURVE_SECP256R1]);
    mbedtls_ssl_conf_min_version(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                 MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_authmode(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_x509_crt_init(&g_caSslContext->ca);
    mbedtls_x509_crt_init(&g_caSslContext->crt);
    mbedtls_pk_init(&g_caSslContext->pkey);
    mbedtls_x509_crl_init(&g_caSslContext->crl);
    oc_mutex_unlock(g_sslContextMutex);

    // CAsetTlsAdapterCallbacks
    g_caSslContext->adapterCallbacks[1].recvCallback = CATCPPacketReceivedCB;
    g_caSslContext->adapterCallbacks[1].sendCallback = CATCPPacketSendCB;

    // CAsetPkixInfoCallback
    g_getPkixInfoCallback = infoCallback_that_loads_x509;

    // CAsetTlsCipherSuite
    mbedtls_ssl_conf_ciphersuites(&g_caSslContext->clientTlsConf,
                                         tlsCipher[SSL_ECDHE_ECDSA_WITH_AES_128_CCM]);
    mbedtls_ssl_conf_ciphersuites(&g_caSslContext->serverTlsConf,
                                         tlsCipher[SSL_ECDHE_ECDSA_WITH_AES_128_CCM]);
    g_caSslContext->cipher = SSL_ECDHE_ECDSA_WITH_AES_128_CCM;

    CAdeinitSslAdapter();

    if (g_caSslContext != NULL ||
        g_sslContextMutex != NULL)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    return ret;
}

// CAdeinitSslAdapter()
TEST(TLSAdaper, Test_6)
{
    int ret = 0xFF;
    ret = testCAdeinitSslAdapter();
    EXPECT_EQ(0, ret);
}

/* **************************
 *
 *
 * Server side test
 *
 *
 * *************************/

static void * testServer(void * arg)
{
    CAEndpoint_t serverAddr;
    serverAddr.adapter = CA_ADAPTER_TCP;
    serverAddr.flags = CA_SECURE;
    serverAddr.port = 4432;
    char addr[] = {0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x00}; // 127.0.0.1
    memcpy(serverAddr.addr, addr, sizeof(addr));
    serverAddr.ifindex = 0;
    unsigned char buffer[2048] = {'\0'};
    int buflen = 0;

    CAinitSslAdapter();

    CAsetSslAdapterCallbacks(CATCPPacketReceivedCB_server, CATCPPacketSendCB_server, CA_ADAPTER_TCP);
    CAsetPkixInfoCallback(infoCallback_that_loads_x509);

    // CAsetCredentialTypesCallback
    g_getCredentialTypesCallback = clutch;

    CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM);

    CAsetPskCredentialsCallback(GetDtlsPskCredentials);

    socketOpen_server();

    CASecureEndpoint_t * sep = (CASecureEndpoint_t *) malloc (sizeof(CASecureEndpoint_t));
    sep->endpoint = serverAddr;

    for (int i = 0; i < 7; i++)
    {
        PacketReceive_server(buffer, &buflen);
        CAdecryptSsl(sep, (uint8_t *)buffer, buflen);
    }

    CAcloseSslConnection(&serverAddr);

    // CAdeinitSslAdapter
    oc_mutex_lock(g_sslContextMutex);
    DeletePeerList();
    mbedtls_x509_crt_free(&g_caSslContext->crt);
    mbedtls_pk_free(&g_caSslContext->pkey);
    mbedtls_ssl_config_free(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_free(&g_caSslContext->serverTlsConf);
    mbedtls_ctr_drbg_free(&g_caSslContext->rnd);
    mbedtls_entropy_free(&g_caSslContext->entropy);
    OICFree(g_caSslContext);
    g_caSslContext = NULL;
    oc_mutex_unlock(g_sslContextMutex);
    oc_mutex_free(g_sslContextMutex);
    g_sslContextMutex = NULL;

    socketClose_server();

    if (control_client_message_len == msglen && memcmp(msg, control_client_message,
                                                            control_client_message_len) == 0)
    {
        *((int*)arg) = 0;
        return NULL;
    }
    else
    {
        *((int*)arg) = 0xFF;
        return (void *) 0xFF;
    }
}

TEST(TLSAdaper, Test_7)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 1;

    ret = pthread_create( &thread1, NULL, testServer, &arg);
    if (ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, client, (void*) NULL);
    if (ret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(NULL, arg);
}

/* **************************
 *
 * CAsetPskCredentialsCallback test
 * CAsetPkixInfoCallback test
 * CAsetCredentialTypesCallback test
 *
 * *************************/

static int testCAsetPskCredentialsCallback()
{
    static CAgetPskCredentialsHandler credCallback = (CAgetPskCredentialsHandler)dummyHandler;
    CAsetPskCredentialsCallback(credCallback);
    if (g_getCredentialsCallback == (CAgetPskCredentialsHandler)dummyHandler)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static int testCAsetPkixInfoCallback()
{
    static CAgetPkixInfoHandler infoCallback = (CAgetPkixInfoHandler)dummyHandler;
    CAsetPkixInfoCallback(infoCallback);
    if (g_getPkixInfoCallback == (CAgetPkixInfoHandler)dummyHandler)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static int testCAsetCredentialTypesCallback()
{
    static CAgetCredentialTypesHandler credTypesCallback = (CAgetCredentialTypesHandler)dummyHandler;
    CAsetCredentialTypesCallback(credTypesCallback);
    if (g_getCredentialTypesCallback == (CAgetCredentialTypesHandler)dummyHandler)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

// CAsetPskCredentialsCallback()
TEST(TLSAdaper, Test_9_0)
{
    int ret = 0xFF;
    ret = testCAsetPskCredentialsCallback();
    EXPECT_EQ(0, ret);
}
// CAsetPkixInfoCallback()
TEST(TLSAdaper, Test_9_1)
{
    int ret = 0xFF;
    ret = testCAsetPkixInfoCallback();
    EXPECT_EQ(0, ret);
}
// CAsetCredentialTypesCallback()
TEST(TLSAdaper, Test_9_2)
{
    int ret = 0xFF;
    ret = testCAsetCredentialTypesCallback();
    EXPECT_EQ(0, ret);
}

/* **************************
 *
 *
 * CAsetTlsCipherSuite test
 *
 *
 * *************************/

static int testCAsetTlsCipherSuite()
{
    int ret = 0, status = 0;
    CAEndpoint_t serverAddr;
    serverAddr.adapter = CA_ADAPTER_TCP;
    serverAddr.flags = CA_SECURE;
    serverAddr.port = 4433;
    char addr[] = {0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x00}; // 127.0.0.1
    memcpy(serverAddr.addr, addr, sizeof(addr));
    serverAddr.ifindex = 0;

    CAinitSslAdapter();

    // CAsetCredentialTypesCallback
    g_getCredentialTypesCallback = clutch;

    status = CAsetTlsCipherSuite(MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256);
    if (SSL_RSA_WITH_AES_256_CBC_SHA256 != g_caSslContext->cipher || status != CA_STATUS_OK)
    {
        ret += 1;
    }

    status = CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256);
    if (SSL_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 != g_caSslContext->cipher || status != CA_STATUS_OK)
    {
        ret += 1;
    }
    status = CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8);
    if (SSL_ECDHE_ECDSA_WITH_AES_128_CCM_8 != g_caSslContext->cipher || status != CA_STATUS_OK)
    {
        ret += 1;
    }

    status = CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM);
    if (SSL_ECDHE_ECDSA_WITH_AES_128_CCM != g_caSslContext->cipher || status != CA_STATUS_OK)
    {
        ret += 1;
    }

    status = CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256);
    if (SSL_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 != g_caSslContext->cipher || status != CA_STATUS_OK)
    {
        ret += 1;
    }

    status = CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384);
    if (SSL_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 != g_caSslContext->cipher || status != CA_STATUS_OK)
    {
        ret += 1;
    }

    status = CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384);
    if (SSL_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 != g_caSslContext->cipher || status != CA_STATUS_OK)
    {
        ret += 1;
    }

    status = CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256);
    if (SSL_ECDHE_PSK_WITH_AES_128_CBC_SHA256 != g_caSslContext->cipher || status != CA_STATUS_OK)
    {
        ret += 1;
    }

    status = CAsetTlsCipherSuite(MBEDTLS_TLS_ECDH_ANON_WITH_AES_128_CBC_SHA256);
    if (SSL_ECDH_ANON_WITH_AES_128_CBC_SHA256 != g_caSslContext->cipher || status != CA_STATUS_OK)
    {
        ret += 1;
    }

    status = CAsetTlsCipherSuite(dummyHandler);
    if (CA_STATUS_FAILED != status)
    {
        ret += 1;
    }

    // CAdeinitSslAdapter
    oc_mutex_lock(g_sslContextMutex);
    DeletePeerList();
    mbedtls_x509_crt_free(&g_caSslContext->crt);
    mbedtls_pk_free(&g_caSslContext->pkey);
    mbedtls_ssl_config_free(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_free(&g_caSslContext->serverTlsConf);
    mbedtls_ctr_drbg_free(&g_caSslContext->rnd);
    mbedtls_entropy_free(&g_caSslContext->entropy);
    OICFree(g_caSslContext);
    g_caSslContext = NULL;
    oc_mutex_unlock(g_sslContextMutex);
    oc_mutex_free(g_sslContextMutex);
    g_sslContextMutex = NULL;

    return ret;
}

// CAinitTlsAdapter()
TEST(TLSAdaper, Test_10)
{
    int ret = 0xff;
    ret = testCAsetTlsCipherSuite();
    EXPECT_EQ(0, ret);
}

static void * testCAsslGenerateOwnerPsk(void * arg)
{
    int ret = 0;
    CAEndpoint_t serverAddr;
    serverAddr.adapter = CA_ADAPTER_TCP;
    serverAddr.flags = CA_SECURE;
    serverAddr.port = 4433;
    char addr[] = {0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x00}; // 127.0.0.1
    memcpy(serverAddr.addr, addr, sizeof(addr));
    serverAddr.ifindex = 0;

    uint8_t label[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t rsrcServerDeviceId[] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A};
    uint8_t provServerDeviceId[] = {0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A};
    uint8_t ownerPsk[0x100] = {0x00};
    uint8_t predictedPSK[] = {
        0xba, 0x72, 0x16, 0xbc, 0x7f, 0x8c, 0xfe, 0xfc, 0xd0, 0xac, 0x1a, 0x37, 0xad, 0x60, 0xe8, 0x9e,
        0xb3, 0x31, 0xa2, 0x30, 0xaf, 0x68, 0xc9, 0xa6, 0x89, 0x8a, 0x04, 0x21, 0x6c, 0xbd, 0x04, 0x08,
        0x68, 0x11, 0x54, 0x9e, 0x2a, 0x10, 0x91, 0x94, 0x3c, 0x44, 0x52, 0xc7, 0xfa, 0x78, 0x44, 0x87,
        0xea, 0x30, 0x08, 0x5f, 0xc1, 0x64, 0xaa, 0x0d, 0xfd, 0x84, 0x16, 0x83, 0x20, 0xc9, 0x08, 0x65,
        0xd2, 0x4a, 0x55, 0x9e, 0x8f, 0x88, 0x3c, 0x57, 0x10, 0xbd, 0x5a, 0x30, 0x01, 0xb4, 0x59, 0x63,
        0x64, 0x19, 0x8d, 0xfa, 0x5c, 0x86, 0x92, 0xf7, 0x60, 0x99, 0xdb, 0xae, 0x0e, 0xad, 0x80, 0xf1,
        0x82, 0xaf, 0x1b, 0x14, 0x0c, 0x99, 0x13, 0x53, 0x54, 0x33, 0x6a, 0x17, 0x24, 0x5c, 0x9d, 0xdb,
        0x5a, 0xfb, 0x73, 0x2f, 0x41, 0xe8, 0xeb, 0x2e, 0x68, 0xfe, 0xee, 0x0b, 0xdc, 0x54, 0x50, 0xf1,
        0x1e, 0x16, 0x19, 0x2c, 0x4e, 0xb6, 0x97, 0x9f, 0x9c, 0x32, 0x9c, 0x0e, 0xe0, 0xe1, 0x32, 0x64,
        0x16, 0x34, 0x53, 0x8e, 0xc5, 0xe3, 0xe5, 0xbc, 0x2c, 0x10, 0xae, 0x81, 0x2c, 0x1a, 0xb2, 0xb7,
        0xa3, 0xbe, 0x0f, 0xab, 0xfd, 0xf7, 0x87, 0x53, 0xcd, 0x3e, 0x31, 0xfb, 0x2d, 0x69, 0x6a, 0xd5,
        0xc3, 0x27, 0x04, 0x2b, 0x37, 0x02, 0x91, 0x05, 0x0c, 0x4e, 0x2a, 0xfc, 0x6c, 0x42, 0xe8, 0x37,
        0x23, 0x2f, 0x60, 0x6e, 0x0c, 0xed, 0x7c, 0xe0, 0x5f, 0x47, 0xb3, 0x51, 0x86, 0x5b, 0x26, 0x08,
        0x2a, 0x05, 0x89, 0xb0, 0xdd, 0x6f, 0xc6, 0x76, 0xc5, 0x2a, 0x60, 0x07, 0x0e, 0xb1, 0x71, 0x67,
        0x21, 0x11, 0xf8, 0xb5, 0x52, 0xa3, 0xf3, 0xf0, 0xd4, 0x5f, 0xdf, 0x44, 0x66, 0x23, 0xd8, 0x4e,
        0xbd, 0x64, 0x39, 0x43, 0x03, 0x37, 0xaa, 0xd7, 0xea, 0xb3, 0x6d, 0x2f, 0x84, 0x9c, 0x02, 0x49
    };

    // CAinitTlsAdapter
    g_sslContextMutex = oc_mutex_new();
    oc_mutex_lock(g_sslContextMutex);
    g_caSslContext = (SslContext_t *)OICCalloc(1, sizeof(SslContext_t));
    g_caSslContext->peerList = u_arraylist_create();
    mbedtls_entropy_init(&g_caSslContext->entropy);
    mbedtls_ctr_drbg_init(&g_caSslContext->rnd);
    unsigned char * seed = (unsigned char*) SEED;
    mbedtls_ctr_drbg_seed(&g_caSslContext->rnd, mbedtls_entropy_func_clutch,
                                  &g_caSslContext->entropy, seed, sizeof(SEED));
    mbedtls_ctr_drbg_set_prediction_resistance(&g_caSslContext->rnd, MBEDTLS_CTR_DRBG_PR_OFF);
    mbedtls_ssl_config_init(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_defaults(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_psk_cb(&g_caSslContext->clientTlsConf, GetPskCredentialsCallback, NULL);
    mbedtls_ssl_conf_rng( &g_caSslContext->clientTlsConf, mbedtls_ctr_drbg_random,
                          &g_caSslContext->rnd);
    mbedtls_ssl_conf_curves(&g_caSslContext->clientTlsConf, curve[ADAPTER_CURVE_SECP256R1]);
    mbedtls_ssl_conf_min_version(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_MAJOR_VERSION_3,
                                 MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_authmode(&g_caSslContext->clientTlsConf, MBEDTLS_SSL_VERIFY_REQUIRED);
    CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256);
    mbedtls_x509_crt_init(&g_caSslContext->ca);
    mbedtls_x509_crt_init(&g_caSslContext->crt);
    mbedtls_pk_init(&g_caSslContext->pkey);
    mbedtls_x509_crl_init(&g_caSslContext->crl);
    oc_mutex_unlock(g_sslContextMutex);

    // CAsetTlsAdapterCallbacks
    CAsetSslAdapterCallbacks(CATCPPacketReceivedCB, CATCPPacketSendCB, CA_ADAPTER_TCP);

    // CAsetPkixInfoCallback
    CAsetPkixInfoCallback(infoCallback_that_loads_x509);

    // CAsetCredentialTypesCallback
    g_getCredentialTypesCallback = clutch;

    CAsetTlsCipherSuite(MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256);

    CAsetPskCredentialsCallback(GetDtlsPskCredentials);

    socketConnect();

    // CAinitiateSslHandshake
    oc_mutex_lock(g_sslContextMutex);
    InitiateTlsHandshake(&serverAddr);
    oc_mutex_unlock(g_sslContextMutex);

    ret = CAsslGenerateOwnerPsk(&serverAddr,
          label, sizeof(label),
          rsrcServerDeviceId, sizeof(rsrcServerDeviceId),
          provServerDeviceId, sizeof(provServerDeviceId),
          ownerPsk, 0x100);

    // CAcloseTlsConnection
    oc_mutex_lock(g_sslContextMutex);
    SslEndPoint_t * tep = GetSslPeer(&serverAddr);
    mbedtls_ssl_close_notify(&tep->ssl);
    RemovePeerFromList(&tep->sep.endpoint);
    oc_mutex_unlock(g_sslContextMutex);

    // CAdeinitTlsAdapter
    oc_mutex_lock(g_sslContextMutex);
    DeletePeerList();
    mbedtls_x509_crt_free(&g_caSslContext->crt);
    mbedtls_pk_free(&g_caSslContext->pkey);
    mbedtls_ssl_config_free(&g_caSslContext->clientTlsConf);
    mbedtls_ssl_config_free(&g_caSslContext->serverTlsConf);
    mbedtls_ctr_drbg_free(&g_caSslContext->rnd);
    mbedtls_entropy_free(&g_caSslContext->entropy);
    OICFree(g_caSslContext);
    g_caSslContext = NULL;
    oc_mutex_unlock(g_sslContextMutex);
    oc_mutex_free(g_sslContextMutex);
    g_sslContextMutex = NULL;

    socketClose();

    if (ret == 0 && memcmp(predictedPSK, ownerPsk, sizeof(predictedPSK)) == 0)
    {
        *((int*)arg) = 0;
        return NULL;
    }
    else
    {
        *((int*)arg) = 0xFF;
        return (void *) 0xFF;
    }
}

TEST(TLSAdaper, Test_11)
{
    pthread_t thread1, thread2;
    int ret = 0;
    int arg = 1;

    ret = pthread_create( &thread1, NULL, server, (void*) NULL);
    if(ret)
    {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    ret = pthread_create( &thread2, NULL, testCAsslGenerateOwnerPsk, &arg);
    if(ret)
    {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
        exit(EXIT_FAILURE);
    }

    sleep(5);

    EXPECT_EQ(0, arg);
}
