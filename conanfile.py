from conans import ConanFile
from conans import CMake


class Cis1CWUTransport(ConanFile):
    name = "cis1_cwu_transport"
    version = "0.0.2"
    description = "Transport part of CWU protocol for cis1."
    author = "MokinIA <mia@tomsksoft.com>"
    settings = "os", "arch", "compiler", "build_type"
    generators = "cmake"
    exports = []
    exports_sources = ["CMakeLists.txt", "include/*", "src/*"]
    requires = ("boost_system/1.69.0@bincrafters/stable",
                "boost_asio/1.69.0@bincrafters/stable",
                "cis1_proto_utils/test@tomsksoft/cis1")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include/cis1_cwu_transport", src="include")
        self.copy("libcis1_cwu_transport.a", dst="lib", src="lib")
        self.copy("libcis1_cwu_transport.lib", dst="lib", src="lib")

    def package_info(self):
        self.cpp_info.libs = ["cis1_cwu_transport"]
