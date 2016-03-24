# execute just once
if(AADC_FOUND)
    message(STATUS "AADC config file already found.")
    return()
endif(AADC_FOUND)

set(AADC_OPENCV_FOUND FALSE)
set(AADC_OPENNI2_FOUND FALSE)
set(AADC_OSG_FOUND FALSE)
set(AADC_ARUCO_FOUND FALSE)

#------------------------------------------------------
#-------OpenCV------------------------------------------
#-------------------------------------------------------
find_package(OpenCV REQUIRED  PATHS 
            "/opt/opencv/3.0.0"
            "C:/SDK/opencv/3.0.0/build/x64/vc10/lib")

if(NOT OpenCV_LIBS)
    message(FATAL_ERROR "OpenCV lib not found. Please specify the OPENCV_DIR")
else(NOT OpenCV_LIBS)
    set(OpenCV_INCLUDE_DIR ${OpenCV_INCLUDE_DIRS})
    message(STATUS "OpenCV lib found. OpenCV Version is ${OpenCV_VERSION}")
    set(AADC_OPENCV_FOUND TRUE)
    if(NOT UNIX)
        set(OpenCV_INCLUDE_DIR ${OpenCV_INCLUDE_DIRS})
    endif(NOT UNIX)		
endif(NOT OpenCV_LIBS)

#--------OpenNI2----------------------------------------    
# OpenNI does not provide a CMake config file.
# So we have to set it from the outside.
if(UNIX)
    set(OPENNI2_DIR "/opt/openni/2.3.0" CACHE PATH "The root directory of OpenNI2")
else(UNIX)
    set(OPENNI2_DIR "C:/SDK/OpenNI2" CACHE PATH "The root directory of OpenNI2")
endif(UNIX)

FIND_LIBRARY(OPENNI2_LIBS
                NAMES 
                    OpenNI2
                PATHS 
                    "${OPENNI2_DIR}/Bin/x64-Release"
                PATH_SUFFIXES so
                )

if(NOT OPENNI2_LIBS)
    message(FATAL_ERROR "OpenNI2 lib not found. Please specify the OPENNI2_DIR")
else(NOT OPENNI2_LIBS)    
    set(AADC_OPENNI2_FOUND TRUE)
    set(OPENNI2_INCLUDE_DIR "${OPENNI2_DIR}/Include/" CACHE PATH "The include directory of OpenNI2") 
    message("OpenNI2 lib found. OPENNI2_INCLUDE_DIR is ${OPENNI2_INCLUDE_DIR}")
endif(NOT OPENNI2_LIBS)                
            

#-------------------------------------------------------
#--------aruco------------------------------------------
#-------------------------------------------------------
find_package(aruco REQUIRED PATHS 
				"/opt/aruco/1.3.0/lib"	
				"C:/SDK/aruco/1.3.0/lib/cmake"
				CONFIGS
				Findaruco.cmake)	


if(NOT aruco_LIBS)
	message(FATAL_ERROR "aruco lib not found. Please specify the ARUCO_DIR.")
else(NOT aruco_LIBS)
	set(AADC_ARUCO_FOUND TRUE)
	message("aruco lib found. ARUCO Version  is ${aruco_VERSION}")
endif(NOT aruco_LIBS)	

#------------------------------------------------------------------	
#---------adtf display toolbox-------------------------------------
#------------------------------------------------------------------
set(ADTF_DISPLAY_TOOLBOX_DIR "${ADTF_DIR}/addons/adtf-display-toolbox" CACHE PATH "The ADTF disply toolbox dir")
find_package(ADTF_DISPLAY_TOOLBOX
                PATHS
                ${ADTF_DISPLAY_TOOLBOX_DIR})
if(ADTF_DISPLAY_TOOLBOX_FOUND)
	message("-- Found DisplayToolbox: ${ADTF_DISPLAY_TOOLBOX_VERSION}")
endif(ADTF_DISPLAY_TOOLBOX_FOUND)

#------------------------------------------------------------------	
#--------open scene graph------------------------------------------
#------------------------------------------------------------------	

#find package is not working
#find_package(OpenSceneGraph 3.2.0 REQUIRED osgUtil)
#message("OSG: ${OSG_FOUND}")
if(UNIX)
	set(OSG_DIR "/opt/osg/3.2.0" CACHE PATH "The OSG dir")
	set(OSG_LIBRARY_DIR "${OSG_DIR}/lib64/")
else(UNIX)
	set(OSG_DIR "C:/SDK/Osg/3.2.0" CACHE PATH "The OSG dir")
	set(OSG_LIBRARY_DIR "${OSG_DIR}/lib/")
endif(UNIX)

set(OSG_INCLUDE_DIR "${OSG_DIR}/include/")

FIND_LIBRARY(OSG_LIBRARIES_RELEASE NAMES 
				osg
				osgUtil
				OpenThreads
				osgAnimation
				osgDB
				osgFX
				PATHS 
				"${OSG_LIBRARY_DIR}"
					)
					
FIND_LIBRARY(OSG_LIBRARIES_DEBUG NAMES 
				osgd
				osgUtild
				OpenThreadsd
				osgAnimationd
				osgDBd
				osgFXd						 
				PATHS 
				"${OSG_LIBRARY_DIR}")
if(NOT OSG_LIBRARIES_RELEASE AND NOT OSG_LIBRARIES_DEBUG)
	message(FATAL_ERROR "OSG lib not found. Please specify the OSG_DIR.")
else(NOT OSG_LIBRARIES_RELEASE AND NOT OSG_LIBRARIES_DEBUG)
	set(AADC_OSG_FOUND TRUE)
	message("OSG lib found. OSG_DIR is ${OSG_DIR}")		
endif(NOT OSG_LIBRARIES_RELEASE AND NOT OSG_LIBRARIES_DEBUG)

#--------------------------------------------------
if(NOT AADC_OPENCV_FOUND OR NOT AADC_OPENNI2_FOUND OR NOT AADC_ARUCO_FOUND OR NOT AADC_OSG_FOUND)
    message(FATAL_ERROR "At least one of the required libraries is not found")
endif(NOT AADC_OPENCV_FOUND OR NOT AADC_OPENNI2_FOUND OR NOT AADC_ARUCO_FOUND OR NOT AADC_OSG_FOUND)
