/**
 *
 * ADTF external data exchange communication example
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved.
 *
 * @author               $Author: AHAUSMA $
 * @date                 $Date: 2012-11-21 11:20:50 +0100 (Wed, 21 Nov 2012) $
 * @version              $Revision: 35564 $
 *
 * @remarks
 *
 */
 
/**
 * \page page_example_dataexchange_raw Demo ADTF external data exchange communication
 *
 * This example communicates from outside ADTF over the Messagebus/Dataexchange to an ADTF.
 *          It uses the UDP communication protocol. For detailed information please have a look into 
 *          ADTF-SDK Documentation  (ADTF Extreme Programmers). To use the example you have set  
 *          the destination IP and the Port-Address as parameter. On the other side, you need a running ADTF with an Inport named 
 *          \b state_raw you will need a filter that reacts on received Data. Such an example Filter can be found in the receive_filter 
 * 
 * \par Location
 * \code
 *    ./src/examples/src/misc/dataexchange_raw
 * \endcode
 *
 * \par Build Environment
 * To see how to set up the build environment have a look at this page @ref page_cmake_overview
 *
 * \par This example shows:
 * \li how to send data from an external programm over the MessageBus
 *
 * \par The Implementation for Demo ADTF external data exchange communication
 * \include dataexchange_raw.cpp
 *
 *
 */