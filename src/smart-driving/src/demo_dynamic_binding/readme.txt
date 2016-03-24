/**
 *
 * Demy Dynamic Bindings.
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
 * \page page_demo_dynamic_bindings Demo Server and Client Filter
 *
 * This Examples shows how to implement a Server Filter and a Client Filter.
 * Within ADTF it is called Dynamic Binding: see @ref page_adtf_dynamic_binding
 * 
 * \par Location
 * \code
 *    ./src/examples/src/filters/demo_dynamic_binding/demo_client/
 *    ./src/examples/src/filters/demo_dynamic_binding/demo_server/
 * \endcode
 *
 * \par Build Environment
 * To see how to set up the build environment have a look at this page @ref page_cmake_overview
 *
 * \par This example shows:
 * \li how to implement a adtf dynamic filter for implementing an interface and providing as Server
 * \li how to implement a adtf dynamic filter for implementing a client filter and subscribe for a provided interface via dynamic binding
 * \li how to use and configure th interface 
 *
 * \par Call Sequence
 * For better understanding the sequence of calls while using base device classes of ADTF SDK the 
 * device implementations are added to the installation as cpp-file.\n
 * So you can debug through sequence.
 * 
 * The filters of this example will implement and use following interface:
 * \include demo_published_interface.h
 * 
 * \par The Header for the Demo Server Filter 
 * \include demo_server_filter.h
 *
 * \par The Implementation Demo Server Filter 
 * \include demo_server_filter.cpp
 *
 * \par The Header for the Demo Client Filter 
 * \include demo_client_filter.h
 *
 * \par The Implementation Demo Server Filter 
 * \include demo_client_filter.cpp
 *
 */