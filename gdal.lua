project "gdal"

  local prj = project()
  local prjDir = prj.basedir

  -- -------------------------------------------------------------
  -- project
  -- -------------------------------------------------------------

  -- common project settings

  dofile (_BUILD_DIR .. "/3rdparty_static_project.lua")

  -- project specific settings

  uuid "B6ED8CCA-6592-4807-BFFD-247A379C395B"

  flags {
    "NoPCH",
  }

  files {
    -- "gdal/alg/contour.cpp",
    "gdal/alg/delaunay.c",
    "gdal/alg/gdalapplyverticalshiftgrid.cpp",
    -- "gdal/alg/gdalchecksum.cpp",
    "gdal/alg/gdal_crs.c",
    "gdal/alg/gdalcutline.cpp",
    "gdal/alg/gdaldither.cpp",
    "gdal/alg/gdalgeoloc.cpp",
    -- "gdal/alg/gdalgridavx.cpp",
    -- "gdal/alg/gdalgrid.cpp",
    -- "gdal/alg/gdalgridsse.cpp",
    "gdal/alg/gdallinearsystem.cpp",
    -- "gdal/alg/gdalmatching.cpp",
    "gdal/alg/gdalmediancut.cpp",
    -- "gdal/alg/gdal_octave.cpp",
    "gdal/alg/gdalpansharpen.cpp",
    -- "gdal/alg/gdalproximity.cpp",
    "gdal/alg/gdalrasterize.cpp",
    -- "gdal/alg/gdalrasterpolygonenumerator.cpp",
    "gdal/alg/gdal_rpc.cpp",
    -- "gdal/alg/gdalsievefilter.cpp",
    -- "gdal/alg/gdal_simplesurf.cpp",
    -- "gdal/alg/gdalsimplewarp.cpp",
    "gdal/alg/gdal_tps.cpp",
    "gdal/alg/gdaltransformer.cpp",
    -- "gdal/alg/gdaltransformgeolocs.cpp",
    "gdal/alg/gdalwarper.cpp",
    "gdal/alg/gdalwarpkernel.cpp",
    -- "gdal/alg/gdalwarpkernel_opencl.cpp",
    "gdal/alg/gdalwarpoperation.cpp",
    "gdal/alg/llrasterize.cpp",
    -- "gdal/alg/polygonize.cpp",
    -- "gdal/alg/rasterfill.cpp",
    "gdal/alg/thinplatespline.cpp",
    "gdal/apps/commonutils.cpp",
    -- "gdal/apps/gdaladdo.cpp",
    -- "gdal/apps/gdalbuildvrt_bin.cpp",
    "gdal/apps/gdalbuildvrt_lib.cpp",
    -- "gdal/apps/gdal_contour.cpp",
    -- "gdal/apps/gdaldem_bin.cpp",
    -- "gdal/apps/gdaldem_lib.cpp",
    -- "gdal/apps/gdalenhance.cpp",
    -- "gdal/apps/gdal_grid_bin.cpp",
    -- "gdal/apps/gdal_grid_lib.cpp",
    -- "gdal/apps/gdalinfo_bin.cpp",
    -- "gdal/apps/gdalinfo_lib.cpp",
    -- "gdal/apps/gdallocationinfo.cpp",
    -- "gdal/apps/gdalmanage.cpp",
    -- "gdal/apps/gdal_rasterize_bin.cpp",
    -- "gdal/apps/gdal_rasterize_lib.cpp",
    -- "gdal/apps/gdalserver.cpp",
    -- "gdal/apps/gdalsrsinfo.cpp",
    -- "gdal/apps/gdaltindex.cpp",
    -- "gdal/apps/gdaltransform.cpp",
    -- "gdal/apps/gdal_translate_bin.cpp",
    -- "gdal/apps/gdal_translate_lib.cpp",
    -- "gdal/apps/gdalwarp_bin.cpp",
    -- "gdal/apps/gdalwarp_lib.cpp",
    -- "gdal/apps/gnmanalyse.cpp",
    -- "gdal/apps/gnmmanage.cpp",
    -- "gdal/apps/nearblack_bin.cpp",
    -- "gdal/apps/nearblack_lib.cpp",
    -- "gdal/apps/ogr2ogr_bin.cpp",
    "gdal/apps/ogr2ogr_lib.cpp",
    -- "gdal/apps/ogrinfo.cpp",
    -- "gdal/apps/ogrlineref.cpp",
    -- "gdal/apps/ogrtindex.cpp",
    -- "gdal/apps/testepsg.cpp",
    -- "gdal/frmts/aaigrid/aaigriddataset.cpp",
    "gdal/frmts/adrg/adrgdataset.cpp",
    "gdal/frmts/adrg/srpdataset.cpp",
    -- "gdal/frmts/aigrid/aigccitt.c",
    -- "gdal/frmts/aigrid/aigdataset.cpp",
    -- "gdal/frmts/aigrid/aigopen.c",
    -- "gdal/frmts/aigrid/gridlib.c",
    -- "gdal/frmts/airsar/airsardataset.cpp",
    -- "gdal/frmts/arg/argdataset.cpp",
    -- "gdal/frmts/blx/blx.c",
    -- "gdal/frmts/blx/blxdataset.cpp",
    -- "gdal/frmts/bmp/bmpdataset.cpp",
    -- "gdal/frmts/bsb/bsbdataset.cpp",
    -- "gdal/frmts/bsb/bsb_read.c",
    -- "gdal/frmts/cals/calsdataset.cpp",
    -- "gdal/frmts/ceos2/ceos.c",
    -- "gdal/frmts/ceos2/ceosrecipe.c",
    -- "gdal/frmts/ceos2/ceossar.c",
    -- "gdal/frmts/ceos2/link.c",
    -- "gdal/frmts/ceos2/sar_ceosdataset.cpp",
    -- "gdal/frmts/ceos/ceosdataset.cpp",
    -- "gdal/frmts/ceos/ceosopen.c",
    -- "gdal/frmts/coasp/coasp_dataset.cpp",
    -- "gdal/frmts/cosar/cosar_dataset.cpp",
    -- "gdal/frmts/ctg/ctgdataset.cpp",
    "gdal/frmts/derived/deriveddataset.cpp",
    "gdal/frmts/derived/derivedlist.c",
    -- "gdal/frmts/dimap/dimapdataset.cpp",
    "gdal/frmts/dted/dted_api.c",
    "gdal/frmts/dted/dted_create.c",
    "gdal/frmts/dted/dteddataset.cpp",
    "gdal/frmts/dted/dted_ptstream.c",
    -- "gdal/frmts/e00grid/e00griddataset.cpp",
    -- "gdal/frmts/elas/elasdataset.cpp",
    -- "gdal/frmts/envisat/adsrange.cpp",
    -- "gdal/frmts/envisat/envisatdataset.cpp",
    -- "gdal/frmts/envisat/EnvisatFile.c",
    -- "gdal/frmts/envisat/records.c",
    -- "gdal/frmts/envisat/unwrapgcps.cpp",
    -- "gdal/frmts/ers/ersdataset.cpp",
    -- "gdal/frmts/ers/ershdrnode.cpp",
    -- "gdal/frmts/fit/fit.cpp",
    -- "gdal/frmts/fit/fitdataset.cpp",
    "gdal/frmts/gdalallregister.cpp",
    -- "gdal/frmts/gff/gff_dataset.cpp",
    "gdal/frmts/gif/biggifdataset.cpp",
    "gdal/frmts/gif/gifabstractdataset.cpp",
    "gdal/frmts/gif/gifdataset.cpp",
    -- "gdal/frmts/gsg/gs7bgdataset.cpp",
    -- "gdal/frmts/gsg/gsagdataset.cpp",
    -- "gdal/frmts/gsg/gsbgdataset.cpp",
    "gdal/frmts/gtiff/geotiff.cpp",
    "gdal/frmts/gtiff/gt_citation.cpp",
    "gdal/frmts/gtiff/gt_jpeg_copy.cpp",
    "gdal/frmts/gtiff/gt_overview.cpp",
    "gdal/frmts/gtiff/gt_wkt_srs.cpp",
    "gdal/frmts/gtiff/libgeotiff/geo_extra.c",
    "gdal/frmts/gtiff/libgeotiff/geo_free.c",
    "gdal/frmts/gtiff/libgeotiff/geo_get.c",
    "gdal/frmts/gtiff/libgeotiff/geo_names.c",
    "gdal/frmts/gtiff/libgeotiff/geo_new.c",
    "gdal/frmts/gtiff/libgeotiff/geo_normalize.c",
    "gdal/frmts/gtiff/libgeotiff/geo_print.c",
    "gdal/frmts/gtiff/libgeotiff/geo_set.c",
    "gdal/frmts/gtiff/libgeotiff/geo_simpletags.c",
    "gdal/frmts/gtiff/libgeotiff/geo_tiffp.c",
    "gdal/frmts/gtiff/libgeotiff/geotiff_proj4.c",
    "gdal/frmts/gtiff/libgeotiff/geo_trans.c",
    "gdal/frmts/gtiff/libgeotiff/geo_write.c",
    "gdal/frmts/gtiff/libgeotiff/xtiff.c",
    "gdal/frmts/gtiff/tif_float.c",
    "gdal/frmts/gtiff/tifvsi.cpp",
    -- "gdal/frmts/gxf/gxfdataset.cpp",
    -- "gdal/frmts/gxf/gxf_ogcwkt.c",
    -- "gdal/frmts/gxf/gxfopen.c",
    -- "gdal/frmts/gxf/gxf_proj4.c",
    -- "gdal/frmts/hf2/hf2dataset.cpp",
    "gdal/frmts/hfa/hfaband.cpp",
    "gdal/frmts/hfa/hfacompress.cpp",
    "gdal/frmts/hfa/hfadataset.cpp",
    "gdal/frmts/hfa/hfadictionary.cpp",
    "gdal/frmts/hfa/hfaentry.cpp",
    "gdal/frmts/hfa/hfafield.cpp",
    "gdal/frmts/hfa/hfaopen.cpp",
    "gdal/frmts/hfa/hfa_overviews.cpp",
    "gdal/frmts/hfa/hfatype.cpp",
    -- "gdal/frmts/ilwis/ilwiscoordinatesystem.cpp",
    -- "gdal/frmts/ilwis/ilwisdataset.cpp",
    -- "gdal/frmts/ingr/JpegHelper.cpp",
    -- "gdal/frmts/iris/irisdataset.cpp",
    "gdal/frmts/iso8211/ddffield.cpp",
    "gdal/frmts/iso8211/ddffielddefn.cpp",
    "gdal/frmts/iso8211/ddfmodule.cpp",
    "gdal/frmts/iso8211/ddfrecord.cpp",
    "gdal/frmts/iso8211/ddfsubfielddefn.cpp",
    "gdal/frmts/iso8211/ddfutils.cpp",
    -- "gdal/frmts/jaxapalsar/jaxapalsardataset.cpp",
    -- "gdal/frmts/jdem/jdemdataset.cpp",
    "gdal/frmts/jp2kak/jp2kakdataset.cpp",
    "gdal/frmts/jpeg/jpgdataset.cpp",
    "gdal/frmts/jpeg/vsidataio.cpp",
    -- "gdal/frmts/jpipkak/jpipkakdataset.cpp",
    -- "gdal/frmts/jpipkak/kdu_cache.cpp",
    -- "gdal/frmts/kmlsuperoverlay/kmlsuperoverlaydataset.cpp",
    -- "gdal/frmts/l1b/l1bdataset.cpp",
    -- "gdal/frmts/leveller/levellerdataset.cpp",
    -- "gdal/frmts/map/mapdataset.cpp",
    -- "gdal/frmts/mbtiles/mbtilesdataset.cpp",
    "gdal/frmts/mem/memdataset.cpp",
    "gdal/frmts/mrsid/mrsiddataset.cpp",
    "gdal/frmts/mrsid/mrsidstream.cpp",
    -- "gdal/frmts/msgn/msg_basic_types.cpp",
    -- "gdal/frmts/msgn/msgndataset.cpp",
    -- "gdal/frmts/msgn/msg_reader_core.cpp",
    -- "gdal/frmts/ngsgeoid/ngsgeoiddataset.cpp",
    "gdal/frmts/nitf/ecrgtocdataset.cpp",
    "gdal/frmts/nitf/mgrs.c",
    "gdal/frmts/nitf/nitfaridpcm.cpp",
    "gdal/frmts/nitf/nitfbilevel.cpp",
    "gdal/frmts/nitf/nitfdataset.cpp",
    "gdal/frmts/nitf/nitfdes.c",
    "gdal/frmts/nitf/nitffile.c",
    "gdal/frmts/nitf/nitf_gcprpc.cpp",
    "gdal/frmts/nitf/nitfimage.c",
    "gdal/frmts/nitf/nitfrasterband.cpp",
    "gdal/frmts/nitf/nitfwritejpeg_12.cpp",
    "gdal/frmts/nitf/nitfwritejpeg.cpp",
    "gdal/frmts/nitf/rpftocdataset.cpp",
    "gdal/frmts/nitf/rpftocfile.cpp",
    -- "gdal/frmts/northwood/grcdataset.cpp",
    -- "gdal/frmts/northwood/grddataset.cpp",
    -- "gdal/frmts/northwood/northwood.cpp",
    -- "gdal/frmts/ozi/ozidataset.cpp",
    -- "gdal/frmts/pdf/ogrpdflayer.cpp",
    -- "gdal/frmts/pdf/pdfcreatecopy.cpp",
    -- "gdal/frmts/pdf/pdfdataset.cpp",
    -- "gdal/frmts/pdf/pdfio.cpp",
    -- "gdal/frmts/pdf/pdfobject.cpp",
    -- "gdal/frmts/pdf/pdfreadvectors.cpp",
    -- "gdal/frmts/pdf/pdfwritabledataset.cpp",
    -- "gdal/frmts/pds/isis2dataset.cpp",
    -- "gdal/frmts/pds/isis3dataset.cpp",
    -- "gdal/frmts/pds/nasakeywordhandler.cpp",
    -- "gdal/frmts/pds/pds4dataset.cpp",
    -- "gdal/frmts/pds/pdsdataset.cpp",
    -- "gdal/frmts/pds/vicardataset.cpp",
    -- "gdal/frmts/pds/vicarkeywordhandler.cpp",
    "gdal/frmts/png/pngdataset.cpp",
    -- "gdal/frmts/prf/phprfdataset.cpp",
    -- "gdal/frmts/rasterlite/rasterlitecreatecopy.cpp",
    -- "gdal/frmts/rasterlite/rasterlitedataset.cpp",
    -- "gdal/frmts/rasterlite/rasterliteoverviews.cpp",
    -- "gdal/frmts/raw/ace2dataset.cpp",
    -- "gdal/frmts/raw/atlsci_spheroid.cpp",
    -- "gdal/frmts/raw/btdataset.cpp",
    -- "gdal/frmts/raw/cpgdataset.cpp",
    -- "gdal/frmts/raw/ctable2dataset.cpp",
    -- "gdal/frmts/raw/dipxdataset.cpp",
    -- "gdal/frmts/raw/doq1dataset.cpp",
    -- "gdal/frmts/raw/doq2dataset.cpp",
    -- "gdal/frmts/raw/ehdrdataset.cpp",
    -- "gdal/frmts/raw/eirdataset.cpp",
    -- "gdal/frmts/raw/envidataset.cpp",
    -- "gdal/frmts/raw/fastdataset.cpp",
    -- "gdal/frmts/raw/fujibasdataset.cpp",
    -- "gdal/frmts/raw/genbindataset.cpp",
    -- "gdal/frmts/raw/gscdataset.cpp",
    -- "gdal/frmts/raw/gtxdataset.cpp",
    -- "gdal/frmts/raw/hkvdataset.cpp",
    -- "gdal/frmts/raw/idadataset.cpp",
    -- "gdal/frmts/raw/iscedataset.cpp",
    -- "gdal/frmts/raw/krodataset.cpp",
    -- "gdal/frmts/raw/landataset.cpp",
    -- "gdal/frmts/raw/lcpdataset.cpp",
    -- "gdal/frmts/raw/loslasdataset.cpp",
    -- "gdal/frmts/raw/mffdataset.cpp",
    -- "gdal/frmts/raw/ndfdataset.cpp",
    -- "gdal/frmts/raw/ntv2dataset.cpp",
    -- "gdal/frmts/raw/pauxdataset.cpp",
    -- "gdal/frmts/raw/pnmdataset.cpp",
    "gdal/frmts/raw/rawdataset.cpp",
    -- "gdal/frmts/raw/roipacdataset.cpp",
    -- "gdal/frmts/raw/rrasterdataset.cpp",
    -- "gdal/frmts/raw/snodasdataset.cpp",
    -- "gdal/frmts/rik/rikdataset.cpp",
    -- "gdal/frmts/rmf/rmfdataset.cpp",
    -- "gdal/frmts/rmf/rmfdem.cpp",
    -- "gdal/frmts/rmf/rmflzw.cpp",
    -- "gdal/frmts/r/rcreatecopy.cpp",
    -- "gdal/frmts/r/rdataset.cpp",
    -- "gdal/frmts/rs2/rs2dataset.cpp",
    -- "gdal/frmts/safe/safedataset.cpp",
    -- "gdal/frmts/saga/sagadataset.cpp",
    -- "gdal/frmts/sdts/sdtsattrreader.cpp",
    -- "gdal/frmts/sdts/sdtscatd.cpp",
    -- "gdal/frmts/sdts/sdtsdataset.cpp",
    -- "gdal/frmts/sdts/sdtsindexedreader.cpp",
    -- "gdal/frmts/sdts/sdtsiref.cpp",
    -- "gdal/frmts/sdts/sdtslib.cpp",
    -- "gdal/frmts/sdts/sdtslinereader.cpp",
    -- "gdal/frmts/sdts/sdtspointreader.cpp",
    -- "gdal/frmts/sdts/sdtspolygonreader.cpp",
    -- "gdal/frmts/sdts/sdtsrasterreader.cpp",
    -- "gdal/frmts/sdts/sdtstransfer.cpp",
    -- "gdal/frmts/sdts/sdtsxref.cpp",
    -- "gdal/frmts/sentinel2/sentinel2dataset.cpp",
    -- "gdal/frmts/sgi/sgidataset.cpp",
    "gdal/frmts/srtmhgt/srtmhgtdataset.cpp",
    -- "gdal/frmts/terragen/terragendataset.cpp",
    -- "gdal/frmts/til/tildataset.cpp",
    -- "gdal/frmts/tsx/tsxdataset.cpp",
    "gdal/frmts/usgsdem/usgsdem_create.cpp",
    "gdal/frmts/usgsdem/usgsdemdataset.cpp",
    "gdal/frmts/vrt/pixelfunctions.cpp",
    "gdal/frmts/vrt/vrtdataset.cpp",
    "gdal/frmts/vrt/vrtderivedrasterband.cpp",
    "gdal/frmts/vrt/vrtdriver.cpp",
    "gdal/frmts/vrt/vrtfilters.cpp",
    "gdal/frmts/vrt/vrtpansharpened.cpp",
    "gdal/frmts/vrt/vrtrasterband.cpp",
    "gdal/frmts/vrt/vrtrawrasterband.cpp",
    "gdal/frmts/vrt/vrtsourcedrasterband.cpp",
    "gdal/frmts/vrt/vrtsources.cpp",
    "gdal/frmts/vrt/vrtwarped.cpp",
    -- "gdal/frmts/xpm/xpmdataset.cpp",
    -- "gdal/frmts/xyz/xyzdataset.cpp",
    -- "gdal/frmts/zmap/zmapdataset.cpp",
    "gdal/gcore/gdalabstractbandblockcache.cpp",
    "gdal/gcore/gdalallvalidmaskband.cpp",
    "gdal/gcore/gdalarraybandblockcache.cpp",
    "gdal/gcore/gdalclientserver.cpp",
    "gdal/gcore/gdalcolortable.cpp",
    "gdal/gcore/gdaldataset.cpp",
    "gdal/gcore/gdaldefaultasync.cpp",
    "gdal/gcore/gdaldefaultoverviews.cpp",
    "gdal/gcore/gdaldllmain.cpp",
    "gdal/gcore/gdaldriver.cpp",
    "gdal/gcore/gdaldrivermanager.cpp",
    "gdal/gcore/gdalexif.cpp",
    "gdal/gcore/gdalgeorefpamdataset.cpp",
    "gdal/gcore/gdalhashsetbandblockcache.cpp",
    "gdal/gcore/gdaljp2abstractdataset.cpp",
    "gdal/gcore/gdaljp2box.cpp",
    "gdal/gcore/gdaljp2metadata.cpp",
    "gdal/gcore/gdaljp2metadatagenerator.cpp",
    -- "gdal/gcore/gdaljp2structure.cpp",
    "gdal/gcore/gdalmajorobject.cpp",
    "gdal/gcore/gdal_mdreader.cpp",
    "gdal/gcore/gdal_misc.cpp",
    "gdal/gcore/gdalmultidomainmetadata.cpp",
    "gdal/gcore/gdalnodatamaskband.cpp",
    "gdal/gcore/gdalnodatavaluesmaskband.cpp",
    "gdal/gcore/gdalopeninfo.cpp",
    "gdal/gcore/gdaloverviewdataset.cpp",
    "gdal/gcore/gdalpamdataset.cpp",
    "gdal/gcore/gdalpamproxydb.cpp",
    "gdal/gcore/gdalpamrasterband.cpp",
    "gdal/gcore/gdalproxydataset.cpp",
    "gdal/gcore/gdalproxypool.cpp",
    "gdal/gcore/gdalrasterband.cpp",
    "gdal/gcore/gdalrasterblock.cpp",
    "gdal/gcore/gdal_rat.cpp",
    "gdal/gcore/gdalrescaledalphaband.cpp",
    "gdal/gcore/gdalvirtualmem.cpp",
    "gdal/gcore/mdreader/reader_alos.cpp",
    "gdal/gcore/mdreader/reader_digital_globe.cpp",
    "gdal/gcore/mdreader/reader_eros.cpp",
    "gdal/gcore/mdreader/reader_geo_eye.cpp",
    "gdal/gcore/mdreader/reader_kompsat.cpp",
    "gdal/gcore/mdreader/reader_landsat.cpp",
    "gdal/gcore/mdreader/reader_orb_view.cpp",
    "gdal/gcore/mdreader/reader_pleiades.cpp",
    "gdal/gcore/mdreader/reader_rapid_eye.cpp",
    "gdal/gcore/mdreader/reader_rdk1.cpp",
    "gdal/gcore/mdreader/reader_spot.cpp",
    "gdal/gcore/overview.cpp",
    "gdal/gcore/rasterio.cpp",
    "gdal/gcore/rasterio_ssse3.cpp",
    -- "gdal/gnm/gnm_frmts/db/gnmdbdriver.cpp",
    -- "gdal/gnm/gnm_frmts/db/gnmdbnetwork.cpp",
    -- "gdal/gnm/gnm_frmts/file/gnmfiledriver.cpp",
    -- "gdal/gnm/gnm_frmts/file/gnmfilenetwork.cpp",
    -- "gdal/gnm/gnm_frmts/gnmregisterall.cpp",
    -- "gdal/gnm/gnmgenericnetwork.cpp",
    -- "gdal/gnm/gnmgraph.cpp",
    -- "gdal/gnm/gnmlayer.cpp",
    -- "gdal/gnm/gnmnetwork.cpp",
    -- "gdal/gnm/gnmresultlayer.cpp",
    -- "gdal/gnm/gnmrule.cpp",
    "gdal/ogr/gml2ogrgeometry.cpp",
    "gdal/ogr/ogr2gmlgeometry.cpp",
    "gdal/ogr/ogr_api.cpp",
    "gdal/ogr/ograpispy.cpp",
    "gdal/ogr/ograssemblepolygon.cpp",
    "gdal/ogr/ogrcircularstring.cpp",
    "gdal/ogr/ogrcompoundcurve.cpp",
    "gdal/ogr/ogrct.cpp",
    "gdal/ogr/ogrcurvecollection.cpp",
    "gdal/ogr/ogrcurve.cpp",
    "gdal/ogr/ogrcurvepolygon.cpp",
    "gdal/ogr/ogr_expat.cpp",
    "gdal/ogr/ogrfeature.cpp",
    "gdal/ogr/ogrfeaturedefn.cpp",
    "gdal/ogr/ogrfeaturequery.cpp",
    "gdal/ogr/ogrfeaturestyle.cpp",
    "gdal/ogr/ogrfielddefn.cpp",
    "gdal/ogr/ogr_fromepsg.cpp",
    "gdal/ogr/ogr_geocoding.cpp",
    "gdal/ogr/ogrgeomediageometry.cpp",
    "gdal/ogr/ogrgeometrycollection.cpp",
    "gdal/ogr/ogrgeometry.cpp",
    "gdal/ogr/ogrgeometryfactory.cpp",
    "gdal/ogr/ogrgeomfielddefn.cpp",
    "gdal/ogr/ogr_geo_utils.cpp",
    "gdal/ogr/ogrlinearring.cpp",
    "gdal/ogr/ogrlinestring.cpp",
    "gdal/ogr/ogrmulticurve.cpp",
    "gdal/ogr/ogrmultilinestring.cpp",
    "gdal/ogr/ogrmultipoint.cpp",
    "gdal/ogr/ogrmultipolygon.cpp",
    "gdal/ogr/ogrmultisurface.cpp",
    "gdal/ogr/ogr_opt.cpp",
    "gdal/ogr/ogrpgeogeometry.cpp",
    "gdal/ogr/ogrpoint.cpp",
    "gdal/ogr/ogrpolygon.cpp",
    "gdal/ogr/ogrpolyhedralsurface.cpp",
    -- "gdal/ogr/ogrsf_frmts/aeronavfaa/ograeronavfaadatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/aeronavfaa/ograeronavfaadriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/aeronavfaa/ograeronavfaalayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/arcgen/ograrcgendatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/arcgen/ograrcgendriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/arcgen/ograrcgenlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/avc_bin.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/avc_e00gen.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/avc_e00parse.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/avc_e00read.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/avc_mbyte.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/avc_misc.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/avc_rawbin.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/ogravcbindatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/ogravcbindriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/ogravcbinlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/ogravcdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/ogravce00datasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/ogravce00driver.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/ogravce00layer.cpp",
    -- "gdal/ogr/ogrsf_frmts/avc/ogravclayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/bna/ogrbnadatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/bna/ogrbnadriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/bna/ogrbnalayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/bna/ogrbnaparser.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/gdalcaddataset.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadclasses.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadcolors.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/caddictionary.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadfile.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadfileio.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadfilestreamio.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadgeometry.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadheader.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadobjects.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/cadtables.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/dwg/io.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/dwg/r2000.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/libopencad/opencad.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/ogrcaddriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/ogrcadlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/cad/vsilfileio.cpp",
    -- "gdal/ogr/ogrsf_frmts/csv/ogrcsvdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/csv/ogrcsvdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/csv/ogrcsvlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/dgn/dgnfloat.cpp",
    -- "gdal/ogr/ogrsf_frmts/dgn/dgnhelp.cpp",
    -- "gdal/ogr/ogrsf_frmts/dgn/dgnopen.cpp",
    -- "gdal/ogr/ogrsf_frmts/dgn/dgnread.cpp",
    -- "gdal/ogr/ogrsf_frmts/dgn/dgnstroke.cpp",
    -- "gdal/ogr/ogrsf_frmts/dgn/dgnwrite.cpp",
    -- "gdal/ogr/ogrsf_frmts/dgn/ogrdgndatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/dgn/ogrdgndriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/dgn/ogrdgnlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/intronurbs.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogr_autocad_services.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxf_blockmap.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxfblockslayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxfblockswriterlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxfdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxf_dimension.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxfdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxf_feature.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxf_hatch.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxflayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxf_leader.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxf_ocstransformer.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxf_polyline_smooth.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxfreader.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxfwriterds.cpp",
    -- "gdal/ogr/ogrsf_frmts/dxf/ogrdxfwriterlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/edigeo/ogredigeodatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/edigeo/ogredigeodriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/edigeo/ogredigeolayer.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogr_attrind.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrdatasource.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogreditablelayer.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogremulatedtransaction.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogr_gensql.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrlayer.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrlayerdecorator.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrlayerpool.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogr_miattrind.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrmutexeddatasource.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrmutexedlayer.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrregisterall.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrsfdriver.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrsfdriverregistrar.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrunionlayer.cpp",
    "gdal/ogr/ogrsf_frmts/generic/ogrwarpedlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/geoconcept/geoconcept.c",
    -- "gdal/ogr/ogrsf_frmts/geoconcept/geoconcept_syscoord.c",
    -- "gdal/ogr/ogrsf_frmts/geoconcept/ogrgeoconceptdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/geoconcept/ogrgeoconceptdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/geoconcept/ogrgeoconceptlayer.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/libjson/arraylist.c",
    "gdal/ogr/ogrsf_frmts/geojson/libjson/debug.c",
    -- "gdal/ogr/ogrsf_frmts/geojson/libjson/json_c_version.c",
    "gdal/ogr/ogrsf_frmts/geojson/libjson/json_object.c",
    -- "gdal/ogr/ogrsf_frmts/geojson/libjson/json_object_iterator.c",
    "gdal/ogr/ogrsf_frmts/geojson/libjson/json_tokener.c",
    "gdal/ogr/ogrsf_frmts/geojson/libjson/json_util.c",
    "gdal/ogr/ogrsf_frmts/geojson/libjson/linkhash.c",
    "gdal/ogr/ogrsf_frmts/geojson/libjson/printbuf.c",
    -- "gdal/ogr/ogrsf_frmts/geojson/ogresrijsondriver.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/ogresrijsonreader.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/ogrgeojsondatasource.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/ogrgeojsondriver.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonlayer.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonreader.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonutils.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonwritelayer.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/ogrgeojsonwriter.cpp",
    -- "gdal/ogr/ogrsf_frmts/geojson/ogrtopojsondriver.cpp",
    "gdal/ogr/ogrsf_frmts/geojson/ogrtopojsonreader.cpp",
    -- "gdal/ogr/ogrsf_frmts/georss/ogrgeorssdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/georss/ogrgeorssdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/georss/ogrgeorsslayer.cpp",
    "gdal/ogr/ogrsf_frmts/gml/gfstemplate.cpp",
    "gdal/ogr/ogrsf_frmts/gml/gmlfeatureclass.cpp",
    "gdal/ogr/ogrsf_frmts/gml/gmlfeature.cpp",
    "gdal/ogr/ogrsf_frmts/gml/gmlhandler.cpp",
    "gdal/ogr/ogrsf_frmts/gml/gmlpropertydefn.cpp",
    "gdal/ogr/ogrsf_frmts/gml/gmlreader.cpp",
    "gdal/ogr/ogrsf_frmts/gml/gmlreadstate.cpp",
    "gdal/ogr/ogrsf_frmts/gml/gmlregistry.cpp",
    "gdal/ogr/ogrsf_frmts/gml/gmlutils.cpp",
    "gdal/ogr/ogrsf_frmts/gml/hugefileresolver.cpp",
    "gdal/ogr/ogrsf_frmts/gml/ogrgmldatasource.cpp",
    "gdal/ogr/ogrsf_frmts/gml/ogrgmldriver.cpp",
    "gdal/ogr/ogrsf_frmts/gml/ogrgmllayer.cpp",
    "gdal/ogr/ogrsf_frmts/gml/parsexsd.cpp",
    "gdal/ogr/ogrsf_frmts/gml/resolvexlinks.cpp",
    -- "gdal/ogr/ogrsf_frmts/gmt/ogrgmtdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/gmt/ogrgmtdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/gmt/ogrgmtlayer.cpp",
    "gdal/ogr/ogrsf_frmts/gpkg/gdalgeopackagerasterband.cpp",
    "gdal/ogr/ogrsf_frmts/gpkg/ogrgeopackagedatasource.cpp",
    "gdal/ogr/ogrsf_frmts/gpkg/ogrgeopackagedriver.cpp",
    "gdal/ogr/ogrsf_frmts/gpkg/ogrgeopackagelayer.cpp",
    "gdal/ogr/ogrsf_frmts/gpkg/ogrgeopackageselectlayer.cpp",
    "gdal/ogr/ogrsf_frmts/gpkg/ogrgeopackagetablelayer.cpp",
    "gdal/ogr/ogrsf_frmts/gpkg/ogrgeopackageutility.cpp",
    -- "gdal/ogr/ogrsf_frmts/gpsbabel/ogrgpsbabeldatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/gpsbabel/ogrgpsbabeldriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/gpsbabel/ogrgpsbabelwritedatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/gpx/ogrgpxdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/gpx/ogrgpxdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/gpx/ogrgpxlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/gtm/gtm.cpp",
    -- "gdal/ogr/ogrsf_frmts/gtm/gtmtracklayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/gtm/gtmwaypointlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/gtm/ogrgtmdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/gtm/ogrgtmdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/gtm/ogrgtmlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/htf/ogrhtfdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/htf/ogrhtfdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/htf/ogrhtflayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/idrisi/ogridrisidatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/idrisi/ogridrisidriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/idrisi/ogridrisilayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/jml/ogrjmldataset.cpp",
    -- "gdal/ogr/ogrsf_frmts/jml/ogrjmllayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/jml/ogrjmlwriterlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/kml/kml.cpp",
    -- "gdal/ogr/ogrsf_frmts/kml/kmlnode.cpp",
    -- "gdal/ogr/ogrsf_frmts/kml/kmlvector.cpp",
    "gdal/ogr/ogrsf_frmts/kml/ogr2kmlgeometry.cpp",
    -- "gdal/ogr/ogrsf_frmts/kml/ogrkmldatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/kml/ogrkmldriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/kml/ogrkmllayer.cpp",
    "gdal/ogr/ogrsf_frmts/mem/ogrmemdatasource.cpp",
    "gdal/ogr/ogrsf_frmts/mem/ogrmemdriver.cpp",
    "gdal/ogr/ogrsf_frmts/mem/ogrmemlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_bounds.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_coordsys.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_datfile.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_feature.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_feature_mif.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_geometry.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_idfile.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_imapinfofile.cpp",
    "gdal/ogr/ogrsf_frmts/mitab/mitab_indfile.cpp",
    "gdal/ogr/ogrsf_frmts/mitab/mitab_mapcoordblock.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_mapfile.cpp",
    "gdal/ogr/ogrsf_frmts/mitab/mitab_mapheaderblock.cpp",
    "gdal/ogr/ogrsf_frmts/mitab/mitab_mapindexblock.cpp",
    "gdal/ogr/ogrsf_frmts/mitab/mitab_mapobjectblock.cpp",
    "gdal/ogr/ogrsf_frmts/mitab/mitab_maptoolblock.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_middatafile.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_miffile.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_ogr_datasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_ogr_driver.cpp",
    "gdal/ogr/ogrsf_frmts/mitab/mitab_rawbinblock.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_spatialref.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_tabfile.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_tabseamless.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_tabview.cpp",
    -- "gdal/ogr/ogrsf_frmts/mitab/mitab_tooldef.cpp",
    "gdal/ogr/ogrsf_frmts/mitab/mitab_utils.cpp",
    -- "gdal/ogr/ogrsf_frmts/mvt/mvt_tile.cpp",
    -- "gdal/ogr/ogrsf_frmts/mvt/mvtutils.cpp",
    -- "gdal/ogr/ogrsf_frmts/mvt/ogrmvtdataset.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ntf_codelist.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ntf_estlayers.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ntffilereader.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ntf_generic.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ntf_raster.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ntfrecord.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ntfstroke.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ogrntfdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ogrntfdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ogrntffeatureclasslayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/ntf/ogrntflayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/ods/ods_formula.cpp",
    -- "gdal/ogr/ogrsf_frmts/ods/ods_formula_node.cpp",
    -- "gdal/ogr/ogrsf_frmts/ods/ogrodsdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/ods/ogrodsdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/openair/ogropenairdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/openair/ogropenairdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/openair/ogropenairlabellayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/openair/ogropenairlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/openfilegdb/filegdbindex.cpp",
    -- "gdal/ogr/ogrsf_frmts/openfilegdb/filegdbtable.cpp",
    -- "gdal/ogr/ogrsf_frmts/openfilegdb/ogropenfilegdbdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/openfilegdb/ogropenfilegdbdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/openfilegdb/ogropenfilegdblayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/osm/ogrosmdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/osm/ogrosmdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/osm/ogrosmlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/osm/osm_parser.cpp",
    -- "gdal/ogr/ogrsf_frmts/pds/ogrpdsdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/pds/ogrpdsdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/pds/ogrpdslayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/pgdump/ogrpgdumpdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/pgdump/ogrpgdumpdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/pgdump/ogrpgdumplayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/rec/ll_recio.cpp",
    -- "gdal/ogr/ogrsf_frmts/rec/ogrrecdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/rec/ogrrecdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/rec/ogrreclayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/s57/ddfrecordindex.cpp",
    -- "gdal/ogr/ogrsf_frmts/s57/ogrs57datasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/s57/ogrs57driver.cpp",
    -- "gdal/ogr/ogrsf_frmts/s57/ogrs57layer.cpp",
    -- "gdal/ogr/ogrsf_frmts/s57/s57classregistrar.cpp",
    -- "gdal/ogr/ogrsf_frmts/s57/s57featuredefns.cpp",
    -- "gdal/ogr/ogrsf_frmts/s57/s57filecollector.cpp",
    -- "gdal/ogr/ogrsf_frmts/s57/s57reader.cpp",
    -- "gdal/ogr/ogrsf_frmts/s57/s57writer.cpp",
    -- "gdal/ogr/ogrsf_frmts/sdts/ogrsdtsdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/sdts/ogrsdtsdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/sdts/ogrsdtslayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/segukooa/ogrsegukooadatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/segukooa/ogrsegukooadriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/segukooa/ogrsegukooalayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/segy/ogrsegydatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/segy/ogrsegydriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/segy/ogrsegylayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/selafin/io_selafin.cpp",
    -- "gdal/ogr/ogrsf_frmts/selafin/ogrselafindatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/selafin/ogrselafindriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/selafin/ogrselafinlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/shape/dbfopen_wrapper.c",
    -- "gdal/ogr/ogrsf_frmts/shape/ogrshapedatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/shape/ogrshapedriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/shape/ogrshapelayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/shape/sbnsearch_wrapper.c",
    -- "gdal/ogr/ogrsf_frmts/shape/shape2ogr.cpp",
    -- "gdal/ogr/ogrsf_frmts/shape/shpopen_wrapper.c",
    -- "gdal/ogr/ogrsf_frmts/shape/shptree_wrapper.c",
    -- "gdal/ogr/ogrsf_frmts/shape/shp_vsi.c",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqliteapiroutines.c",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqlitedatasource.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqlitedriver.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqliteexecutesql.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqlitelayer.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqliteselectlayer.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqlitesinglefeaturelayer.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqlitetablelayer.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqliteutility.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqlitevfs.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqliteviewlayer.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/ogrsqlitevirtualogr.cpp",
    "gdal/ogr/ogrsf_frmts/sqlite/rasterlite2.cpp",
    -- "gdal/ogr/ogrsf_frmts/sua/ogrsuadatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/sua/ogrsuadriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/sua/ogrsualayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/svg/ogrsvgdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/svg/ogrsvgdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/svg/ogrsvglayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/sxf/ogrsxfdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/sxf/ogrsxfdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/sxf/ogrsxflayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/ogrtigerdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/ogrtigerdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/ogrtigerlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigeraltname.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerarealandmarks.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigercompletechain.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerentitynames.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerfeatureids.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerfilebase.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigeridhistory.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerkeyfeatures.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerlandmarks.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigeroverunder.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerpip.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerpoint.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerpolychainlink.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerpolygoncorrections.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerpolygon.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerpolygoneconomic.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerspatialmetadata.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigertlidrange.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerzerocellid.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerzipcodes.cpp",
    -- "gdal/ogr/ogrsf_frmts/tiger/tigerzipplus4.cpp",
    -- "gdal/ogr/ogrsf_frmts/vdv/ogrvdvdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/ogrvfkdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/ogrvfkdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/ogrvfklayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/vfkdatablock.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/vfkdatablocksqlite.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/vfkfeature.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/vfkfeaturesqlite.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/vfkproperty.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/vfkpropertydefn.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/vfkreader.cpp",
    -- "gdal/ogr/ogrsf_frmts/vfk/vfkreadersqlite.cpp",
    "gdal/ogr/ogrsf_frmts/vrt/ogrvrtdatasource.cpp",
    "gdal/ogr/ogrsf_frmts/vrt/ogrvrtdriver.cpp",
    "gdal/ogr/ogrsf_frmts/vrt/ogrvrtlayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/wasp/ogrwaspdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/wasp/ogrwaspdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/wasp/ogrwasplayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/xlsx/ogrxlsxdatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/xlsx/ogrxlsxdriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_apt_reader.cpp",
    -- "gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_awy_reader.cpp",
    -- "gdal/ogr/ogrsf_frmts/xplane/ogrxplanedatasource.cpp",
    -- "gdal/ogr/ogrsf_frmts/xplane/ogrxplanedriver.cpp",
    -- "gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_fix_reader.cpp",
    -- "gdal/ogr/ogrsf_frmts/xplane/ogrxplanelayer.cpp",
    -- "gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_nav_reader.cpp",
    -- "gdal/ogr/ogrsf_frmts/xplane/ogr_xplane_reader.cpp",
    "gdal/ogr/ogrspatialreference.cpp",
    "gdal/ogr/ogr_srs_dict.cpp",
    "gdal/ogr/ogr_srs_erm.cpp",
    "gdal/ogr/ogr_srs_esri.cpp",
    "gdal/ogr/ogr_srsnode.cpp",
    "gdal/ogr/ogr_srs_ozi.cpp",
    "gdal/ogr/ogr_srs_panorama.cpp",
    "gdal/ogr/ogr_srs_pci.cpp",
    "gdal/ogr/ogr_srs_proj4.cpp",
    "gdal/ogr/ogr_srs_usgs.cpp",
    "gdal/ogr/ogr_srs_validate.cpp",
    "gdal/ogr/ogr_srs_xml.cpp",
    "gdal/ogr/ogrsurface.cpp",
    "gdal/ogr/ogrtriangle.cpp",
    "gdal/ogr/ogrtriangulatedsurface.cpp",
    "gdal/ogr/ogrutils.cpp",
    "gdal/ogr/ogr_xerces.cpp",
    "gdal/ogr/osr_cs_wkt.c",
    "gdal/ogr/osr_cs_wkt_parser.c",
    "gdal/ogr/swq.cpp",
    "gdal/ogr/swq_expr_node.cpp",
    "gdal/ogr/swq_op_general.cpp",
    "gdal/ogr/swq_op_registrar.cpp",
    "gdal/ogr/swq_parser.cpp",
    "gdal/ogr/swq_select.cpp",
    "gdal/port/cpl_alibaba_oss.cpp",
    "gdal/port/cpl_atomic_ops.cpp",
    "gdal/port/cpl_aws.cpp",
    "gdal/port/cpl_azure.cpp",
    "gdal/port/cpl_base64.cpp",
    "gdal/port/cpl_conv.cpp",
    "gdal/port/cpl_cpu_features.cpp",
    "gdal/port/cpl_csv.cpp",
    "gdal/port/cpl_error.cpp",
    "gdal/port/cpl_findfile.cpp",
    "gdal/port/cpl_getexecpath.cpp",
    "gdal/port/cplgetsymbol.cpp",
    "gdal/port/cpl_google_cloud.cpp",
    "gdal/port/cpl_google_oauth2.cpp",
    "gdal/port/cpl_hash_set.cpp",
    "gdal/port/cpl_http.cpp",
    "gdal/port/cpl_json.cpp",
    "gdal/port/cpl_json_streaming_parser.cpp",
    "gdal/port/cplkeywordparser.cpp",
    "gdal/port/cpl_list.cpp",
    "gdal/port/cpl_md5.cpp",
    "gdal/port/cpl_minixml.cpp",
    "gdal/port/cpl_minizip_ioapi.cpp",
    "gdal/port/cpl_minizip_unzip.cpp",
    "gdal/port/cpl_minizip_zip.cpp",
    "gdal/port/cpl_multiproc.cpp",
    "gdal/port/cpl_path.cpp",
    "gdal/port/cpl_progress.cpp",
    "gdal/port/cpl_quad_tree.cpp",
    "gdal/port/cpl_recode.cpp",
    "gdal/port/cpl_recode_iconv.cpp",
    "gdal/port/cpl_recode_stub.cpp",
    "gdal/port/cpl_sha1.cpp",
    "gdal/port/cpl_sha256.cpp",
    "gdal/port/cpl_spawn.cpp",
    "gdal/port/cpl_string.cpp",
    "gdal/port/cplstring.cpp",
    "gdal/port/cplstringlist.cpp",
    "gdal/port/cpl_strtod.cpp",
    "gdal/port/cpl_swift.cpp",
    "gdal/port/cpl_time.cpp",
    "gdal/port/cpl_virtualmem.cpp",
    "gdal/port/cpl_vsi_error.cpp",
    "gdal/port/cpl_vsil_abstract_archive.cpp",
    "gdal/port/cpl_vsil_buffered_reader.cpp",
    "gdal/port/cpl_vsil_cache.cpp",
    "gdal/port/cpl_vsil.cpp",
    "gdal/port/cpl_vsil_crypt.cpp",
    "gdal/port/cpl_vsil_curl.cpp",
    "gdal/port/cpl_vsil_curl_streaming.cpp",
    "gdal/port/cpl_vsil_gzip.cpp",
    "gdal/port/cpl_vsil_sparsefile.cpp",
    "gdal/port/cpl_vsil_stdin.cpp",
    "gdal/port/cpl_vsil_stdout.cpp",
    "gdal/port/cpl_vsil_subfile.cpp",
    "gdal/port/cpl_vsil_tar.cpp",
    "gdal/port/cpl_vsil_unix_stdio_64.cpp",
    "gdal/port/cpl_vsil_win32.cpp",
    "gdal/port/cpl_vsi_mem.cpp",
    "gdal/port/cpl_vsisimple.cpp",
    "gdal/port/cpl_worker_thread_pool.cpp",
    "gdal/port/cpl_xml_validate.cpp",
  }

  defines {
    -- "AERONAVFAA_ENABLED",
    -- "ARCGEN_ENABLED",
    -- "AVCBIN_ENABLED",
    "BIGTIFF_SUPPORT",
    -- "BNA_ENABLED",
    -- "CAD_ENABLED",
    -- "CSV_ENABLED",
    -- "DGN_ENABLED",
    -- "DO_NOT_USE_DEBUG_BOOL", -- only enable if FRMT_pdf is enabled
    "DONT_DEPRECATE_SPRINTF",
    -- "DXF_ENABLED",
    -- "EDIGEO_ENABLED",
    "ENABLE_LIBJPEG_NO_RETURN",
    -- "ENABLE_UFFD",
    -- "FRMT_aaigrid",
    "FRMT_adrg",
    -- "FRMT_aigrid",
    -- "FRMT_airsar",
    -- "FRMT_arg",
    -- "FRMT_blx",
    -- "FRMT_bmp",
    -- "FRMT_bsb",
    -- "FRMT_cals",
    -- "FRMT_ceos",
    -- "FRMT_ceos2",
    -- "FRMT_coasp",
    -- "FRMT_cosar",
    -- "FRMT_ctg",
    "FRMT_derived",
    -- "FRMT_dimap",
    "FRMT_dted",
    -- "FRMT_e00grid",
    -- "FRMT_elas",
    -- "FRMT_envisat",
    -- "FRMT_ers",
    -- "FRMT_fit",
    -- "FRMT_gff",
    "FRMT_gif",
    -- "FRMT_gsg",
    "FRMT_gtiff",
    -- "FRMT_gxf",
    -- "FRMT_hf2",
    "FRMT_hfa",
    -- "FRMT_idrisi",
    -- "FRMT_ilwis",
    -- "FRMT_ingr",
    -- "FRMT_iris",
    "FRMT_iso8211",
    -- "FRMT_jaxapalsar",
    -- "FRMT_jdem",
    "FRMT_jp2kak",
    "FRMT_jpeg",
    -- "FRMT_jpipkak",
    -- "FRMT_kmlsuperoverlay",
    -- "FRMT_l1b",
    -- "FRMT_leveller",
    -- "FRMT_map",
    -- "FRMT_mbtiles",
    "FRMT_mem",
    "FRMT_mrsid",
    -- "FRMT_msgn",
    -- "FRMT_ngsgeoid",
    "FRMT_nitf",
    -- "FRMT_northwood",
    -- "FRMT_ozi",
    -- "FRMT_pdf",
    -- "FRMT_pds",
    "FRMT_png",
    -- "FRMT_prf",
    -- "FRMT_r",
    -- "FRMT_rasterlite",
    -- "FRMT_raw",
    -- "FRMT_rik",
    -- "FRMT_rmf",
    -- "FRMT_rs2",
    -- "FRMT_safe",
    -- "FRMT_saga",
    -- "FRMT_sdts",
    -- "FRMT_sentinel2",
    -- "FRMT_sgi",
    -- "FRMT_sigdem",
    "FRMT_srtmhgt",
    -- "FRMT_terragen",
    -- "FRMT_til",
    -- "FRMT_tsx",
    "FRMT_usgsdem",
    "FRMT_vrt",
    -- "FRMT_xpm",
    -- "FRMT_xyz",
    -- "FRMT_zmap",
    "GDAL_COMPILATION",
    -- "GDALDRIVER", -- only enable if FRMT_blx is enabled
    -- "GDAL_SUPPORT", -- only enable if FRMT_msgn is enabled
    -- "GEOCONCEPT_ENABLED",
    -- "GEOJSON_ENABLED",
    -- "GEORSS_ENABLED",
    "GML_ENABLED",
    -- "GMT_ENABLED",
    -- "GNMDB_ENABLED",
    -- "GNM_ENABLED",
    -- "GNMFILE_ENABLED",
    -- "GPSBABEL_ENABLED",
    -- "GPX_ENABLED",
    -- "GTM_ENABLED",
    -- "HAVE_AVX_AT_COMPILE_TIME",
    "HAVE_EXPAT",
    "HAVE_LIBJPEG",
    "HAVE_LIBZ",
    -- "HAVE_MITAB",
    "HAVE_SQLITE",
    -- "HTF_ENABLED",
    -- "IDRISI_ENABLED",
    -- "INST_DATA=\"/usr/local/share/gdal\"",
    "INTERNAL_LIBGEOTIFF",
    -- "JML_ENABLED",
    "JPEG_SUPPORTED",
    "KDU_INCLUDE_TIFF",
    -- "KML_ENABLED",
    "MEM_ENABLED",
    -- "MITAB_USE_OFTDATETIME", -- only enable if HAVE_MITAB is enabled
    -- "MVT_ENABLED",
    -- "NTF_ENABLED",
    -- "ODS_ENABLED",
    -- "OGR", -- only enable if HAVE_MITAB is enabled
    -- "OPENAIR_ENABLED",
    -- "OPENFILEGDB_ENABLED",
    -- "OSM_ENABLED",
    "PAM_ENABLED",
    -- "PDS_ENABLED",
    -- "PGDUMP_ENABLED",
    -- "REC_ENABLED",
    "_REENTRANT",
    -- "S57_ENABLED",
    -- "SAOffset=vsi_l_offset", -- only enable if SHAPE_ENABLED is enabled
    -- "SDTS_ENABLED",
    -- "SEGUKOOA_ENABLED",
    -- "SEGY_ENABLED",
    -- "SELAFIN_ENABLED",
    -- "SHAPE_ENABLED",
    "SQLITE_ENABLED",
    -- "SUA_ENABLED",
    -- "SVG_ENABLED",
    -- "SXF_ENABLED",
    -- "TAB_ENABLED",
    -- "TIGER_ENABLED",
    -- "USE_CPL", -- only enable if SHAPE_ENABLED is enabled
    -- "USE_ONLY_CRYPTODLL_ALG", -- only enable if HAVE_CRYPTOPP is enabled
    -- "VDV_ENABLED",
    -- "VFK_ENABLED",
    "VRT_ENABLED",
    -- "WASP_ENABLED",
    -- "XLSX_ENABLED",
    "XML_STATIC", -- needed to use the expat static library
    "_XOPEN_SOURCE=700",
    -- "XPLANE_ENABLED",
  }

  local neon_defines = {
    "KDU_NEON_INTRINSICS", -- kakadu
  }

  local intel_intrinsic_defines = {
    "HAVE_SSE_AT_COMPILE_TIME", -- gdal
    "HAVE_SSSE3_AT_COMPILE_TIME",
    "KDU_NO_AVX", -- kakadu
    "KDU_NO_AVX2",
    "KDU_NO_SSE4",
    "KDU_X86_INTRINSICS",
  }

  local cocoa_defines = {
    "_DARWIN_C_SOURCE",
  }

  local windows_defines = {
    "_CRT_NONSTDC_NO_DEPRECATE",
    "_CRT_SECURE_NO_WARNINGS",
  }

  includedirs {
    _3RDPARTY_DIR .. "/gdal/gdal/alg",
    _3RDPARTY_DIR .. "/gdal/gdal/apps",
    _3RDPARTY_DIR .. "/gdal/gdal/gcore",
    _3RDPARTY_DIR .. "/gdal/gdal/frmts",
    _3RDPARTY_DIR .. "/gdal/gdal/frmts/gtiff",
    _3RDPARTY_DIR .. "/gdal/gdal/frmts/gtiff/libgeotiff",
    _3RDPARTY_DIR .. "/gdal/gdal/frmts/iso8211",
    _3RDPARTY_DIR .. "/gdal/gdal/frmts/jpeg",
    _3RDPARTY_DIR .. "/gdal/gdal/frmts/mem",
    _3RDPARTY_DIR .. "/gdal/gdal/frmts/raw",
    _3RDPARTY_DIR .. "/gdal/gdal/frmts/vrt",
    _3RDPARTY_DIR .. "/gdal/gdal/ogr",
    _3RDPARTY_DIR .. "/gdal/gdal/ogr/ogrsf_frmts",
    _3RDPARTY_DIR .. "/gdal/gdal/ogr/ogrsf_frmts/generic",
    _3RDPARTY_DIR .. "/gdal/gdal/ogr/ogrsf_frmts/geojson",
    _3RDPARTY_DIR .. "/gdal/gdal/ogr/ogrsf_frmts/geojson/libjson",
    _3RDPARTY_DIR .. "/gdal/gdal/ogr/ogrsf_frmts/mem",
    _3RDPARTY_DIR .. "/gdal/gdal/ogr/ogrsf_frmts/sqlite",
    _3RDPARTY_DIR .. "/gdal/gdal/port",
    _3RDPARTY_DIR .. "/giflib",
    _3RDPARTY_DIR .. "/libjpeg-turbo",
    _3RDPARTY_DIR .. "/kakadu/include",
    _3RDPARTY_DIR .. "/libexpat/expat/lib",
    _3RDPARTY_DIR .. "/libtiff/libtiff",
    _3RDPARTY_DIR .. "/libpng",
    _3RDPARTY_DIR .. "/sqlite",
    _3RDPARTY_DIR .. "/zlib",
  }

  -- -------------------------------------------------------------
  -- configurations
  -- -------------------------------------------------------------

  if (os.is("windows") and not _TARGET_IS_WINUWP) then
    -- -------------------------------------------------------------
    -- configuration { "windows" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/3rdparty_static_win.lua")

    -- project specific configuration settings

    configuration { "windows" }

      defines {
        intel_intrinsic_defines,
        windows_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "windows","Debug","x32" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_win_x86_debug.lua")

    -- project specific configuration settings

    -- configuration { "windows","Debug","x32" }

    -- -------------------------------------------------------------
    -- configuration { "windows","Debug","x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_win_x64_debug.lua")

    -- project specific configuration settings

    -- configuration { "windows","Debug","x64" }

    -- -------------------------------------------------------------
    -- configuration { "windows","Release","x32" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_win_x86_release.lua")

    -- project specific configuration settings

    -- configuration { "windows","Release","x32" }

    -- -------------------------------------------------------------
    -- configuration { "windows","Release","x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_win_x64_release.lua")

    -- project specific configuration settings

    -- configuration { "windows","Release","x64" }

    -- -------------------------------------------------------------
  end

  if (os.is("linux") and not _OS_IS_ANDROID) then
    -- -------------------------------------------------------------
    -- configuration { "linux" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux.lua")

    -- project specific configuration settings

    -- configuration { "linux" }

    -- -------------------------------------------------------------
    -- configuration { "linux","Debug","x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux_x64_debug.lua")

    -- project specific configuration settings

    configuration { "linux","Debug","x64" }

      defines {
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "linux","Release","x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux_x64_release.lua")

    -- project specific configuration settings

    configuration { "linux","Release","x64" }

      defines {
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "linux", "Debug", "ARM64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux_arm64_debug.lua")

    -- project specific configuration settings

    configuration { "linux", "Debug", "ARM64" }

      defines {
        "KDU_NO_NEON", -- neon intrinsics for linux arm64 are not supported
      }

    -- -------------------------------------------------------------
    -- configuration { "linux", "Release", "ARM64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_linux_arm64_release.lua")

    -- project specific configuration settings

    configuration { "linux", "Release", "ARM64" }

      defines {
        "KDU_NO_NEON", -- neon intrinsics for linux arm64 are not supported
      }

    -- -------------------------------------------------------------
  end

  if (os.is("macosx") and not _OS_IS_IOS and not _OS_IS_ANDROID) then
    -- -------------------------------------------------------------
    -- configuration { "macosx" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_mac.lua")

    -- project specific configuration settings

    configuration { "macosx" }

      defines {
        cocoa_defines,
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "macosx","Debug","x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_mac_x64_debug.lua")

    -- project specific configuration settings

    -- configuration { "macosx","Debug","x64" }

    -- -------------------------------------------------------------
    -- configuration { "macosx","Release","x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_mac_x64_release.lua")

    -- project specific configuration settings

    -- configuration { "macosx","Release","x64" }

    -- -------------------------------------------------------------
  end

  if (_OS_IS_IOS) then
    -- -------------------------------------------------------------
    -- configuration { "ios" } == _OS_IS_IOS
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios.lua")

    -- project specific configuration settings

    configuration { "ios*" }

      defines {
        cocoa_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "ios_arm64_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios_arm64_debug.lua")

    -- project specific configuration settings

    configuration { "ios_arm64_debug" }

      defines {
        neon_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "ios_arm64_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios_arm64_release.lua")

    -- project specific configuration settings

    configuration { "ios_arm64_release" }

      defines {
        neon_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "ios_sim64_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios_sim64_debug.lua")

    -- project specific configuration settings

    configuration { "ios_sim64_debug" }

      defines {
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "ios_sim64_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_ios_sim64_release.lua")

    -- project specific configuration settings

    configuration { "ios_sim64_release" }

      defines {
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
  end

  if (_OS_IS_ANDROID) then
    -- -------------------------------------------------------------
    -- configuration { "android" } == _OS_IS_ANDROID
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android.lua")

    -- project specific configuration settings

    configuration { "android*" }

      defines {
        "KDU_NO_THREADS",
      }

    -- -------------------------------------------------------------
    -- configuration { "android_armv7_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_armv7_debug.lua")

    -- project specific configuration settings

    configuration { "android_armv7_debug" }

      defines {
        neon_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "android_armv7_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_armv7_release.lua")

    -- project specific configuration settings

    configuration { "android_armv7_release" }

      defines {
        neon_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "android_x86_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_x86_debug.lua")

    -- project specific configuration settings

    configuration { "android_x86_debug" }

      defines {
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "android_x86_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_x86_release.lua")

    -- project specific configuration settings

    configuration { "android_x86_release" }

      defines {
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "android_arm64_debug" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_arm64_debug.lua")

    -- project specific configuration settings

    configuration { "android_arm64_debug" }

      defines {
        neon_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "android_arm64_release" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_android_arm64_release.lua")

    -- project specific configuration settings

    configuration { "android_arm64_release" }

      defines {
        neon_defines,
      }

    -- -------------------------------------------------------------
  end

  if (_TARGET_IS_WINUWP) then
    -- -------------------------------------------------------------
    -- configuration { "winuwp" } == _TARGET_IS_WINUWP
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp.lua")

    -- project specific configuration settings

    configuration { "windows" }

      defines {
        windows_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_debug","x32" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_x86_debug.lua")

    -- project specific configuration settings

    configuration { "winuwp_debug","x32" }

      defines {
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_release","x32" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_x86_release.lua")

    -- project specific configuration settings

    configuration { "winuwp_release","x32" }

      defines {
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_debug","x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_x64_debug.lua")

    -- project specific configuration settings

    configuration { "winuwp_debug","x64" }

      defines {
        intel_intrinsic_defines,
      }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_release","x64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_x64_release.lua")

    -- project specific configuration settings

    configuration { "winuwp_release","x64" }

      defines {
        intel_intrinsic_defines,
      }

      buildoptions {
        "/wd4789", -- Silences buffer overrun warning which causes errors during LTCG with Ob3 optimizations on
      }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_debug","ARM" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_arm_debug.lua")

    -- project specific configuration settings

    configuration { "winuwp_debug","ARM" }

      defines {
        "KDU_NO_NEON", -- neon intrinsics for Windows is not supported
      }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_release","ARM" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_arm_release.lua")

    -- project specific configuration settings

    configuration { "winuwp_release","ARM" }

      defines {
        "KDU_NO_NEON", -- neon intrinsics for Windows is not supported
      }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_debug","ARM64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_arm64_debug.lua")

    -- project specific configuration settings

    configuration { "winuwp_debug","ARM64" }

      defines {
        "KDU_NO_NEON", -- neon intrinsics for Windows is not supported
      }

    -- -------------------------------------------------------------
    -- configuration { "winuwp_release","ARM64" }
    -- -------------------------------------------------------------

    -- common configuration settings

    dofile (_BUILD_DIR .. "/static_winuwp_arm64_release.lua")

    -- project specific configuration settings

    configuration { "winuwp_release","ARM64" }

      defines {
        "KDU_NO_NEON", -- neon intrinsics for Windows is not supported
      }

    -- -------------------------------------------------------------
  end

  if (_IS_QT) then
    -- -------------------------------------------------------------
    -- configuration { "qt" }
    -- -------------------------------------------------------------

    local qt_include_dirs = PROJECT_INCLUDE_DIRS

    -- Add additional QT include dirs
    -- table.insert(qt_include_dirs,<INCLUDE_PATH>)

    createqtfiles(project(),qt_include_dirs)

    -- -------------------------------------------------------------
  end
