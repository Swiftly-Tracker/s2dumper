add_rules("mode.debug", "mode.release")

set_languages("cxx23")

local sdk_path = "vendor/s2sdk"

target("s2dumper")
    set_kind("binary")
    
    set_languages("cxx23")
    set_optimize("fastest")
    set_symbols("debug")
    set_strip("none")

    add_files({
        "src/*.cpp",
        "src/**/*.cpp",

        sdk_path.."/public/tier1/convar.cpp",
        sdk_path.."/public/tier0/memoverride.cpp",

        "vendor/safetyhook/safetyhook.cpp",
        "vendor/safetyhook/Zydis.c",
    })

    add_includedirs({
        "vendor",

        sdk_path,
        sdk_path.."/thirdparty/protobuf-3.21.8/src",
        sdk_path.."/public",
        sdk_path.."/public/engine",
        sdk_path.."/public/mathlib",
        sdk_path.."/public/vstdlib",
        sdk_path.."/public/tier0",
        sdk_path.."/public/tier1",
        sdk_path.."/public/entity2",
        sdk_path.."/public/game/server",
        sdk_path.."/game/shared",
        sdk_path.."/game/server",
        sdk_path.."/common",
    })

    if is_plat("windows") then
        add_cxflags("/utf-8")
    end

    add_cxxflags("gcc::-Wno-invalid-offsetof")
    add_cxxflags("gcc::-Wno-return-local-addr")
    add_cxxflags("gcc::-Wno-overloaded-virtual")
    add_cxxflags("gcc::-Wno-unknown-pragmas")
    add_cxxflags("gcc::-Wno-non-virtual-dtor")
    add_cxxflags("gcc::-Wno-attributes")
    add_cxxflags("gcc::-Wno-array-bounds")
    add_cxxflags("gcc::-Wno-int-to-pointer-cast")
    add_cxxflags("gcc::-Wno-sign-compare")
    add_cxxflags("gcc::-Wno-write-strings")
    add_cxxflags("gcc::-Wno-class-memaccess")
    add_cxxflags("gcc::-fexceptions")
    add_cxxflags("gcc::-fPIC")
    
    add_cflags("gcc::-Wno-return-local-addr")
    add_cflags("gcc::-Wno-unknown-pragmas")
    add_cflags("gcc::-Wno-attributes")
    add_cflags("gcc::-Wno-array-bounds")
    add_cflags("gcc::-Wno-int-to-pointer-cast")
    add_cflags("gcc::-Wno-sign-compare")
    add_cflags("gcc::-Wno-write-strings")
    add_cflags("gcc::-fexceptions")
    add_cflags("gcc::-fPIC")
    add_cflags("gcc::-pipe")
    add_cflags("gcc::-fno-strict-aliasing")
    add_cflags("gcc::-Wall")
    add_cflags("gcc::-Wno-uninitialized")
    add_cflags("gcc::-Wno-unused")
    add_cflags("gcc::-Wno-switch")
    add_cflags("gcc::-msse")
    add_cflags("gcc::-fvisibility=hidden")
    add_cflags("gcc::-mfpmath=sse")
    add_cflags("gcc::-fno-omit-frame-pointer")
    add_cflags("gcc::-fvisibility-inlines-hidden")
    add_cflags("gcc::-fno-exceptions")
    add_cflags("gcc::-fno-threadsafe-statics")
    add_cflags("gcc::-Wno-register")
    add_cflags("gcc::-Wno-delete-non-virtual-dtor")

    add_cxxflags("cl::/Zc:__cplusplus")
    add_cxxflags("cl::/Ox")
    add_cxxflags("cl::/Zo")
    add_cxxflags("cl::/Oy-")
    add_cxxflags("cl::/TP")
    add_cxxflags("cl::/W3")
    add_cxxflags("cl::/EHsc")
    add_cxxflags("cl::/wd4003")
    add_cxxflags("cl::/wd4005")
    add_cxxflags("cl::/wd4018")
    add_cxxflags("cl::/wd4101")
    add_cxxflags("cl::/wd4146")
    add_cxxflags("cl::/wd4267")
    add_cxxflags("cl::/wd4244")
    add_cxxflags("cl::/wd4530")
    add_cxxflags("cl::/wd4828")
    add_cxxflags("cl::/wd4834")

    set_runtimes("MT")

    add_files({
        sdk_path.."/mathlib/mathlib.cpp"
    }, { cxxflags = "-g0" })

    if is_plat("windows") then
        add_links({
            sdk_path.."/lib/public/win64/tier0.lib",
            sdk_path.."/lib/public/win64/tier1.lib",
            sdk_path.."/lib/public/win64/interfaces.lib",
            sdk_path.."/lib/public/win64/2015/libprotobuf.lib",
            sdk_path.."/lib/public/win64/steam_api64.lib",
            "vendor/s2binlib/s2binlib.lib"
        })
    else
        add_links({
            sdk_path.."/lib/linux64/libtier0.so",
            sdk_path.."/lib/linux64/tier1.a",
            sdk_path.."/lib/linux64/interfaces.a",
            sdk_path.."/lib/linux64/release/libprotobuf.a",
            sdk_path.."/lib/linux64/libsteam_api.so",
            "vendor/s2binlib/libs2binlib.a"
        })
    end

    if(is_plat("windows")) then
        add_defines({
            "COMPILER_MSVC",
            "COMPILER_MSVC64",
            "WIN32",
            "WINDOWS",
            "CRT_SECURE_NO_WARNINGS",
            "CRT_SECURE_NO_DEPRECATE",
            "CRT_NONSTDC_NO_DEPRECATE",
            "_MBCS",
            "META_IS_SOURCE2",
            "COMPILER_MSVC",
            "COMPILER_MSVC64",
            "WIN32",
            "_WIN32",
            "WINDOWS",
            "_WINDOWS",
            "CRT_SECURE_NO_WARNINGS",
            "_CRT_SECURE_NO_WARNINGS",
            "CRT_SECURE_NO_DEPRECATE",
            "_CRT_SECURE_NO_DEPRECATE",
            "CRT_NONSTDC_NO_DEPRECATE",
            "_CRT_NONSTDC_NO_DEPRECATE",
            "_MBCS",
            "META_IS_SOURCE2",
            "X64BITS",
            "PLATFORM_64BITS",
            "NDEBUG",
            "JSON_HAS_CPP_14",
            "JSON_HAS_CPP_11",
            "_LIBCPP_STD_VER",
        })
    else
        add_defines({
            "_LINUX",
            "LINUX",
            "POSIX",
            "GNUC",
            "COMPILER_GCC",
            "PLATFORM_64BITS",
            "META_IS_SOURCE2",
            "_GLIBCXX_USE_CXX11_ABI=0",

            "_vsnprintf=vsnprintf",
            "_alloca=alloca",
            "strcmpi=strcasecmp",
            "strnicmp=strncasecmp",
            "_snprintf=snprintf",
            "_stricmp=strcasecmp",
            "_strnicmp=strncasecmp",
            "stricmp=strcasecmp",
        })
    end

    if is_plat("windows") then
        add_links({
            "psapi",
            "winmm",
            "ws2_32",
            "wldap32",
            "advapi32",
            "kernel32",
            "comdlg32",
            "crypt32",
            "normaliz",
            "wsock32",
            "legacy_stdio_definitions",
            "legacy_stdio_wide_specifiers",
            "user32",
            "gdi32",
            "winspool",
            "shell32",
            "ole32",
            "oleaut32",
            "uuid",
            "odbc32",
            "odbccp32",
            "dbghelp",
            "ntdll",
            "kernel32",
        })
    else
        add_links({
            "gnutls",
            "z",
            "pthread",
            "ssl",
            "crypto",
            "m",
            "dl",
            "readline",
            "rt",
            "idn2",
            "psl",
            "brotlidec",
            "backtrace",
            "stdc++",
        })
    end
