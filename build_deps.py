import os
import sys
import shutil
import zipfile
import platform
import tempfile
import subprocess

target_os = ""
target_arch = ""
target_ver = ""

use_curl = False
use_wget = False
use_pwsh = False

if len(sys.argv) >= 2:
	target_os = sys.argv[1]
else:
	if platform.uname().system == "Darwin":
		target_os = "macOS"
	else:
		target_os = platform.uname().system

if len(sys.argv) >= 3:
	target_arch = sys.argv[2]
elif target_os == "iOS":
	target_arch = "arm64"
else:
	target_arch = platform.uname().machine

if len(sys.argv) == 4:
	target_ver = sys.argv[3]
elif target_os == "iOS":
	target_ver = "16.4"
else:
	target_ver = platform.uname().release

print("Building dependencies for", target_os, target_arch)

if target_os == "Android" and target_arch == "arm64-v8a":
	target_arch = "aarch64"

out_dir = os.path.join(os.getcwd(), "Deps", target_os, target_arch)
inc_dir = os.path.join(out_dir, "include")
lib_dir = os.path.join(out_dir, "lib")
bin_dir = os.path.join(out_dir, "bin")
src_dir = os.getcwd()

if os.path.isdir(out_dir):
	shutil.rmtree(out_dir)
os.makedirs(out_dir, exist_ok = True)

