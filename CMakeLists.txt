set(PROJECT_NAME Smartik)

################################################################################
# Source groups
################################################################################

set(Resources
        "RFIX.WIN/${PROJECT_NAME}.rc2"
        "RFIX/${PROJECT_NAME}Fix.grc"
        "RINT/${PROJECT_NAME}.grc"
)
source_group("Resources" FILES ${Resources})

set(Resources__Images
        "RFIX/Images/MenuIcon_18x18.svg"
)
source_group("Resources\\Images" FILES ${Resources__Images})

set(Src
        "Src/${PROJECT_NAME}.c"
        "Src/APICommon.c"
)
source_group("Src" FILES ${Src})



set(Src__Headers
        "Src/APICommon.h"
        "Src/APIEnvir.h"
        "Src/ColRec.h"
        "Src/CurWallRec.h"
        "Src/DoorRec.h"
        "Src/Extras.h"
        "Src/FloorRec.h"
        "Src/GeoAnalysis.h"
        "Src/Numberer.h"
        "Src/Polygons.h"
        "Src/RecHandler.h"
        "Src/ResourceIDs.h"
        "Src/Resources.hpp"
        "Src/Room.h"
        "Src/RoomRec.h"
        "Src/TmrTech.h"
        "Src/WallMatch.h"
        "Src/WallRec.h"
        "Src/WindowRec.h"
)
source_group("Src\\Headers" FILES ${Src__Headers})



set(Src__Recognition__2DTo3D
        "Src/ColRec.cpp"
        "Src/CurWallRec.cpp"
        "Src/DoorRec.cpp"
        "Src/RecHandler.cpp"
        "Src/WallMatch.cpp"
        "Src/WallRec.cpp"
        "Src/WindowRec.cpp"
)
source_group("Src\\Recognition\\2DTo3D" FILES ${Src__Recognition__2DTo3D})

set(Src__Recognition__Floor
        "Src/FloorRec.cpp"
)
source_group("Src\\Recognition\\Floor" FILES ${Src__Recognition__Floor})

set(Src__Recognition__Room
        "Src/Room.cpp"
        "Src/RoomRec.cpp"
)
source_group("Src\\Recognition\\Room" FILES ${Src__Recognition__Room})


set(Src__Tech
        "Src/Extras.cpp"
        "Src/GeoAnalysis.cpp"
        "Src/Polygons.cpp"
        "Src/Resources.cpp"
        "Src/TmrTech.cpp"
)
source_group("Src\\Tech" FILES ${Src__Tech})


set(Src__Tools__Numberer
        "Src/Numberer.cpp"
)
source_group("Src\\Tools\\Numberer" FILES ${Src__Tools__Numberer})

set(ALL_FILES
        ${no_group_source_files}
        ${Resources}
        ${Resources__Images}
        ${Src}
        ${Src__Export}
        ${Src__Headers}
        ${Src__Navigator}
        ${Src__Recognition__2DTo3D}
        ${Src__Recognition__Floor}
        ${Src__Recognition__Room}
        ${Src__Render}
        ${Src__Tech}
        ${Src__Texturer}
        ${Src__Tools}
        ${Src__Tools__Floors}
        ${Src__Tools__Level_Dimensions}
        ${Src__Tools__Numberer}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUG   "${PROJECT_NAME}"
        TARGET_NAME_RELEASE "${PROJECT_NAME}"
)

################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/Build/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/Build/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>"
)

################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT MSVC_RUNTIME_LIBRARY_STR
        $<$<CONFIG:Debug>:MultiThreadedDebugDLL>
        $<$<CONFIG:Release>:MultiThreadedDLL>
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
        "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/../../../Support>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Inc"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/GSRoot"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/GSRoot/STL/imp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/GSUtils"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/DGLib"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/GX"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/GXImage"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/GXImageBase"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/DGGraphix"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/Graphix"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/TextEngine"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/InputOutput"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/UCLib"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/UDLib"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/Pattern"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../Support/Modules/VBAttrDialogs"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:_DEBUG>"
        "$<$<CONFIG:Release>:NDEBUG>"
        "WIN32"
        "WINDOWS"
        "_WINDOWS"
        "_USRDLL"
        "_STLP_DONT_FORCE_MSVC_LIB_NAME"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:/Od;/RTC1;/Gy;/ZI>
            $<$<CONFIG:Release>:/O2;/Zi>
            /Gm-;/GF;/Gr;/W4;/nologo;/WX;/TP;/GR;/wd4996;/GS;/Zc:wchar_t-;/Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:/DEBUG;/INCREMENTAL>
            $<$<CONFIG:Release>:/DEBUG:FULL;/OPT:REF;/OPT:ICF;/INCREMENTAL:NO>
            /MACHINE:X64;/export:GetExportedFuncAddrs,%401;/export:SetImportedFuncAddrs,%402
    )
endif()
