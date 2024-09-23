add_rules("mode.debug","mode.release")
add_requires("glm","glfw","glad","glm")
set_languages("c++17")

target("demo")
    set_kind("binary")
    add_includedirs("include")
    add_files("src/demo.cpp")
    add_packages("glm","glfw","glad","glm")

    after_build(
        function (target)
            os.cp("shaders", path.join(target:targetdir(), "./"))
        end
    )
