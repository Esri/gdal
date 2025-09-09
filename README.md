# Esri instructions

```bash
  #-DGDAL_USE_MRSID=ON \
  #-DGDAL_USE_KDU=ON \
  #-DGDAL_USE_PDFIUM=ON \

cmake .. \
  -DCMAKE_C_COMPILER=/usr/local/rtc/llvm/19.1.2/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/local/rtc/llvm/19.1.2/bin/clang++ \
  -DBUILD_PYTHON_BINDINGS=OFF \
  -DGDAL_ENABLE_PLUGINS=OFF \
  -DBUILD_JAVA_BINDINGS=OFF \
  -DBUILD_CSHARP_BINDINGS=OFF \
  -DGDAL_USE_JPEG=ON \
  -DGDAL_USE_EXPAT=ON \
  -DGDAL_USE_GIF=ON \
  -DGDAL_USE_LERC=ON \
  -DGDAL_USE_OPENJPEG=ON \
  -DGDAL_USE_PNG=ON \
  -DGDAL_USE_SQLITE3=ON \
  -DGDAL_USE_TIFF=ON \
  -DGDAL_USE_ZLIB=ON \
  -DEMBED_RESOURCE_FILES=ON \
  -DUSE_ONLY_EMBEDDED_RESOURCE_FILES=ON \
  -DBUILD_APPS=OFF \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -G Ninja

  cmake .. ^
  -DBUILD_PYTHON_BINDINGS=OFF ^
  -DGDAL_ENABLE_PLUGINS=OFF ^
  -DBUILD_JAVA_BINDINGS=OFF ^
  -DBUILD_CSHARP_BINDINGS=OFF ^
  -DBUILD_APPS=OFF ^
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
  -DPROJ_LIBRARY=C:/OSGeo4W/lib ^
  -DPROJ_INCLUDE_DIR=C:/OSGeo4W/include ^
  -G Ninja

  # Extract unique defines
  egrep -oh '\-D[a-Z_]* ' compile_commands.json | sort -u
```

GDAL - Geospatial Data Abstraction Library
====

[![Build Status](https://github.com/OSGeo/gdal/actions/workflows/linux_build.yml/badge.svg)](https://github.com/osgeo/gdal/actions?query=workflow%3A%22Linux+Builds%22+branch%3Amaster)
[![Build Status](https://github.com/OSGeo/gdal/actions/workflows/macos.yml/badge.svg)](https://github.com/osgeo/gdal/actions?query=workflow%3A%22MacOS+build%22+branch%3Amaster)
[![Build Status](https://github.com/OSGeo/gdal/actions/workflows/windows_build.yml/badge.svg)](https://github.com/osgeo/gdal/actions?query=workflow%3A%22Windows+builds%22+branch%3Amaster)
[![Build Status](https://github.com/OSGeo/gdal/actions/workflows/android_cmake.yml/badge.svg)](https://github.com/osgeo/gdal/actions?query=workflow%3A%22Android+CMake+build%22+branch%3Amaster)
[![Build Status](https://github.com/OSGeo/gdal/actions/workflows/clang_static_analyzer.yml/badge.svg)](https://github.com/osgeo/gdal/actions?query=workflow%3A%22CLang+Static+Analyzer%22+branch%3Amaster)
[![Build Status](https://github.com/OSGeo/gdal/actions/workflows/code_checks.yml/badge.svg)](https://github.com/osgeo/gdal/actions?query=workflow%3A%22Code+Checks%22+branch%3Amaster)
[![Build Status](https://github.com/OSGeo/gdal/actions/workflows/conda.yml/badge.svg)](https://github.com/osgeo/gdal/actions?query=workflow%3A%22Conda%22+branch%3Amaster)
[![Build Status](https://scan.coverity.com/projects/749/badge.svg?flat=1)](https://scan.coverity.com/projects/gdal)
[![Documentation build Status](https://github.com/OSGeo/gdal/workflows/Docs/badge.svg)](https://github.com/osgeo/gdal/actions?query=workflow%3A%22Docs%22+branch%3Amaster)
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/gdal.svg)](https://issues.oss-fuzz.com/issues?q=status:open%20gdal)
[![Coverage Status](https://coveralls.io/repos/github/OSGeo/gdal/badge.svg?branch=master)](https://coveralls.io/github/OSGeo/gdal?branch=master)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/8250/badge)](https://www.bestpractices.dev/projects/8250)
[![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/OSGeo/gdal/badge)](https://securityscorecards.dev/viewer/?uri=github.com/OSGeo/gdal)

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.5884351.svg)](https://doi.org/10.5281/zenodo.5884351)

[![Powered by NumFOCUS](https://img.shields.io/badge/powered%20by-NumFOCUS-orange.svg?style=flat&colorA=E1523D&colorB=007D8A )](http://numfocus.org)


GDAL is an open source MIT licensed translator library for raster and vector geospatial data formats.

* Main site: https://gdal.org - Developer and user docs, links to other resources
* GIT repository: https://github.com/OSGeo/gdal
* Bug tracker: https://github.com/OSGeo/gdal/issues
* Download: https://download.osgeo.org/gdal
* Mailing list: https://lists.osgeo.org/mailman/listinfo/gdal-dev

[//]: # (numfocus-fiscal-sponsor-attribution)

The GDAL project uses a [custom governance](./GOVERNANCE.md)
and is fiscally sponsored by [NumFOCUS](https://numfocus.org/). Consider making
a [tax-deductible donation](https://numfocus.org/donate-to-gdal) to help the project
pay for developer time, professional services, travel, workshops, and a variety of other needs.

<div align="center">
  <a href="https://numfocus.org/project/gdal">
    <img height="60px"
         src="https://raw.githubusercontent.com/numfocus/templates/master/images/numfocus-logo.png"
         align="center">
  </a>
</div>
<br>

NumFOCUS is 501(c)(3) non-profit charity in the United States; as such, donations to
NumFOCUS are tax-deductible as allowed by law. As with any donation, you should
consult with your personal tax adviser or the IRS about your particular tax situation.

### How to build

See [BUILDING.md](BUILDING.md)

### How to contribute

See [CONTRIBUTING.md](CONTRIBUTING.md)

### Docker images

See [docker/](docker/)

### Code of Conduct

See [doc/source/community/code_of_conduct.rst](doc/source/community/code_of_conduct.rst)

### Security policy

See [SECURITY.md](SECURITY.md)

### Citing GDAL/OGR in publications

See [CITATION](CITATION) and [CITATION.cff](CITATION.cff)
