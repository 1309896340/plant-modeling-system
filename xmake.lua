add_rules("mode.debug","mode.release")
add_requires("glm","glfw","glad","glm")
add_requires("imgui",{version="v1.91.1-docking", configs={opengl3=true,glfw=true}})
add_requires("stb",{version="2024.06.01"})
set_languages("c++17")
-- target("demo")
--     set_kind("binary")
--     add_includedirs("include")
--     add_files("src/demo.cpp")
--     add_packages("glm","glfw","glad","glm","imgui","stb")
--     after_build(
--         function (target)
--             os.cp("assets", path.join(target:targetdir(), "./"))
--             os.cp("favicon.png", path.join(target:targetdir(), "./"))
--         end
--     )

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


target("compute_shader_demo")
    set_kind("binary")
    add_includedirs("include")
    add_files("src/compute_shader_demo.cpp")
    add_packages("glm","glfw","glad","glm","imgui","stb")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
            os.cp("favicon.png", path.join(target:targetdir(), "./"))
        end
    )

