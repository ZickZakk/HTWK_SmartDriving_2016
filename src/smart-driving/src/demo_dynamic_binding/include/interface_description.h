/**
 *
 * Struct Definitions for the Demo Server Client example
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: A1RHEND $
 * $Date: 2013-02-20 11:04:57 +0100 (Mi, 20 Feb 2013) $
 * $Revision: 37066 $
 *
 * @remarks
 *
 */

#ifndef _HEADER_PLUGIN_DEMO_SERVERCLIENT_
#define _HEADER_PLUGIN_DEMO_SERVERCLIENT_

//simple_ui32_raw
#define MNAME_I32 "tI32"
#define MNAME_I32_DESCRIPTION "<struct name=\"tI32\" version=\"1\" alignment=\"1\">" \
                                            "<element name=\"value\" type=\"tInt32\" bytepos=\"0\" alignment=\"1\" arraysize=\"1\" byteorder=\"BE\"/>" \
                                         "</struct>"

//simple_struct_raw
#define MNAME_STRUCT              "tPublishedDataStruct"
#define MNAME_STRUCT_DESCRIPTION  "<struct name=\"tPublishedDataStruct\" version=\"1\" alignment=\"4\">" \
                                      "<element name=\"i8Value\" type=\"tInt8\" bytepos=\"0\" alignment=\"1\" arraysize=\"1\" byteorder=\"LE\"/>" \
                                      "<element name=\"ui32Value\" type=\"tUInt32\" bytepos=\"1\" alignment=\"1\" arraysize=\"1\" byteorder=\"LE\"/>" \
                                  "</struct>"

//simple_struct_raw
#define MNAME_STATE              "tDemoServerState"
#define MNAME_STATE_DESCRIPTION  "<struct name=\"tDemoServerState\" version=\"1\" alignment=\"4\">" \
                                      "<element name=\"i32ActivateCount\" type=\"tInt32\" bytepos=\"0\" alignment=\"1\" arraysize=\"1\" byteorder=\"LE\"/>" \
                                      "<element name=\"ui8ServerState\" type=\"tUInt8\" bytepos=\"4\" alignment=\"1\" arraysize=\"1\" byteorder=\"LE\"/>" \
                                      "<element name=\"i64ProcessCallCounter\" type=\"tInt64\" bytepos=\"5\" alignment=\"4\" arraysize=\"1\" byteorder=\"LE\"/>" \
                                      "<element name=\"i64TransmitCounter\" type=\"tInt64\" bytepos=\"13\" alignment=\"4\" arraysize=\"1\" byteorder=\"LE\"/>" \
                                  "</struct>"

#endif //_HEADER_PLUGIN_DEMO_SERVERCLIENT_
