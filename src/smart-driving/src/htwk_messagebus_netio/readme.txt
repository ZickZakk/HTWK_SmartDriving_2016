 /**
 *
 * Demo Message Bus NetIO Filter
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved.
 *
 * @author               $Author: voigtlpi $
 * @date                 $Date: 2009-07-16 15:36:37 +0200 (Do, 16 Jul 2009) $
 * @version              $Revision: 10093 $
 *
 * @remarks
 *
 */
 
/**
 * \page page_demo_messagebus_netio Demo Message Bus NetIO Filter
 *
 * Implements a udp filter
 * 
 * \par Location
 * \code
 *    ./src/examples/src/filters/demo_messagebus_netio/
 * \endcode
 *
 * \par Build Environment
 * To see how to set up the build environment have a look at this page @ref page_cmake_overview
 *
 * \par This example shows:
 * \li how to implement a common adtf filter for create a message bus channel within code
 * \li the possibility to meassure the real messagebus capacity
 * \li how to send or receive data over the messagebus
 *
 * 
 * \par The Header for the Demo Message Bus NetIO Filter
 * \include demonetiofilter.h
 *
 * \par The Implementation the Demo Message Bus NetIO Filter
 * \include demonetiofilter.cpp
 * 
 */