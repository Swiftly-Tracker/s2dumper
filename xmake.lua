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
        "src/**/*.cpp"
    })

    add_includedirs({
        "vendor"
    })