cmake_cmd = ["cmake", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_PREFIX_PATH=" + out_dir, "-DCMAKE_INSTALL_PREFIX=" + out_dir, ".."]

if target_os == "Windows":
	cflags = "-DCMAKE_C_FLAGS=-I" + inc_dir
else:
	cflags = "-DCMAKE_C_FLAGS=-fPIC -I" + inc_dir

if shutil.which("ninja"):
	cmake_cmd.extend(["-G", "Ninja"])

if target_os == "iOS":
	cmake_cmd.extend(["-DCMAKE_SYSTEM_NAME=iOS", "-DCMAKE_OSX_ARCHITECTURES=" + target_arch,
					"-DCMAKE_OSX_SYSROOT=iphoneos", "-DCMAKE_OSX_DEPLOYMENT_TARGET=" + target_ver])
elif target_os == "Android":
	if target_arch == "aarch64":
		target_arch = "arm64-v8a"
	cmake_cmd.extend(["-DCMAKE_TOOLCHAIN_FILE=" + os.environ['NDK'] + '/build/cmake/android.toolchain.cmake',
					"-DANDROID_ABI=" + target_arch, "-DANDROID_PLATFORM=android-" + target_ver])

use_curl = shutil.which("curl")
use_wget = shutil.which("wget")
use_pwsh = shutil.which("powershell")

def download_file(name, url):
	if use_curl:
		subprocess.run(["curl", "-o", name, "-L", url])
	elif use_wget:
		subprocess.run(["wget", "-O", name, url])
	elif use_pwsh:
		subprocess.run(["powershell", "-Command", "\"Invoke-WebRequest " + url + " -OutFile " + name])

def build_cmake(name, url, args):
	print(name)

	if url:
		ar_name = name + ".tar.gz"
		download_file(ar_name, url)
		subprocess.run(["cmake", "-E", "tar", "zxf", ar_name])

	os.chdir(name)

	if os.path.isdir("build"):
		shutil.rmtree("build")

	os.mkdir("build")
	os.chdir("build")

	cmd = cmake_cmd.copy()
	cmd.extend(args)
	
	subprocess.run(cmd)
	subprocess.run(["cmake", "--build", ".", "--config=Release"])
	subprocess.run(["cmake", "--install", ".", "--config=Release"])
	
	os.chdir(os.path.join("..", ".."))

def build_autotools(name, url):
	print(name)

	if url:
		ar_name = name + ".tar.gz"
		download_file(ar_name, url)
		subprocess.run(["tar", "zxf", ar_name])

	os.chdir(name)

	subprocess.run(["sh", "configure", "--prefix=" + out_dir])
	subprocess.run(["make", "-j"])
	subprocess.run(["make", "install"])
	
	os.chdir("..")

# Libraries included in the source tree
os.chdir("Deps")

build_cmake("Lua", None, [])
build_cmake("PhysFS", None, [])

os.chdir("..")

with tempfile.TemporaryDirectory() as tmpdir:
	os.chdir(tmpdir)

	build_cmake("zlib-ng-2.0.7", "https://github.com/zlib-ng/zlib-ng/archive/refs/tags/2.0.7.tar.gz", [
		"-DZLIB_COMPAT:BOOL=ON",
		"-DBUILD_SHARED_LIBS:BOOL=OFF",
		"-DZLIB_ENABLE_TESTS:BOOL=OFF",
		cflags
	])

	build_cmake("libpng-1.6.39", "https://sourceforge.net/projects/libpng/files/libpng16/1.6.39/libpng-1.6.39.tar.gz/download", [
		"-DPNG_BUILD_ZLIB=" + out_dir,
		"-DPNG_SHARED:BOOL=OFF",
		"-DPNG_EXECUTABLES:BOOL=OFF",
		"-DPNG_TESTS:BOOL=OFF",
		cflags
	])

	build_cmake("bullet3-3.25", "https://github.com/bulletphysics/bullet3/archive/refs/tags/3.25.tar.gz", [
		"-DUSE_GRAPHICAL_BENCHMARK=OFF",
		"-DUSE_OPENVR:BOOL=OFF",
		"-DUSE_GLUT:BOOL=OFF",
		"-DBUILD_UNIT_TESTS:BOOL=OFF",
		"-DBUILD_SHARED_LIBS:BOOL=OFF",
		"-DBUILD_PYBULLET:BOOL=OFF",
		"-DBUILD_OPENGL3_DEMOS:BOOL=OFF",
		"-DBUILD_EXTRAS:BOOL=OFF",
		"-DBUILD_ENET:BOOL=OFF",
		"-DBUILD_CPU_DEMOS:BOOL=OFF",
		"-DBUILD_CLSOCKET:BOOL=OFF",
		"-DBUILD_BULLET_ROBOTICS_EXTRA:BOOL=OFF",
		"-DBUILD_BULLET_ROBOTICS_GUI_EXTRA:BOOL=OFF",
		"-DBUILD_BULLET2_DEMOS:BOOL=OFF",
		"-DINSTALL_LIBS:BOOL=ON",
		cflags
	])

	build_cmake("libjpeg-turbo-2.1.91", "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.91.tar.gz", [
		"-DENABLE_SHARED:BOOL=OFF",
		cflags
	])

	build_cmake("libogg-1.3.5", "https://github.com/xiph/ogg/releases/download/v1.3.5/libogg-1.3.5.tar.gz", [
		"-DINSTALL_DOCS:BOOL=OFF",
		"-DBUILD_TESTING:BOOL=OFF",
		cflags
	])

	build_cmake("libvorbis-1.3.7", "https://github.com/xiph/vorbis/releases/download/v1.3.7/libvorbis-1.3.7.tar.gz", [
		cflags
	])

	build_cmake("flac-1.4.2", "https://github.com/xiph/flac/archive/refs/tags/1.4.2.tar.gz", [
		"-DBUILD_CXXLIBS:BOOL=OFF",
		"-DBUILD_PROGRAMS:BOOL=OFF",
		"-DBUILD_EXAMPLES:BOOL=OFF",
		"-DBUILD_TESTING:BOOL=OFF",
		"-DBUILD_DOCS:BOOL=OFF",
		"-DINSTALL_MANPAGES:BOOL=OFF",
		"-DINSTALL_PKGCONFIG_MODULES:BOOL=OFF",
		"-DWITH_OGG:BOOL=ON",
		"-DBUILD_SHARED_LIBS:BOOL=OFF",
		cflags
	])

	if target_os != "iOS":
		# bzip2
		# harfbuzz
		build_cmake("freetype-2.13.0", "https://download.savannah.gnu.org/releases/freetype/freetype-2.13.0.tar.gz", [
			"-DZLIB_INCLUDE_DIR:PATH=" + inc_dir,
			"-DZLIB_LIBRARY_RELEASE:FILEPATH=" + os.path.join(lib_dir, "libz.a"),
			"-DPNG_PNG_INCLUDE_DIR=" + inc_dir,
			"-DPNG_LIBRARY_RELEASE:FILEPATH=" + os.path.join(lib_dir, "libpng.a"),
			cflags
		])

		build_cmake("meshoptimizer-0.19", "https://github.com/zeux/meshoptimizer/archive/refs/tags/v0.19.tar.gz", [cflags])

	if target_os == "Windows":	# Cairo is impossible to build on Windows due to autotools / Meson
		download_file("cairo.zip", "https://github.com/preshing/cairo-windows/releases/download/1.17.2/cairo-windows-1.17.2.zip")
		with zipfile.ZipFile("cairo.zip", 'r') as ar:
			ar.extractall(".")

		shutil.copytree("cairo-windows-1.17.2/include", os.path.join(inc_dir, "cairo"))
		if target_arch == "AMD64":
			shutil.copy("cairo-windows-1.17.2/lib/x64/cairo.lib", os.path.join(lib_dir, "cairo.lib"))
			shutil.copy("cairo-windows-1.17.2/lib/x64/cairo.dll", os.path.join(bin_dir, "cairo.dll"))
		else:
			shutil.copy("cairo-windows-1.17.2/lib/x86/cairo.lib", os.path.join(lib_dir, "cairo.lib"))
			shutil.copy("cairo-windows-1.17.2/lib/x86/cairo.dll", os.path.join(bin_dir, "cairo.dll"))

		os.chdir("..")

		download_file("openal.zip", "https://openal-soft.org/openal-binaries/openal-soft-1.23.1-bin.zip")
		with zipfile.ZipFile("openal.zip", 'r') as ar:
			ar.extractall(".")

		shutil.copytree("openal-soft-1.23.1-bin/include/AL", os.path.join(inc_dir, "AL"))
		if target_arch == "AMD64":
			shutil.copy("openal-soft-1.23.1-bin/libs/Win64/libOpenAL32.dll.a", os.path.join(lib_dir, "libOpenAL32.dll.a"))
			shutil.copy("openal-soft-1.23.1-bin/libs/Win64/OpenAL32.def", os.path.join(lib_dir, "OpenAL32.def"))
			shutil.copy("openal-soft-1.23.1-bin/libs/Win64/OpenAL32.lib", os.path.join(lib_dir, "OpenAL32.lib"))
			shutil.copy("openal-soft-1.23.1-bin/bin/Win64/soft_oal.dll", os.path.join(bin_dir, "OpenAL32.dll"))
		else:
			shutil.copy("openal-soft-1.23.1-bin/libs/Win32/libOpenAL32.dll.a", os.path.join(lib_dir, "libOpenAL32.dll.a"))
			shutil.copy("openal-soft-1.23.1-bin/libs/Win32/OpenAL32.def", os.path.join(lib_dir, "OpenAL32.def"))
			shutil.copy("openal-soft-1.23.1-bin/libs/Win32/OpenAL32.lib", os.path.join(lib_dir, "OpenAL32.lib"))
			shutil.copy("openal-soft-1.23.1-bin/bin/Win32/soft_oal.dll", os.path.join(bin_dir, "OpenAL32.dll"))

		os.chdir("..")
	#else:
		#build_autotools(,)

	os.chdir(src_dir)

print("NekoEngine dependencies built in", out_dir)
