set(SOURCE
    ogrdgnv8datasource.cpp
    ogrdgnv8driver.cpp
    ogrdgnv8layer.cpp
    ogrdwg_blockmap.cpp
    ogrdwg_dimension.cpp
    ogrdwg_hatch.cpp
    ogrdwgblockslayer.cpp
    ogrdwgdatasource.cpp
    ogrdwgdriver.cpp
    ogrdwglayer.cpp
    ogrteigha.cpp)
add_gdal_driver(TARGET ogr_DWG_DGNV8
                SOURCES ${SOURCE}
                CORE_SOURCES
                      ogrdgnv8drivercore.cpp
                      ogrdwgdrivercore.cpp
                DRIVER_NAME_OPTION DWG
                SKIP_GDAL_PRIV_HEADER  # because of issue with _REENTRANT
                PLUGIN_CAPABLE
                NO_SHARED_SYMBOL_WITH_CORE
  NO_CXX_WFLAGS_EFFCXX
  NO_WFLAG_OLD_STYLE_CAST
)
if(NOT TARGET ogr_DWG_DGNV8)
    return()
endif()

if (OGR_ENABLE_DRIVER_DWG_PLUGIN)
  # a bit ugly but we need those symbols from the DXF driver Note: we need to use target_sources() to add them, and not
  # edit the SOURCE variable, as the OGR_ENABLE_DRIVER_DWG_PLUGIN variable is automatically created only after
  # add_gdal_driver()
  target_sources(ogr_DWG_DGNV8 PRIVATE ../dxf/ogr_autocad_services.cpp ../dxf/ogrdxf_polyline_smooth.cpp
                                       ../dxf/intronurbs.cpp)
else ()
  # Another hack for the non-plugin use case
  target_compile_definitions(ogrsf_frmts PRIVATE -DDGNV8_ENABLED)
endif ()

gdal_standard_includes(ogr_DWG_DGNV8)
target_include_directories(ogr_DWG_DGNV8 PRIVATE $<TARGET_PROPERTY:ogr_DXF,SOURCE_DIR>)
gdal_target_link_libraries(ogr_DWG_DGNV8 PRIVATE TEIGHA::TEIGHA)
if (Threads_FOUND)
  gdal_target_link_libraries(ogr_DWG_DGNV8 PRIVATE Threads::Threads)
endif ()

option(BUILD_CREATEDGNV8TESTFILE "Build test program createdgnv8testfile" ON)
mark_as_advanced(BUILD_CREATEDGNV8TESTFILE)
if (BUILD_CREATEDGNV8TESTFILE)
  add_executable(createdgnv8testfile createdgnv8testfile.cpp)
  target_link_libraries(createdgnv8testfile PRIVATE TEIGHA::TEIGHA)
endif ()
