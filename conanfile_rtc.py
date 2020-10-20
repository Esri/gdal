from conans import ConanFile


class GDALConan(ConanFile):
    name = "gdal"
    version = "2.3.1"
    url = "https://github.com/Esri/gdal/tree/runtimecore"
    license = "https://github.com/Esri/gdal/blob/runtimecore/gdal/LICENSE.TXT"
    description = (
        "GDAL is an open source X/MIT licensed translator library for raster and vector geospatial data formats."
    )

    # RTC specific triple
    settings = "platform_architecture_target"

    def package(self):
        base = self.source_folder + "/"
        relative = "3rdparty/gdal/"

        # headers
        self.copy("*.h*", src=base + "gdal/gcore", dst=relative + "gdal/gcore", excludes=("*.html", "*.in", "*.vc"))
        self.copy("*.h*", src=base + "gdal/ogr", dst=relative + "gdal/ogr", excludes=("*.html", "*.in", "*.vc"))
        self.copy("*.h*", src=base + "gdal/port", dst=relative + "gdal/port", excludes=("*.html", "*.in", "*.vc"))

        # libraries
        output = "output/" + str(self.settings.platform_architecture_target) + "/staticlib"
        self.copy("*" + self.name + "*", src=base + "../../" + output, dst=output)
