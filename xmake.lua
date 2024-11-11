add_rules("mode.debug","mode.release")
set_runtimes("MD")
add_requires("glm","glfw","glad","glm","eigen")
add_requires("imgui",{version="v1.91.1-docking", configs={opengl3=true,glfw=true}})
add_requires("stb",{version="2024.06.01"})
add_requires("vcpkg::opencascade",{alias = "opencascade"})
set_languages("c++20")

target("demo")
    set_kind("binary")
    add_includedirs("include")
    add_files("src/demo.cpp")
    add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
            os.cp("favicon.png", path.join(target:targetdir(), "./"))
        end
    )

-- target("skeleton_demo")
--     set_kind("binary")
--     add_includedirs("include")
--     add_files("src/skeleton_demo.cpp")
--     add_packages("glm","glfw","glad","glm","imgui","stb")
--     after_build(
--         function (target)
--             os.cp("assets", path.join(target:targetdir(), "./"))
--             os.cp("favicon.png", path.join(target:targetdir(), "./"))
--         end
--     )

-- target("compute_shader_demo")
--     set_kind("binary")
--     add_includedirs("include")
--     add_files("src/compute_shader_demo.cpp")
--     add_packages("glm","glfw","glad","glm","imgui","stb")
--     after_build(
--         function (target)
--             os.cp("assets", path.join(target:targetdir(), "./"))
--             os.cp("favicon.png", path.join(target:targetdir(), "./"))
--         end
--     )

-- target("raytracing_image")
--     set_kind("binary")
--     add_includedirs("include")
--     add_files("src/raytracing_image.cpp")
--     add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
--     after_build(
--         function (target)
--             os.cp("assets", path.join(target:targetdir(), "./"))
--             os.cp("favicon.png", path.join(target:targetdir(), "./"))
--         end
--     )


-- 主

-- target("test")
--     set_kind("binary")
--     add_includedirs("include")
--     add_files("src/test.cpp")
--     add_packages("glm","glfw","glad","glm","imgui","stb","eigen","opencascade")
--     after_build(
--         function (target)
--             os.cp("assets", path.join(target:targetdir(), "./"))
--             os.cp("favicon.png", path.join(target:targetdir(), "./"))
--         end
--     )