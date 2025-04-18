add_rules("mode.debug","mode.release")
set_runtimes("MD")
add_requires("glm","glfw","glad","eigen","antlr4-runtime")
add_requires("imgui",{version="v1.91.5-docking", configs={opengl3=true,glfw=true}})
add_requires("stb",{version="2024.06.01"})
set_languages("c++20")

add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate",{outputdir=".vscode"})

on_load(
  function (target)
      local antlr4 = os.getenv("ANTLR4")
      if not antlr4 then
          print("Please set ANTLR4 environment variable to the directory containing antlr-4.13.2-complete.jar")
          os.exit(1)
      end
      os.cd("lsystem/antlr4/g4")

      os.exec("java -jar ../antlr-4.13.2-complete.jar -Dlanguage=Cpp -o .antlr4/LSystemParser -no-listener -visitor LSystemParser.g4")
      os.cp(".antlr4/LSystemParser/*.cpp", "../LSystemParser/src/")
      os.cp(".antlr4/LSystemParser/*.h", "../LSystemParser/include/")

      os.exec("java -jar ../antlr-4.13.2-complete.jar -Dlanguage=Cpp -o .antlr4/LSystemInput -no-listener -visitor LSystemInput.g4")
      os.cp(".antlr4/LSystemInput/*.cpp", "../LSystemInput/src/")
      os.cp(".antlr4/LSystemInput/*.h", "../LSystemInput/include/")

      os.cd("../../..")
  end
)

target("lsystem")
  set_kind("static")
  add_files("lsystem/src/*.cpp")
  add_includedirs("lsystem/include",{public=true})
  
  add_files("lsystem/antlr4/LSystemParser/src/*.cpp")
  add_files("lsystem/antlr4/LSystemInput/src/*.cpp")
  add_includedirs("lsystem/antlr4/LSystemParser/include")
  add_includedirs("lsystem/antlr4/LSystemInput/include")

  add_packages("antlr4-runtime","glm","eigen","imgui","stb","glfw","glad")

target("scene")
    set_kind("static")
    add_files("src/*.cpp")
    add_includedirs("include")
    add_deps("lsystem")
    add_packages("glm","glfw","glad","imgui","stb","eigen")
    
-- ================================模块演示================================

target("lsystem_demonstration")
    set_kind("binary")
    add_files("demo/lsystem_demonstration.cpp")
    add_includedirs("include")
    add_packages("glm","imgui","glad","eigen")
    add_deps("scene")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
        end
    )
    set_enabled(true)

target("geometry_test")
    set_kind("binary")
    add_includedirs("include")
    add_files("demo/geometry_test.cpp")
    add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
    add_deps("lsystem","scene")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
        end
    )
    set_enabled(false)

target("geometry_test2")
    set_kind("binary")
    add_includedirs("include")
    add_files("demo/geometry_test2.cpp")
    add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
    add_deps("lsystem","scene")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
        end
    )
    set_enabled(false)

target("geometry_test3")
    set_kind("binary")
    add_includedirs("include")
    add_files("demo/geometry_test3.cpp")
    add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
    add_deps("lsystem","scene")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
        end
    )
    set_enabled(false)

-- 测试imgui使用
target("imgui_test")
    set_kind("binary")
    add_includedirs("include")
    add_files("demo/imgui_test.cpp")
    add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
    add_deps("lsystem","scene")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
        end
    )
    set_enabled(false)
    

-- 测试骨架
target("skeleton_test")
    set_kind("binary")
    add_includedirs("include")
    add_files("demo/skeleton_test.cpp")
    add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
    add_deps("lsystem","scene")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
        end
    )
    set_enabled(false)

-- 测试计算着色器使用
target("compute_shader_demo")
    set_kind("binary")
    add_includedirs("include")
    add_files("demo/compute_shader_demo.cpp")
    add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
    add_deps("lsystem","scene")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
        end
    )
    set_enabled(false)

-- 测试光线追踪渲染
target("raytracing_image")
    set_kind("binary")
    add_includedirs("include")
    add_files("demo/raytracing_image.cpp")
    add_packages("glm","glfw","glad","glm","imgui","stb","eigen")
    add_deps("lsystem","scene")
    after_build(
        function (target)
            os.cp("assets", path.join(target:targetdir(), "./"))
        end
    )
    set_enabled(false)

