add_rules("mode.debug","mode.release")
set_runtimes("MD")
add_requires("glm","glfw","glad","glm","eigen","antlr4-runtime")
add_requires("imgui",{version="v1.91.5-docking", configs={opengl3=true,glfw=true}})
add_requires("stb",{version="2024.06.01"})
set_languages("c++20")


after_build(
    function (target)
        os.cp("assets", path.join(target:targetdir(), "./"))
        os.cp("favicon.png", path.join(target:targetdir(), "./"))
    end
)


target("lsystem")
  set_kind("static")
  add_files("lsystem/src/*.cpp")
  add_includedirs("lsystem/include")
  
  add_files("lsystem/antlr4/g4/LSystemParser/src/*.cpp")
  add_files("lsystem/antlr4/g4/LSystemRewriter/src/*.cpp")
  add_includedirs("lsystem/antlr4/g4/LSystemParser/include")
  add_includedirs("lsystem/antlr4/g4/LSystemRewriter/include")

  add_packages("glm","eigen","imgui","stb","glfw","glad")
  add_packages("antlr4-runtime",{public=true})


  on_load(
    -- 需要设置ANTLR4环境变量到antlr-4.13.2-complete.jar所在目录
    function (target)
        local antlr4 = os.getenv("ANTLR4")
        if not antlr4 then
            print("Please set ANTLR4 environment variable to the directory containing antlr-4.13.2-complete.jar")
            os.exit(1)
        end
        os.cd("lsystem/antlr4/g4")

        os.mkdir(".antlr4/LSystemParser")
        os.mkdir("LSystemParser/src")
        os.mkdir("LSystemParser/include")
        os.exec(format("java -jar %s/antlr-4.13.2-complete.jar -Dlanguage=Cpp -o .antlr4/LSystemParser -no-listener -visitor LSystemParser.g4", antlr4))
        os.cp(".antlr4/LSystemParser/*.cpp", "LSystemParser/src/")
        os.cp(".antlr4/LSystemParser/*.h", "LSystemParser/include/")

        os.mkdir(".antlr4/LSystemRewriter")
        os.mkdir("LSystemRewriter/src")
        os.mkdir("LSystemRewriter/include")
        os.exec(format("java -jar %s/antlr-4.13.2-complete.jar -Dlanguage=Cpp -o .antlr4/LSystemRewriter -no-listener -visitor LSystemRewriter.g4", antlr4))
        os.cp(".antlr4/LSystemRewriter/*.cpp", "LSystemRewriter/src/")
        os.cp(".antlr4/LSystemRewriter/*.h", "LSystemRewriter/include/")

        os.cd("../../..")
    end
  )



-- target("demo")
--     set_kind("binary")
--     add_packages("glm","glfw","glad","glm","imgui","stb","eigen")

--     add_files("src/lsystem_demonstration.cpp")
--     add_includedirs("include")




-- target("demo")
--     set_kind("binary")
--     add_includedirs("include")
--     add_files("src/imgui_test.cpp")
--     add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
--     after_build(
--         function (target)
--             os.cp("assets", path.join(target:targetdir(), "./"))
--             os.cp("favicon.png", path.join(target:targetdir(), "./"))
--         end
--     )
    
    
-- target("demo")
--     set_kind("binary")
--     add_includedirs("include")
--     add_includedirs("LSystem")
--     add_files("src/main_demonstration.cpp")
--     add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
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

