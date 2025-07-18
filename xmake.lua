add_rules("mode.debug", "mode.release")

target("HelloCPlusPlus")
    set_kind("binary")
    set_languages("c++23")
    add_files("src/*.cpp", "version.rc")
    add_includedirs("include")
    add_syslinks("user32", "gdi32", "dxgi", "d3d12", "d3dcompiler")
    
    
  
