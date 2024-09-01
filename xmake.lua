set_project("cairopp")
set_version("0.1.0")

add_requires("cairo")

add_rules("mode.debug", "mode.releasedbg", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "$(buildir)"})

set_policy("build.warning", true)
set_warnings("allextra")
set_languages("cxx17")
set_encodings("utf-8")

target("cairopp-samples")
    set_kind("binary")
    add_files("samples/samples.cc")
    add_packages("cairo")
    add_includedirs(".")
    set_rundir("$(projectdir)/samples")
