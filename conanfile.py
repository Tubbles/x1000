from conan import ConanFile


class app(ConanFile):
    name = "x1000"
    version = "1.0"
    package_type = "application"

    # Optional metadata
    license = "GPL-3.0-only"  # SPDX format
    author = "Tubbles jae91m@gmail.com"
    url = "https://github.com/Tubbles/x1000"
    description = "x1000 fantasy console system"
    topics = ("fantasy console")

    settings = "os", "compiler", "build_type", "arch"
    requires = [
        # "fmt/8.1.1",
        # "sdl/2.0.20",
        "argparse/2.6",
    ]
    generators = "cmake", "gcc", "txt"
