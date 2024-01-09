workspace "Particle Pets"
    configurations { "release", "debug" }
    platforms "native"
    language "C"
    cdialect "C11"

    filter "configurations:debug"
        symbols "On"
        optimize "Off"

    filter "configurations:release"
        defines "NDEBUG"
        optimize "On"

    filter "system:windows"
        defines "_CRT_SECURE_NO_WARNINGS"
        
    filter "action:vs*"
        architecture "x86_64"

    filter { "toolset:clang or gcc", "not action:vs*" }
        buildoptions { "-pedantic", "-Wall", "-Wextra", "-Werror" }
		links "m"

    filter { "toolset:clang or gcc", "configurations:debug" }
        buildoptions "-Wno-error"
        sanitize { "Address", "Fuzzer" }
        
    project "microui"
        kind "StaticLib"
        files { "vendor/microui/src/microui.c", "vendor/microui/src/microui.h" }

    project "frontend"
        kind "ConsoleApp"
        targetname "pp"
        targetdir "build/%{cfg.buildcfg}/%{prj.name}"

        links "microui"
        
        files { "src/*.c", "src/*.inl", "src/*.h" }
        includedirs "vendor/microui/src"

		filter "action:vs*"
			libdirs { path.join(_OPTIONS["sdl-config"], "lib", "x64") }
			includedirs { path.join(_OPTIONS["sdl-config"], "include") }
			links { "SDL2", "SDL2main" }

		filter "not action:vs*" do
			local sdl_cfg = path.join(_OPTIONS["sdl-config"], "sdl2-config")
			buildoptions { string.format("`%s --cflags`", sdl_cfg) }
			linkoptions { string.format("`%s --libs`", sdl_cfg) }
		end