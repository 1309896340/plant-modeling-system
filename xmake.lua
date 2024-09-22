add_rules("mode.debug","mode.release")
add_requires("glm","glfw","glad")

target("demo")
    set_kind("binary")
    add_includedirs("include")
    add_files("src/demo.cpp")
    add_packages("glm","glfw","glad")
