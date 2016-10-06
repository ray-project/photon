from setuptools import setup, find_packages, Extension

photon_client_module = Extension("photon_client",
                                 sources=["photon_extension.c"],
                                 include_dirs=["../../", "../../common/",
                                               "../../common/thirdparty/",
                                               "../../common/lib/python"],
                                 extra_objects=["../../build/photon_client.a", "../../common/build/libcommon.a"],
                                 extra_compile_args=["--std=c99", "-Werror"])

setup(name="PhotonClient",
      version="0.1",
      description="Photon client library for Ray",
      ext_modules=[photon_client_module])
